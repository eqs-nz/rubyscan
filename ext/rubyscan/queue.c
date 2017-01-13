/* queue.c
 * threadsafe event queue for Ruby
 * written by Alex Fulton
 * Sept 2016
 */
#include "mod_runtime.h"

#define QUEUE_OPEN 1
#define QUEUE_CLOSED 0

static VALUE class_queue;

/* prototype declarations */
static VALUE rscan_queue_alloc(VALUE self);
static VALUE rscan_queue_m_initialize(VALUE self);
static rscan_queue_err_t rscan_queue_shift_ptr_gvl(rscan_event_queue_t *queue,
        queue_gvl_arg_t *args, rscan_match_event_t **out);
static void *queue_gvl_wrapper(void *arg);
static void queue_gvl_unblock(void *arg);
static rscan_queue_err_t rscan_queue_shift_inner(rscan_event_queue_t *queue, rscan_match_event_t **val);

/* public interface */
VALUE rscan_queue_define(VALUE root) {
    VALUE vClass = rb_define_class_under(root, "EventQueue", rb_cObject);
    rb_define_alloc_func(vClass, rscan_queue_alloc);
    rb_define_method(vClass, "initialize", rscan_queue_m_initialize, 0);
    return class_queue = vClass;
}

void rscan_queue_close(rscan_event_queue_t *queue) {
    pthread_mutex_lock(&queue->lock);
    if (queue->open) {
        queue->open = QUEUE_CLOSED;
        rscan_queue_drop_all(queue);
    }
    // subsequent calls to close() on a closed queue re-signal the conditions
    pthread_cond_broadcast(&queue->ready);
    pthread_cond_broadcast(&queue->empty);
    pthread_mutex_unlock(&queue->lock);
}

rscan_queue_err_t rscan_queue_closed_p(rscan_event_queue_t *queue) {
    return (queue->open == QUEUE_CLOSED);
}

void rscan_queue_open(rscan_event_queue_t *queue) {
    pthread_mutex_lock(&queue->lock);
    if (!queue->open) {
        queue->open = QUEUE_OPEN;
    }
    pthread_mutex_unlock(&queue->lock);
}

rscan_queue_err_t rscan_queue_open_p(rscan_event_queue_t *queue) {
    return (queue->open == QUEUE_OPEN);
}

void rscan_queue_put(rscan_event_queue_t *queue, rscan_match_event_t *event) {
    rscan_event_queue_member_t *unit;
    int queue_was_empty;
    pthread_mutex_lock(&queue->lock);
    if (!queue->open) {
        // events sent to a closed queue are dropped
        xfree(event);
        pthread_mutex_unlock(&queue->lock);
        return;
    }
    queue_was_empty = Queue_Empty(queue);
    unit = ALLOC(rscan_event_queue_member_t);
    unit->event = event;
    unit->next = NULL;
    if (queue->head == NULL) {
        queue->head = unit;
    }
    if (queue->tail != NULL) {
        queue->tail->next = unit;
    }
    queue->tail = unit;
    if (queue_was_empty) {
        pthread_cond_signal(&queue->ready);
    }
    pthread_mutex_unlock(&queue->lock);
}

rscan_match_event_t* rscan_queue_shift(rscan_event_queue_t *queue) {
    rscan_match_event_t *val;
    int status;
    if (status = rscan_queue_shift_ptr(queue, &val)) {
        errno = status;
        val = NULL;
    }
    return val;
}

// GVL-safe function: rscan_queue_shift_ptr

rscan_queue_err_t rscan_queue_shift_ptr(rscan_event_queue_t *queue, rscan_match_event_t **out) {
    return rscan_queue_shift_ptr_gvl(queue, NULL, out);
}

