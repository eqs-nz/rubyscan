/* scanner.c
 * pattern scanner object class
 * written by Alex Fulton
 * Sept 2016
 */
#include "mod_runtime.h"

static VALUE class_scanner;

static VALUE scanner_management_thread(void *arg);

static VALUE rscan_scanner_alloc(VALUE self);
static VALUE rscan_scanner_m_init(VALUE self);
static VALUE rscan_scanner_m_scan(VALUE self, VALUE data);
static VALUE rscan_scanner_m_phases_get(VALUE self);
static VALUE rscan_scanner_m_phases_set(VALUE self, VALUE phaseArray);
static VALUE rscan_scanner_m_running_get(VALUE self);
static VALUE rscan_scanner_m_running_set(VALUE self, VALUE running);

extern VALUE rscan_scanner_define(VALUE root) {
    VALUE cScanner;

    cScanner = rb_define_class_under(root, "Scanner", rb_cObject);
    rb_define_alloc_func(cScanner, rscan_scanner_alloc);
    rb_define_method(cScanner, "initialize", rscan_scanner_m_init, 0);
    rb_define_method(cScanner, "scan", rscan_scanner_m_scan, 1);
    rb_define_method(cScanner, "phases", rscan_scanner_m_phases_get, 0);
    rb_define_method(cScanner, "phases=", rscan_scanner_m_phases_set, -2);
    rb_define_method(cScanner, "running", rscan_scanner_m_running_get, 0);
    rb_define_method(cScanner, "running=", rscan_scanner_m_running_set, 1);
    rscan_scan_unit_define(cScanner);

    return class_scanner = cScanner;
}

/* rubyscan::Native::Scanner class functions */
static void rscan_scanner_mark(rscan_scanner_t *pScanner) {
    rb_gc_mark(pScanner->phases);
}

static VALUE rscan_scanner_alloc(VALUE klass) {
    rscan_scanner_t *pScanner;
    VALUE self = Data_Make_Struct(klass, rscan_scanner_t, rscan_scanner_mark, RUBY_DEFAULT_FREE, pScanner);
    pScanner->phases = rb_ary_new();
    return self;
}

static VALUE rscan_scanner_m_init(VALUE self) {
    rscan_scanner_t *scanner;
    Get_Scanner(self, scanner);
    scanner->self = self;
    scanner->running = Qfalse;
    scanner->in_buffer = rb_funcall(rscan_class_event_queue(), rb_intern("new"), 0);
    scanner->in_cache = rb_funcall(rscan_class_event_queue(), rb_intern("new"), 0);
    scanner->manager_th = rb_thread_create(&scanner_management_thread, (void *)scanner);
    scanner->phases = rb_ary_new();
    scanner->next_id = 0;
    return self;
}

static VALUE rscan_scanner_m_scan(VALUE self, VALUE data) {
    rscan_scanner_t* scanner;
    rscan_scan_unit_t* unit;
    struct scan_event* event;
    struct event_queue *buffer;
    Check_Type(data, T_STRING);
    Get_Scanner(self, scanner);
    Get_Event_Queue(scanner->in_buffer, buffer);
    event = ALLOC(struct scan_event);
    event->id = rscan_scanner_id_gen_atomic(scanner);
    event->src = scanner;
    event->data = data;

    rscan_event_queue_put(buffer, event);

    return self;
}

static VALUE rscan_scanner_m_phases_get(VALUE self) {
    rscan_scanner_t *scanner;
    Get_Scanner(self, scanner);
    return scanner->phases;
}

static VALUE rscan_scanner_m_phases_set(VALUE self, VALUE phaseArray) {
    rscan_scanner_t *scanner;
    Check_Type(phaseArray, T_ARRAY);
    Get_Scanner(self, scanner);
    scanner->phases = phaseArray;
    return phaseArray;
}

static VALUE rscan_scanner_m_running_get(VALUE self) {
    rscan_scanner_t *scanner;
    Get_Scanner(self, scanner);
    if (scanner->running) {
        return Qtrue;
    } else {
        return Qfalse;
    }
}

static VALUE rscan_scanner_m_running_set(VALUE self, VALUE running) {
    rscan_scanner_t *scanner;
    rscan_scan_unit_t *unit;
    struct event_queue *buffer, *cache;
    VALUE old_state, new_state;
    Get_Scanner(self, scanner);
    Get_Event_Queue(scanner->in_buffer, buffer);
    Get_Event_Queue(scanner->in_cache, cache);
    old_state = scanner->running;
    scanner->running = new_state = RTEST(running) ? Qtrue : Qfalse;
    // switching on
    if (new_state && !old_state) {
        int i;
        // first fire up units, then start handling scans
        for (i = 0; i < RARRAY_LEN(scanner->phases); i++) {
            Get_Scan_Unit(RARRAY_AREF(scanner->phases, i), unit);
            rscan_scan_unit_running_set(unit, new_state);
        }
        rscan_event_queue_open(buffer);
        rscan_event_queue_open(cache);
        rb_thread_run(scanner->manager_th);
    }
    // switching off
    if (old_state &&!new_state) {
        int i;
        // turn off incoming scans, then shut down units
        rscan_event_queue_close(buffer);
        rscan_event_queue_close(cache);
        for (i = 0; i < RARRAY_LENINT(scanner->phases); i++) {
            Get_Scan_Unit(RARRAY_AREF(scanner->phases, i), unit);
            rscan_scan_unit_running_set(unit, new_state);
        }
    }
    return new_state;
}

static VALUE scanner_management_thread(void *arg) {
    rscan_scanner_t *scanner;
    struct event_queue *buffer, *cache;
    struct scan_event *event;
    rscan_scan_unit_t *first_unit;
    rscan_queue_err_t status;
    scanner = (rscan_scanner_t *) arg;
    Get_Event_Queue(scanner->in_buffer, buffer);
    Get_Event_Queue(scanner->in_cache, cache);
    Get_Scan_Unit(RARRAY_AREF(scanner->phases, 0), first_unit);
    
    while (buffer->open) {
        // consume event
        if (status = rscan_event_queue_shift_nogvlwait(buffer, &event)) {
            if (status == RSCAN_QUEUE_FAIL_CLOSED) {
                continue;
            } else {
                // TODO: handle other failure conditions
            }
        }
        // condition: *event points to a new event
        rscan_event_queue_put(cache, event);
        rscan_scan_unit_push_event(first_unit, event);
    }
    
    return Qtrue;

}

extern VALUE rscan_class_scanner() {
    // TODO throw error if class is not loaded
    return class_scanner;
}
