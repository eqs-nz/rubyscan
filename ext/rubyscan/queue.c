/* queue.c
 * threadsafe event queue for Ruby
 * written by Alex Fulton
 * Sept 2016
 */
#include "mod_runtime.h"

#define QUEUE_OPEN 1
#define QUEUE_CLOSED 0

static VALUE class_queue;

typedef struct queue_gvl_arg
{
    struct queue* queue;
    int interrupted;
    int idx;
    void* out;
} queue_gvl_arg_t;

/* prototype declarations */
static VALUE rscan_queue_m_initialize(VALUE self);
static rscan_queue_err_t
rscan_queue_shift_ptr_gvl(struct queue *queue, queue_gvl_arg_t *args, void **out);
static rscan_queue_err_t
rscan_queue_peek_ptr_gvl(struct queue *queue, unsigned int i, queue_gvl_arg_t *args, void **out);
static void *queue_shift_gvl_wrapper(void *arg);
static void *queue_peek_gvl_wrapper(void *arg);
static void queue_gvl_unblock(void *arg);
static rscan_queue_err_t rscan_queue_shift_inner(struct queue *queue, void **val);
static rscan_queue_err_t rscan_queue_peek_inner(struct queue *queue, unsigned int i, void **val);

/* public interface */
VALUE rscan_queue_define(VALUE root)
{
    VALUE klass = rb_define_class_under(root, "EventQueue", rb_cObject);
    rb_define_alloc_func(klass, rscan_queue_alloc);
    rb_define_method(klass, "initialize", rscan_queue_m_initialize, 0);
    return class_queue = klass;
}

void rscan_queue_close(struct queue *queue)
{
    pthread_mutex_lock(&queue->lock);
    if (queue->open)
    {
        queue->open = QUEUE_CLOSED;
        rscan_queue_drop_all(queue);
    }
    // subsequent calls to close() on a closed queue re-signal the conditions
    pthread_cond_broadcast(&queue->ready);
    pthread_cond_broadcast(&queue->empty);
    pthread_mutex_unlock(&queue->lock);
}

rscan_queue_err_t rscan_queue_closed_p(struct queue *queue)
{
    return (queue->open == QUEUE_CLOSED);
}

void rscan_queue_open(struct queue *queue)
{
    pthread_mutex_lock(&queue->lock);
    if (!queue->open)
    {
        queue->open = QUEUE_OPEN;
    }
    pthread_mutex_unlock(&queue->lock);
}

rscan_queue_err_t rscan_queue_open_p(struct queue *queue)
{
    return (queue->open == QUEUE_OPEN);
}

void rscan_queue_put(struct queue *queue, void *data)
{
    struct queue_member *unit;
    int queue_was_empty;
    pthread_mutex_lock(&queue->lock);
    if (!queue->open)
    {
        // events sent to a closed queue are dropped
        xfree(data);
        pthread_mutex_unlock(&queue->lock);
        return;
    }
    queue_was_empty = Queue_Empty(queue);
    unit = ALLOC(struct queue_member);
    unit->data = data;
    unit->next = NULL;
    if (queue->head == NULL)
    {
        queue->head = unit;
    }
    if (queue->tail != NULL)
    {
        queue->tail->next = unit;
    }
    queue->tail = unit;
    if (queue_was_empty)
    {
        pthread_cond_signal(&queue->ready);
    }
    pthread_mutex_unlock(&queue->lock);
}

void* rscan_queue_shift(struct queue *queue)
{
    void *val;
    int status;
    if (status = rscan_queue_shift_ptr(queue, &val))
    {
        errno = status;
        val = NULL;
    }
    return val;
}

// GVL-safe function: rscan_queue_shift_ptr

rscan_queue_err_t rscan_queue_shift_ptr(struct queue *queue, void **out)
{
    return rscan_queue_shift_ptr_gvl(queue, NULL, out);
}

static rscan_queue_err_t rscan_queue_shift_ptr_gvl(struct queue *queue,
                                                   queue_gvl_arg_t *args, void **out)
{
    rscan_queue_err_t status;
    pthread_mutex_lock(&queue->lock);

    while (Queue_Empty(queue))
    {
        if (args != NULL && args->interrupted)
        {
            *out = NULL;
            return RSCAN_QUEUE_FAIL_INTERRUPT;
        }
        if (!queue->open)
        {
            *out = NULL;
            pthread_mutex_unlock(&queue->lock);
            return RSCAN_QUEUE_FAIL_CLOSED;
        }
        pthread_cond_wait(&queue->ready, &queue->lock);

    }
    if (args != NULL && args->interrupted)
    {
        *out = NULL;
        return RSCAN_QUEUE_FAIL_INTERRUPT;
    }
    status = rscan_queue_shift_inner(queue, out);
    pthread_mutex_unlock(&queue->lock);
    return status;
}

