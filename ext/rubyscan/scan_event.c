/* scan_event.c
 * input buffer entry class
 * written by Alex Fulton
 * Jan 2017
 */
#include "mod_runtime.h"

static VALUE class_scan_event;

//static void rscan_scan_event_free(struct scan_event **pointer);
static VALUE rscan_scan_event_alloc(VALUE klass);

extern VALUE rscan_scan_event_define(VALUE root) {
    VALUE klass = rb_define_class_under(root, "ScanEvent", rscan_class_event());
    rb_define_alloc_func(klass, rscan_scan_event_alloc);
    return class_scan_event = klass;
}

/* Scan::Runtime::ScanEvent class functions */
static void rscan_scan_event_free(struct scan_event *pointer) {
    xfree(pointer);
}

static VALUE rscan_scan_event_alloc(VALUE klass) {
    struct scan_event* event;
    VALUE self = Data_Make_Struct(klass, struct scan_event, NULL, rscan_scan_event_free, event);
    return self;
}

extern VALUE rscan_class_scan_event() {
    // TODO throw error if class is not loaded
    return class_scan_event;
}
