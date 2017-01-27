/* event.c
 * parent event class
 * written by Alex Fulton
 * Sept 2016
 */
#include "mod_runtime.h"

static VALUE class_event;

VALUE rscan_event_define(VALUE root) {
    VALUE klass = rb_define_class_under(root, "Event", rb_cObject);
    return class_event = klass;
}

VALUE rscan_class_event() {
    // TODO throw error if class is not loaded
    return class_event;
}
