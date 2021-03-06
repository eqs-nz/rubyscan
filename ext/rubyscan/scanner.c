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
    scanner->phases = rb_ary_new();
    return self;
}

static VALUE rscan_scanner_m_scan(VALUE self, VALUE data) {
    int i, cDataLen;
    char* cData;
    rscan_scanner_t* scanner;
    rscan_scan_unit_t* unit;
    hs_error_t status;
    Check_Type(data, T_STRING);
    Get_Scanner(self, scanner);
    cData = StringValuePtr(data);
    cDataLen = RSTRING_LEN(data);
    for (i = 0; i < RARRAY_LEN(scanner->phases); i++) {
        Get_Scan_Unit(RARRAY_AREF(scanner->phases, i), unit);
        if (status = rscan_scan_unit_invoke(unit, cData, cDataLen)) {
            if (status = HS_SCAN_TERMINATED) {
                continue;
            }
        }
    }

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
    VALUE old_state, new_state;
    Get_Scanner(self, scanner);
    old_state = scanner->running;
    scanner->running = new_state = RTEST(running)? Qtrue : Qfalse;
    // switching on or off
    if ((new_state && !old_state) || (old_state &&!new_state)) {
        int i;
        for (i = 0; i < RARRAY_LEN(scanner->phases); i++) {
            Get_Scan_Unit(RARRAY_AREF(scanner->phases, i), unit);
            rscan_scan_unit_running_set(unit, new_state);
        }
    }
    return new_state;
}

extern VALUE rscan_class_scanner() {
    // TODO throw error if class is not loaded
    return class_scanner;
}
