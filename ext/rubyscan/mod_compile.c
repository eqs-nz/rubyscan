/* mod_compile.c
 * rubyscan compile-time module
 * written by Alex Fulton
 * Sept 2016
 */
#include "mod_compile.h"

static VALUE module_compile = Qnil;

extern VALUE rscan_module_compile_define(VALUE root) {
    VALUE module = rb_define_module_under(root, "Compile");
    /* init compiler class */
    rscan_compiler_define(module);
    /* init expression class */
    rscan_expression_define(module);
    /* init compiler error class */
    rscan_compile_error_define(module);

    return module_compile = module;
}

extern VALUE rscan_module_compile() {
    // TODO throw error if module is not loaded
    return module_compile;
}