static rscan_queue_err_t rscan_queue_shift_ptr_gvl(rscan_event_queue_t *queue,
        queue_gvl_arg_t *args, rscan_match_event_t **out) {
    rscan_queue_err_t status;
    pthread_mutex_lock(&queue->lock);

    while (Queue_Empty(queue)) {
        if (args != NULL && args->interrupted) {
            *out = NULL;
            return RSCAN_QUEUE_FAIL_INTERRUPT;
        }
        if (!queue->open) {
            *out = NULL;
            pthread_mutex_unlock(&queue->lock);
            return RSCAN_QUEUE_FAIL_CLOSED;
        }
        pthread_cond_wait(&queue->ready, &queue->lock);

    }
    if (args != NULL && args->interrupted) {
        *out = NULL;
        return RSCAN_QUEUE_FAIL_INTERRUPT;
    }
    status = rscan_queue_shift_inner(queue, out);
    pthread_mutex_unlock(&queue->lock);
    return status;
}

rscan_queue_err_t rscan_queue_shift_nogvlwait(rscan_event_queue_t *queue, rscan_match_event_t **out) {
    rscan_queue_err_t status;
    queue_gvl_arg_t args;

    if (status = rscan_queue_shift_nonblock_opt(queue, 0, out)) {
        if (status == RSCAN_QUEUE_FAIL_LOCKBUSY || status == RSCAN_QUEUE_FAIL_EMPTY) {

            args.queue = queue;
            args.out = out;
            args.interrupted = 0;
            if (status = (rscan_queue_err_t) rb_thread_call_without_gvl(queue_gvl_wrapper, (void *) &args,
                    queue_gvl_unblock, (void *) &args)) {
                // TODO: handle/log failures
            }
        }
    }
    return status;
}

static void *queue_gvl_wrapper(void *arg) {
    queue_gvl_arg_t *args;
    args = (queue_gvl_arg_t *) arg;
    return (void *) rscan_queue_shift_ptr_gvl(args->queue, args, args->out);
}

static void queue_gvl_unblock(void *arg) {
    queue_gvl_arg_t *args;
    args = (queue_gvl_arg_t *) arg;
    args->interrupted = 1;
    pthread_cond_broadcast(&args->queue->ready);
}
// GVL-safe function: rscan_queue_shift_nonblock_opt

rscan_queue_err_t rscan_queue_shift_nonblock_opt(rscan_event_queue_t *queue,
        int bWaitForLock, rscan_match_event_t **out) {
    int status;
    /* task: acquire lock */
    if (!bWaitForLock) {
        if (status = pthread_mutex_trylock(&queue->lock)) {
            /* condition: exception */
            switch (status) {
                case EBUSY:
                    /* fail */
                    return RSCAN_QUEUE_FAIL_LOCKBUSY;
                default:
                    /* error */
                    return RSCAN_QUEUE_FAIL_UNDEF;
            }
        }
        /* condition: lock is held */
    } else {
        if (status = pthread_mutex_lock(&queue->lock)) {
            /* condition: error */
            return RSCAN_QUEUE_FAIL_UNDEF;
        }
    }
    /* condition: lock is held */
    if (Queue_Empty(queue)) {
        /* condition: fail */
        pthread_mutex_unlock(&queue->lock);
        return RSCAN_QUEUE_FAIL_EMPTY;
    }
    /* condition: lock is held, queue is not empty */
    return rscan_queue_shift_inner(queue, out);
}

void rscan_queue_destroy(rscan_event_queue_t *queue) {
    rscan_queue_drop_all(queue);
    pthread_cond_destroy(&(queue->ready));
    pthread_mutex_destroy(&(queue->lock));
    xfree(queue);
}

void rscan_queue_drop(rscan_event_queue_t *queue) {
    rscan_event_queue_member_t *next;
    pthread_mutex_lock(&queue->lock);

    next = queue->head->next;
    xfree(queue->head);
    queue->head = next;

    pthread_mutex_unlock(&queue->lock);
}

void rscan_queue_drop_all(rscan_event_queue_t *queue) {
    while (Queue_Nonempty(queue)) {
        rscan_queue_drop(queue);
    }
}