rscan_queue_err_t rscan_queue_shift_nogvlwait(struct queue *queue, void **out)
{
    rscan_queue_err_t status;
    queue_gvl_arg_t args;

    if (status = rscan_queue_shift_nonblock_opt(queue, 0, out))
    {
        if (status == RSCAN_QUEUE_FAIL_LOCKBUSY || status == RSCAN_QUEUE_FAIL_EMPTY)
        {

            args.queue = queue;
            args.out = out;
            args.interrupted = 0;
            if (status = (rscan_queue_err_t)(unsigned long long)
                    rb_thread_call_without_gvl(queue_shift_gvl_wrapper,
                    (void *) &args, queue_gvl_unblock, (void *) &args)) {
                // TODO: handle/log failures
            }
        }
    }
    return status;
}

static void *queue_shift_gvl_wrapper(void *arg)
{
    queue_gvl_arg_t *args;
    args = (queue_gvl_arg_t *) arg;
    return (void *)(unsigned long long)
            rscan_queue_shift_ptr_gvl(args->queue, args, args->out);
}

static void queue_gvl_unblock(void *arg)
{
    queue_gvl_arg_t *args;
    args = (queue_gvl_arg_t *) arg;
    args->interrupted = 1;
    pthread_cond_broadcast(&args->queue->ready);
}
// GVL-safe function: rscan_queue_shift_nonblock_opt

rscan_queue_err_t rscan_queue_shift_nonblock_opt(struct queue *queue,
                                                 int bWaitForLock, void **out)
{
    int status;
    /* task: acquire lock */
    if (!bWaitForLock)
    {
        if (status = pthread_mutex_trylock(&queue->lock))
        {
            /* condition: exception */
            switch (status)
            {
            case EBUSY:
                /* fail */
                return RSCAN_QUEUE_FAIL_LOCKBUSY;
            default:
                /* error */
                return RSCAN_QUEUE_FAIL_UNDEF;
            }
        }
        /* condition: lock is held */
    }
    else
    {
        if (status = pthread_mutex_lock(&queue->lock))
        {
            /* condition: error */
            return RSCAN_QUEUE_FAIL_UNDEF;
        }
    }
    /* condition: lock is held */
    if (Queue_Empty(queue))
    {
        /* condition: fail */
        pthread_mutex_unlock(&queue->lock);
        return RSCAN_QUEUE_FAIL_EMPTY;
    }
    /* condition: lock is held, queue is not empty */
    return rscan_queue_shift_inner(queue, out);
}

void* rscan_queue_peek(struct queue *queue, unsigned int i)
{
    void *val;
    int status;
    if (status = rscan_queue_peek_ptr(queue, i, &val))
    {
        errno = status;
        val = NULL;
    }
    return val;
}

// GVL-safe function: rscan_queue_peek_ptr

rscan_queue_err_t rscan_queue_peek_ptr(struct queue *queue, unsigned int i, void **out)
{
    return rscan_queue_peek_ptr_gvl(queue, i, NULL, out);
}

static rscan_queue_err_t 
rscan_queue_peek_ptr_gvl(struct queue *queue, unsigned int i, queue_gvl_arg_t *args, void **out)
{
    unsigned int idx;
    rscan_queue_err_t status;
    pthread_mutex_lock(&queue->lock);

    while (Queue_Empty(queue))
    {
        if (args != NULL && args->interrupted)
        {
            *out = NULL;
            return RSCAN_QUEUE_FAIL_INTERRUPT;
        }
        if (!queue->open)
        {
            *out = NULL;
            pthread_mutex_unlock(&queue->lock);
            return RSCAN_QUEUE_FAIL_CLOSED;
        }
        pthread_cond_wait(&queue->ready, &queue->lock);
    }
    if (args != NULL && args->interrupted)
    {
        *out = NULL;
        return RSCAN_QUEUE_FAIL_INTERRUPT;
    }
    if (args != NULL)
    {
        idx = args->idx;
    }
    else
    {
        idx = i;
    }
    status = rscan_queue_peek_inner(queue, idx, out);
    pthread_mutex_unlock(&queue->lock);
    return status;
}

rscan_queue_err_t rscan_queue_peek_nogvlwait(struct queue *queue, unsigned int i, void **out)
{
    rscan_queue_err_t status;
    queue_gvl_arg_t args;

    if (status = rscan_queue_peek_nonblock_opt(queue, 0, i, out))
    {
        if (status == RSCAN_QUEUE_FAIL_LOCKBUSY || status == RSCAN_QUEUE_FAIL_EMPTY)
        {

            args.queue = queue;
            args.out = out;
            args.idx = i;
            args.interrupted = 0;
            if (status = (rscan_queue_err_t)(unsigned long long)
                    rb_thread_call_without_gvl(queue_peek_gvl_wrapper,
                    (void *) &args, queue_gvl_unblock, (void *) &args))
            {
                // TODO: handle/log failures
            }
        }
    }
    return status;
}

