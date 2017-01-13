/* queue.c
 * threadsafe event queue for Ruby
 * written by Alex Fulton
 * Sept 2016
 */
#include "mod_common.h"

static VALUE module_common = Qnil;

extern VALUE rscan_module_common_define(VALUE root) {
    /* init module */
    VALUE module = rb_define_module_under(root, "Common");
    /* init hyperscan platform info class */
    rscan_platform_define(module);
    /* init error wrappers */
    rscan_hs_error_define(module);
    rscan_thread_error_define(module);
    /* init database class */
    rscan_db_define(module);
    /* init scratch area class */
    rscan_scratch_define(module);

    return module_common = module;
}

extern VALUE rscan_module_common() {
    // TODO throw error if module is not loaded
    return module_common;
}