void rscan_queue_wait_until_empty(rscan_event_queue_t *queue) {
    pthread_mutex_lock(&queue->lock);
    if (!Queue_Empty(queue)) {
        pthread_cond_wait(&queue->empty, &queue->lock);
    }
    pthread_mutex_unlock(&queue->lock);
}

/* rubyscan::Native::EventQueue class functions */
static void rscan_queue_free(rscan_event_queue_t *queue) {
    rscan_queue_destroy(queue);
}

static VALUE rscan_queue_alloc(VALUE klass) {
    rscan_event_queue_t* pointer;

    VALUE self = Data_Make_Struct(klass, rscan_event_queue_t, NULL, rscan_queue_free, pointer);
    return self;
}

static VALUE rscan_queue_m_initialize(VALUE self) {
    rscan_event_queue_t *queue;
    pthread_mutexattr_t attr_lock;
    pthread_condattr_t attr_cond;
    int status;
    pthread_mutexattr_init(&attr_lock);
    pthread_mutexattr_settype(&attr_lock, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutexattr_setprotocol(&attr_lock, PTHREAD_PRIO_INHERIT);
    Get_Queue(self, queue);
    queue->open = 0;
    if (status = pthread_mutex_init(&queue->lock, &attr_lock)) {
        VALUE error, message;
        const char *msg = "Unspecified error";
        /* error */
        switch (status) {
            case EAGAIN:
                msg = "Insufficient system resources to create lock";
                break;
            case ENOMEM:
                msg = "Insufficient memory to create lock";
                break;
            case EPERM:
                msg = "Insufficient permissions to create lock";
                break;
            case EBUSY:
                msg = "Lock already initialized";
                break;
            case EINVAL:
                msg = "Invalid lock attribute object supplied";
                break;
        }
        queue->head = NULL;
        queue->tail = NULL;
        message = rb_sprintf("pthread_mutex_init failed: %s", msg);
        error = rb_exc_new(
                rscan_class_thread_error(), StringValuePtr(message), rb_str_strlen(message));
        rscan_thread_error_set(error, status);
        rb_exc_raise(error);
    }
    pthread_mutexattr_destroy(&attr_lock);

    // TODO- check for pthread_condattr_setclock() and use monotonic timer
    pthread_condattr_init(&attr_cond);
    pthread_condattr_setpshared(&attr_cond, PTHREAD_PROCESS_PRIVATE);
    if (status = pthread_cond_init(&queue->ready, &attr_cond)) {
        VALUE error, message;
        const char *msg = "Unspecified error";
        /* error */
        switch (status) {
            case ENOMEM:
                msg = "ENOMEM: Insufficient memory to create condition variable";
                break;
            case EINVAL:
                msg = "EINVAL: Invalid condition variable attribute object supplied";
                break;
        }
        queue->head = NULL;
        queue->tail = NULL;
        message = rb_sprintf("pthread_cond_init failed: %s", msg);
        error = rb_exc_new(
                rscan_class_thread_error(), StringValuePtr(message), rb_str_strlen(message));
        rscan_thread_error_set(error, status);
        rb_exc_raise(error);
    }
    pthread_condattr_destroy(&attr_cond);

    return self;
}

/* ***UNSAFE FUNCTION*** only call when write-lock on queue is held */
static rscan_queue_err_t rscan_queue_shift_inner(rscan_event_queue_t *queue, rscan_match_event_t **val) {
    rscan_event_queue_member_t *head = queue->head;
    *val = head->event;
    queue->head = head->next;
    if (queue->head != NULL && queue->head == queue->tail) {
        queue->head->next = queue->tail = NULL;
    }
    xfree(head);
    if (Queue_Empty(queue)) {
        pthread_cond_signal(&queue->empty);
    }
    return RSCAN_QUEUE_SUCCESS;
}

extern VALUE rscan_class_queue() {
    // TODO throw error if class is not loaded
    return class_queue;
}
