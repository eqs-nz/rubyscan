/* scanner.c
 * pattern scanner object class
 * written by Alex Fulton
 * Sept 2016
 */
#include "mod_runtime.h"

typedef struct scan_gvl_arg {
    rscan_scan_unit_t* unit;
    char* data;
    int len;
} scan_gvl_arg_t;

static ID id_new;
static ID id_abort_exc_set;

static VALUE class_unit;

static VALUE rscan_scan_unit_alloc(VALUE self);
static VALUE rscan_scan_unit_m_init(VALUE self, VALUE oDb);
static VALUE rscan_scan_unit_m_running_set(VALUE self, VALUE running);

static int rscan_scan_unit_event_handler(unsigned int id, unsigned long long from,
        unsigned long long to, unsigned int flags, void *ctx);
static VALUE rscan_scan_unit_event_receiver_thread(void* arg);
static void* scan_gvl_wrapper(void* arg);

extern VALUE rscan_scan_unit_define(VALUE root) {
    VALUE vClass;
    id_new = rb_intern("new");
    id_abort_exc_set = rb_intern("abort_on_exception=");

    vClass = rb_define_class_under(root, "Unit", rb_cObject);
    rb_define_alloc_func(vClass, rscan_scan_unit_alloc);
    rb_define_method(vClass, "initialize", rscan_scan_unit_m_init, 1);
    rb_define_method(vClass, "running=", rscan_scan_unit_m_running_set, 1);

    return class_unit = vClass;
}

void rscan_scan_unit_running_set(rscan_scan_unit_t *unit, int running) {
    VALUE handler_th, old_state, new_state;
    rscan_event_queue_t *queue;
    Get_Queue(unit->queue, queue);
    old_state = unit->running;
    unit->running = new_state = running ? Qtrue : Qfalse;

    // switching on
    if (!old_state && new_state) {
        handler_th = rb_thread_create(&rscan_scan_unit_event_receiver_thread, (void *) unit);
        rscan_queue_open(queue);
        rb_thread_run(handler_th);
    }

    // switching off
    if (old_state && !new_state) {
        rscan_queue_close(queue);
    }
}

int rscan_scan_unit_running_get(rscan_scan_unit_t *unit) {
    return (int) unit->running;
}

/* rubyscan::Native::Scanner::Unit class functions */
static void rscan_scan_unit_free(rscan_scan_unit_t *pointer) {
    xfree(pointer);
}

static void rscan_scan_unit_mark(rscan_scan_unit_t *pointer) {
    rb_gc_mark(pointer->db);
    rb_gc_mark(pointer->scratch);
    rb_gc_mark(pointer->queue);
    rb_gc_mark(pointer->handler);
}

static VALUE rscan_scan_unit_alloc(VALUE klass) {
    rscan_scan_unit_t *unit;
    VALUE self;
    self = Data_Make_Struct(
            klass, rscan_scan_unit_t, rscan_scan_unit_mark, rscan_scan_unit_free, unit);
    return self;
}

