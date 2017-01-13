/* event.c
 * pattern match event class
 * written by Alex Fulton
 * Sept 2016
 */
#include "mod_runtime.h"

static VALUE class_event;

static void rscan_event_free(rscan_match_event_t **pointer);
static VALUE rscan_event_alloc(VALUE klass);

extern VALUE rscan_event_define(VALUE root) {
    VALUE vClass = rb_define_class_under(root, "MatchEvent", rb_cObject);
    rb_define_alloc_func(vClass, rscan_event_alloc);
    return class_event = vClass;
}

/* rubyscan::Native::MatchEvent class functions */
static void rscan_event_free(rscan_match_event_t **pointer) {
    xfree(*pointer);
    xfree(pointer);
}

static VALUE rscan_event_alloc(VALUE klass) {
    rscan_match_event_t** pEvent;
    VALUE self = Data_Make_Struct(klass, rscan_match_event_t*, NULL, rscan_event_free, pEvent);
    return self;
}

extern VALUE rscan_class_event() {
    // TODO throw error if class is not loaded
    return class_event;
}
