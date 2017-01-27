/* scan_unit.h
 * pattern scanner sub-unit class
 * written by Alex Fulton
 * Sept 2016
 */
#include "mod_runtime.h"

typedef struct scan_gvl_arg {
    rscan_scan_unit_t* unit;
    char* data;
    int len;
} scan_gvl_arg_t;

// TODO: move these symbols and their init code to mod_common
static ID id_new;
static ID id_abort_exc_set;

static VALUE class_unit;

static VALUE rscan_scan_unit_alloc(VALUE self);
static VALUE rscan_scan_unit_m_init(VALUE self, VALUE oDb);
static VALUE rscan_scan_unit_m_running_set(VALUE self, VALUE running);
static hs_error_t rscan_scan_unit_do_scan(rscan_scan_unit_t *unit, char *data, int len);

static int rscan_scan_unit_event_handler(unsigned int id, unsigned long long from,
        unsigned long long to, unsigned int flags, void *ctx);
static VALUE rscan_scan_unit_match_handler_thread(void* arg);
static VALUE rscan_scan_unit_operator_thread(void* arg);
static void* scan_gvl_wrapper(void* arg);

extern VALUE rscan_scan_unit_define(VALUE root) {
    VALUE klass;
    id_new = rb_intern("new");
    id_abort_exc_set = rb_intern("abort_on_exception=");

    klass = rb_define_class_under(root, "Unit", rb_cObject);
    rb_define_alloc_func(klass, rscan_scan_unit_alloc);
    rb_define_method(klass, "initialize", rscan_scan_unit_m_init, 1);
    rb_define_method(klass, "running=", rscan_scan_unit_m_running_set, 1);

    return class_unit = klass;
}

void rscan_scan_unit_running_set(rscan_scan_unit_t *unit, int running) {
    VALUE handler_th, old_state, new_state;
    struct event_queue *op_buffer, *op_cache, *handler_queue;
    Get_Event_Queue(unit->op_buffer, op_buffer);
    Get_Event_Queue(unit->op_cache, op_cache);
    Get_Event_Queue(unit->match_buffer, handler_queue);
    
    pthread_mutex_lock(&unit->lock);
    old_state = unit->running;
    unit->running = new_state = running ? Qtrue : Qfalse;

    // switching on
    if (!old_state && new_state) {
        rscan_event_queue_open(handler_queue);
        rscan_event_queue_open(op_buffer);
        rscan_event_queue_open(op_cache);
        rb_thread_run(unit->op_th);
        rb_thread_run(unit->match_th);
    }

    // switching off
    if (old_state && !new_state) {
        rscan_event_queue_close(handler_queue);
    }
    pthread_mutex_unlock(&unit->lock);
}

int rscan_scan_unit_running_get(rscan_scan_unit_t *unit) {
    return (int) unit->running;
}

/* Rubyscan::Runtime::Scanner::Unit class functions */
static void rscan_scan_unit_free(rscan_scan_unit_t *pointer) {
    xfree(pointer);
}