static VALUE rscan_scan_unit_m_init(VALUE self, VALUE oDb) {
    rscan_scan_unit_t *unit;
    rscan_scratch_t *scratch;
    hs_scratch_t *out;
    rscan_db_t *db;
    hs_error_t status;
    pthread_mutexattr_t attr_lock;
    pthread_condattr_t attr_cond;
    VALUE error;
    rb_need_block();
    Get_Scan_Unit(self, unit);
    unit->running = Qfalse;
    unit->handler = rb_block_proc();
    unit->db = oDb;
    unit->scratch = rb_funcall(rscan_class_scratch(), rb_intern("new"), 0);
    unit->queue = rb_funcall(rscan_class_queue(), rb_intern("new"), 0);
    unit->match = &rscan_scan_unit_event_handler;
    pthread_mutexattr_init(&attr_lock);
    pthread_mutexattr_settype(&attr_lock, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutexattr_setprotocol(&attr_lock, PTHREAD_PRIO_INHERIT);
    if (status = pthread_mutex_init(&unit->lock, &attr_lock)) {
        VALUE message;
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
        message = rb_sprintf("pthread_mutex_init: %s", msg);
        error = rb_exc_new(
                rscan_class_thread_error(), StringValuePtr(message), rb_str_strlen(message));
        rscan_thread_error_set(error, status);
        rb_exc_raise(error);
    }
    pthread_mutexattr_destroy(&attr_lock);

    pthread_condattr_init(&attr_cond);
    pthread_condattr_setpshared(&attr_cond, PTHREAD_PROCESS_PRIVATE);
    if (status = pthread_cond_init(&unit->ready, &attr_cond)) {
        VALUE message;
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
        message = rb_sprintf("pthread_cond_init: %s", msg);
        error = rb_exc_new(
                rscan_class_thread_error(), StringValuePtr(message), rb_str_strlen(message));
        rscan_thread_error_set(error, status);
        rb_exc_raise(error);
    }
    pthread_condattr_destroy(&attr_cond);

    Get_Scratch(unit->scratch, scratch);
    Get_Db(oDb, db);
    if (status = hs_alloc_scratch(db->obj, &out)) {
        // handle error
        VALUE message = rb_sprintf("hs_allocate_scratch: %s", rscan_hs_error_message(status));
        error = rb_exc_new(
                rscan_class_hs_error(), StringValuePtr(message), rb_str_strlen(message));
        rscan_hs_error_set(error, status);
        // throw exception
        rb_exc_raise(error);
    }
    scratch->obj = out;

    return self;
}

static VALUE rscan_scan_unit_m_running_set(VALUE self, VALUE running) {
    rscan_scan_unit_t *unit;
    Get_Scan_Unit(self, unit);

    rscan_scan_unit_running_set(unit, running);

    return running;
}

extern hs_error_t rscan_scan_unit_invoke(rscan_scan_unit_t *unit, char *data, int len) {
    VALUE handler;
    scan_gvl_arg_t arg;
    hs_error_t status;

    // get unit lock
    pthread_mutex_lock(&unit->lock);

    // begin scan
    arg.unit = unit;
    arg.data = data;
    arg.len = len;

    status = (hs_error_t) rb_thread_call_without_gvl(
            scan_gvl_wrapper, (void*) &arg, RUBY_UBF_IO, NULL);

    // release unit lock
    pthread_mutex_unlock(&unit->lock);

    return status;
}

static int rscan_scan_unit_event_handler(unsigned int id, unsigned long long from,
        unsigned long long to, unsigned int flags, void *ctx) {
    rscan_scan_unit_t *unit;
    rscan_event_queue_t *queue;
    rscan_match_event_t *event;

    unit = (rscan_scan_unit_t*) ctx;
    Get_Queue(unit->queue, queue);

    event = ALLOC(rscan_match_event_t);

    event->id = id;
    event->from = from;
    event->to = to;
    event->flags = flags;
    event->src = unit;

    rscan_queue_put(queue, event);

    return HS_SUCCESS;
}

static VALUE rscan_scan_unit_event_receiver_thread(void* arg) {
    rscan_scan_unit_t* unit;
    rscan_event_queue_t* queue;
    rscan_match_event_t* event;
    rscan_match_event_t** eventPtr;
    VALUE args, eventObject;
    int status;
    unit = (rscan_scan_unit_t*) arg;
    Get_Queue(unit->queue, queue);

    // set local thread abort-on-exception status
    rb_funcall(rb_thread_current(), id_abort_exc_set, 1, Qtrue);

    while (queue->open) {
        // consume event
        if (status = rscan_queue_shift_nogvlwait(queue, &event)) {
            if (status == RSCAN_QUEUE_FAIL_CLOSED) {
                continue;
            } else {
                // TODO: handle other failure conditions
            }
        }

        eventObject = rb_funcall(rscan_class_event(), id_new, 0);
        Get_Event_Ptr(eventObject, eventPtr);
        *eventPtr = event;
        args = rb_ary_new_from_args(2, eventObject, INT2NUM(unit));

        // call handler
        rb_proc_call(unit->handler, args);
    }


    return Qtrue;
}

static void* scan_gvl_wrapper(void* arg) {
    hs_error_t status;
    scan_gvl_arg_t *args;
    rscan_db_t *db;
    rscan_scratch_t *scratch;

    args = (scan_gvl_arg_t *) arg;
    Get_Db(args->unit->db, db);
    Get_Scratch(args->unit->scratch, scratch);

    status = hs_scan(db->obj, args->data, args->len, 0, scratch->obj, args->unit->match, (void *) args->unit);

    return (void*) status;
}

extern VALUE rscan_class_scan_unit() {
    // TODO throw error if class is not loaded
    return class_unit;
}