static void *queue_peek_gvl_wrapper(void *arg)
{
    queue_gvl_arg_t *args;
    args = (queue_gvl_arg_t *) arg;
    return (void *)(unsigned long long)
            rscan_queue_peek_ptr_gvl(args->queue, 0, args, args->out);
}

// GVL-safe function: rscan_queue_shift_nonblock_opt

rscan_queue_err_t
rscan_queue_peek_nonblock_opt(struct queue *queue, int bWaitForLock, unsigned int i, void **out)
{
    int status;
    /* task: acquire lock */
    if (!bWaitForLock)
    {
        if (status = pthread_mutex_trylock(&queue->lock))
        {
            /* condition: exception */
            switch (status)
            {
            case EBUSY:
                /* fail */
                return RSCAN_QUEUE_FAIL_LOCKBUSY;
            default:
                /* error */
                return RSCAN_QUEUE_FAIL_UNDEF;
            }
        }
        /* condition: lock is held */
    }
    else
    {
        if (status = pthread_mutex_lock(&queue->lock))
        {
            /* condition: error */
            return RSCAN_QUEUE_FAIL_UNDEF;
        }
    }
    /* condition: lock is held */
    if (Queue_Empty(queue))
    {
        /* condition: fail */
        pthread_mutex_unlock(&queue->lock);
        return RSCAN_QUEUE_FAIL_EMPTY;
    }
    /* condition: lock is held, queue is not empty */
    return rscan_queue_peek_inner(queue, i, out);
}

void rscan_queue_destroy(struct queue *queue)
{
    rscan_queue_drop_all(queue);
    pthread_cond_destroy(&(queue->ready));
    pthread_mutex_destroy(&(queue->lock));
    xfree(queue);
}

void rscan_queue_drop(struct queue *queue)
{
    struct queue_member *next;
    pthread_mutex_lock(&queue->lock);

    next = queue->head->next;
    xfree(queue->head);
    queue->head = next;

    pthread_mutex_unlock(&queue->lock);
}

void rscan_queue_drop_all(struct queue *queue)
{
    while (Queue_Nonempty(queue))
    {
        rscan_queue_drop(queue);
    }
}

void rscan_queue_wait_until_empty(struct queue *queue)
{
    pthread_mutex_lock(&queue->lock);
    if (!Queue_Empty(queue))
    {
        pthread_cond_wait(&queue->empty, &queue->lock);
    }
    pthread_mutex_unlock(&queue->lock);
}

/* rubyscan::Native::EventQueue class functions */
static void rscan_queue_free(struct queue *queue)
{
    rscan_queue_destroy(queue);
}

VALUE rscan_queue_alloc(VALUE klass)
{
    struct queue* pointer;

    VALUE self = Data_Make_Struct(klass, struct queue, NULL, rscan_queue_free, pointer);
    return self;
}

static VALUE rscan_queue_m_initialize(VALUE self)
{
    struct queue *queue;
    pthread_mutexattr_t attr_lock;
    pthread_condattr_t attr_cond;
    int status;
    pthread_mutexattr_init(&attr_lock);
    pthread_mutexattr_settype(&attr_lock, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutexattr_setprotocol(&attr_lock, PTHREAD_PRIO_INHERIT);
    Get_Queue(self, queue);
    queue->open = 0;
    if (status = pthread_mutex_init(&queue->lock, &attr_lock))
    {
        VALUE error, message;
        const char *msg = "Unspecified error";
        /* error */
        switch (status)
        {
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
    if (status = pthread_cond_init(&queue->ready, &attr_cond))
    {
        VALUE error, message;
        const char *msg = "Unspecified error";
        /* error */
        switch (status)
        {
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
static rscan_queue_err_t rscan_queue_shift_inner(struct queue *queue, void **val)
{
    struct queue_member *head = queue->head;
    *val = head->data;
    queue->head = head->next;
    if (queue->head != NULL && queue->head == queue->tail)
    {
        queue->head->next = queue->tail = NULL;
    }
    xfree(head);
    if (Queue_Empty(queue))
    {
        pthread_cond_signal(&queue->empty);
    }
    return RSCAN_QUEUE_SUCCESS;
}

/* ***UNSAFE FUNCTION*** only call when read-lock on queue is held */
static rscan_queue_err_t rscan_queue_peek_inner(struct queue *queue, unsigned int i, void **val)
{
    unsigned int count;
    struct queue_member *head = queue->head;
    
    if(i >= queue->length) {
        *val = NULL;
        return RSCAN_QUEUE_FAIL_BAD_INDEX;
    }
    for(count = 0; count < i; count++) {
        head = head->next;
    }
    *val = head->data;
    return RSCAN_QUEUE_SUCCESS;
}

extern VALUE rscan_class_queue()
{
    // TODO throw error if class is not loaded
    return class_queue;
}
