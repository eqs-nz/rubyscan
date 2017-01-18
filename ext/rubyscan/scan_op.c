/* scan_op.c
 * scan operation event class
 * written by Alex Fulton
 * Jan 2017
 */
#include "mod_runtime.h"

static VALUE class_scan_op;

static void rscan_scan_op_free(rscan_match_event_t **pointer);
static VALUE rscan_event_alloc(VALUE klass);

extern VALUE rscan_scan_op_define(VALUE root) {
    VALUE klass = rb_define_class_under(root, "ScanOp", rscan_class_event());
    rb_define_alloc_func(klass, rscan_event_alloc);
    return class_scan_op_event = klass;
}

/* Scan::Runtime::ScanOp class functions */
static void rscan_scan_op_free(struct scan_op *pointer) {
    xfree(pointer);
}

static VALUE rscan_scan_op_alloc(VALUE klass) {
    struct scan_op* pEvent;
    VALUE self = Data_Make_Struct(klass, struct scan_op, NULL, rscan_scan_op_free, pEvent);
    return self;
}

extern VALUE rscan_class_scan_op() {
    // TODO throw error if class is not loaded
    return class_scan_op;
}
