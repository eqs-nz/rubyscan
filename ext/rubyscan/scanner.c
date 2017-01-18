/* scanner.c
 * pattern scanner object class
 * written by Alex Fulton
 * Sept 2016
 */
#include "mod_runtime.h"

static VALUE class_scanner;

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
    scanner->running = Qfalse;
    scanner->in_buffer = rb_funcall(rscan_class_event_queue(), rb_intern("new"), 0);
    scanner->phases = rb_ary_new();
    scanner->next_id = 0;
    return self;
}

static VALUE rscan_scanner_m_scan(VALUE self, VALUE data) {
    int i/*, cDataLen*/;
    //char* cData;
    rscan_scanner_t* scanner;
    rscan_scan_unit_t* unit;
    rscan_scan_event_t* event;
    struct event_queue *buffer;
    hs_error_t status;
    Check_Type(data, T_STRING);
    Get_Scanner(self, scanner);
    Get_Event_Queue(scanner->in_buffer, buffer);
    event = ALLOC(rscan_scan_event_t);
    event.head.id = __atomic_fetch_add(&scanner->next_id, 1, __ATOMIC_ACQ_REL));
    event.head.src = scanner;
    //cData = StringValuePtr(data);
    //cDataLen = RSTRING_LEN(data);
    event.data = data;

    rscan_queue_put(buffer, event);

    /*
    for (i = 0; i < RARRAY_LEN(scanner->phases); i++) {
        Get_Scan_Unit(RARRAY_AREF(scanner->phases, i), unit);
        if (status = rscan_scan_unit_invoke(unit, cData, cDataLen)) {
            if (status = HS_SCAN_TERMINATED) {
                continue;
            }
        }
    }*/

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
    rscan_event_queue_t *buffer;
    VALUE old_state, new_state;
    Get_Scanner(self, scanner);
    Get_Event_Queue(buffer, scanner->in_buffer);
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
        scanner->manager = rb_thread_new(&scanner_management_thread, (void *)scanner);
        rscan_queue_open(buffer);
        rb_thread_run(manager);
    }
    // switching off
    if (old_state &&!new_state) {
        int i;
        // turn off incoming scans, then shut down units
        rscan_queue_close(buffer);
        for (i = 0; i < RARRAY_LEN(scanner->phases); i++) {
            Get_Scan_Unit(RARRAY_AREF(scanner->phases, i), unit);
            rscan_scan_unit_running_set(unit, new_state);
        }
    }
    return new_state;
}

static VALUE scanner_management_thread(void *arg) {
    rscan_scanner_t *scanner;
    rscan_event_queue_t *queue;
    rscan_scan_event_t *event;
    VALUE first_unit;
    scanner = (rscan_scanner_t *) arg;
    Get_Event_Queue(scanner->in_buffer, queue);
    Get_Scan_Unit(RARRAY_AREF(scanner->phases, 0), first_unit);
    
    while (queue->open) {
        // consume event
        if (status = rscan_queue_shift_nogvlwait(queue, (rscan_event_t **)&event)) {
            if (status == RSCAN_QUEUE_FAIL_CLOSED) {
                continue;
            } else {
                // TODO: handle other failure conditions
            }
        }
        // condition: *event points to a new event
        rscan_scan_unit_invoke(first_unit, )
    }
    
    return Qtrue;

}

extern VALUE rscan_class_scanner() {
    // TODO throw error if class is not loaded
    return class_scanner;
}
