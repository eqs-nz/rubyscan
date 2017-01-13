/* mod_scan.c
 * rubyscan runtime module
 * written by Alex Fulton
 * Sept 2016
 */
#include "mod_runtime.h"

static VALUE module_runtime = Qnil;

extern VALUE rscan_module_runtime_define(VALUE root) {
    VALUE module = rb_define_module_under(root, "Runtime");

    /* init scanner class */
    rscan_scanner_define(module);
    /* init event class */
    rscan_event_define(module);
    /* init queue class */
    rscan_queue_define(module);

    return module_runtime = module;
}

extern VALUE rscan_module_runtime() {
    // TODO throw error if class is not loaded
    return module_runtime;
}