static void rscan_scan_unit_mark(rscan_scan_unit_t *pointer) {
    rb_gc_mark(pointer->db);
    rb_gc_mark(pointer->scratch);
    rb_gc_mark(pointer->op_buffer);
    rb_gc_mark(pointer->op_cache);
    rb_gc_mark(pointer->match_buffer);
    rb_gc_mark(pointer->op_th);
    rb_gc_mark(pointer->match_th);
    rb_gc_mark(pointer->handler_proc);
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
    unit->self = self;
    unit->running = Qfalse;
    unit->handler_proc = rb_block_proc();
    unit->db = oDb;
    unit->op_th = rb_thread_create(&rscan_scan_unit_operator_thread, (void *) unit);
    unit->match_th = rb_thread_create(&rscan_scan_unit_match_handler_thread, (void *) unit);
    unit->scratch = rb_funcall(rscan_class_scratch(), rb_intern("new"), 0);
    unit->op_buffer = rb_funcall(rscan_class_event_queue(), rb_intern("new"), 0);
    unit->op_cache = rb_funcall(rscan_class_event_queue(), rb_intern("new"), 0);
    unit->match_buffer = rb_funcall(rscan_class_event_queue(), rb_intern("new"), 0);
    unit->match_func = &rscan_scan_unit_event_handler;
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

void rscan_scan_unit_push_event(rscan_scan_unit_t *unit, struct scan_event *event) {
    VALUE op_v;
    struct scan_op *op;
    
    op_v = rb_funcall(rscan_class_scan_op(), id_new, 0);
    Get_Scan_Op(op_v, op);
    op->target = unit;
    op->src = event;
    rscan_event_queue_put(unit->op_buffer, op);
}

static hs_error_t rscan_scan_unit_do_scan(rscan_scan_unit_t *unit, char *data, int len) {
    scan_gvl_arg_t arg;
    hs_error_t status;

    // get unit lock
    pthread_mutex_lock(&unit->lock);

    // begin scan
    arg.unit = unit;
    arg.data = data;
    arg.len = len;

    status = (hs_error_t) ((long long)rb_thread_call_without_gvl(
            scan_gvl_wrapper, (void*) &arg, RUBY_UBF_IO, NULL) << 32);

    // release unit lock
    pthread_mutex_unlock(&unit->lock);

    return status;
}

static int rscan_scan_unit_event_handler(unsigned int id, unsigned long long from,
        unsigned long long to, unsigned int flags, void *ctx) {
    struct scan_op *op;
    rscan_scan_unit_t *unit;
    struct event_queue *match_buffer;
    struct match_event *event;

    op = (struct scan_op*) ctx;
    unit = op->target;
    Get_Event_Queue(unit->match_buffer, match_buffer);

    event = ALLOC(struct match_event);
    event->data = ALLOC(struct match_data);
    event->id = rscan_scanner_id_gen_atomic(op->src->src);
    
    event->data->pattern_id = id;
    event->data->from = from;
    event->data->to = to;
    event->data->flags = flags;
    event->src = op;

    rscan_event_queue_put(match_buffer, event);

    return HS_SUCCESS;
}

static VALUE rscan_scan_unit_operator_thread(void* arg) {
    rscan_scan_unit_t* unit;
    struct event_queue *buffer;
    struct event_queue *cache;
    unsigned int status;
    int cDataLen;
    char *cData;
    unit = (rscan_scan_unit_t*) arg;
    Get_Event_Queue(unit->op_buffer, buffer);
    Get_Event_Queue(unit->op_cache, cache);

    // set local thread abort-on-exception status
    rb_funcall(rb_thread_current(), id_abort_exc_set, 1, Qtrue);

    while (buffer->open) {
        struct scan_op *op;
        // consume scan-op from buffer
        if (status = rscan_event_queue_shift_nogvlwait(buffer, &op)) {
            if (status == RSCAN_QUEUE_FAIL_CLOSED) {
                continue;
            } else {
                continue;
                // TODO: handle other failure conditions
            }
        }
        
        // push op onto cache
        rscan_event_queue_put(cache, op);
        
        cData = StringValuePtr(op->src->data);
        cDataLen = RSTRING_LEN(op->src->data);
        
        if (status = rscan_scan_unit_do_scan(unit, cData, cDataLen)) {
            if (status = HS_SCAN_TERMINATED) {
                continue;
            }
            // TODO: handle errors intelligently using exceptions
        }
        
        return Qtrue;
    }


    return Qtrue;
}

static VALUE rscan_scan_unit_match_handler_thread(void* arg) {
    rscan_scan_unit_t* unit;
    struct event_queue *match_buffer;
    struct match_event *event, **event_ptr;
    VALUE args, event_object;
    rscan_queue_err_t status;
    unit = (rscan_scan_unit_t*) arg;
    Get_Event_Queue(unit->match_buffer, match_buffer);

    // set local thread abort-on-exception status
    rb_funcall(rb_thread_current(), id_abort_exc_set, 1, Qtrue);

    while (match_buffer->open) {
        
        // consume event
        if (status = rscan_event_queue_shift_nogvlwait(match_buffer, &event)) {
            if (status == RSCAN_QUEUE_FAIL_CLOSED) {
                continue;
            } else {
                // TODO: handle other failure conditions
            }
        }
        
        // TODO: pool and re-use event wrapper objects
        event_object = rb_funcall(rscan_class_match_event(), id_new, 0);
        Get_Match_Event_Ptr(event_object, event_ptr);
        *event_ptr = event;
        event->self = event_object;
        
        args = rb_ary_new_from_args(2, event_object, unit->self);

        // call handler
        rb_proc_call(unit->handler_proc, args);
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

    status = hs_scan(db->obj, args->data, args->len, 0, scratch->obj, args->unit->match_func, (void *) args->unit);

    return (void*)(unsigned long long) status;
}

extern VALUE rscan_class_scan_unit() {
    // TODO throw error if class is not loaded
    return class_unit;
}
