/* compile_error.c
 * compile error wrapper
 * written by Alex Fulton
 * Sept 2016
 */
#include "mod_compile.h"

static VALUE class_compile_error;

static VALUE rscan_compile_error_alloc(VALUE self);
static VALUE rscan_compile_error_m_message(VALUE self);

extern VALUE rscan_compile_error_define(VALUE root) {
    /* init compile error class */
    VALUE vClass = rb_define_class_under(root, "CompileError", rb_eRuntimeError);
    rb_define_alloc_func(vClass, rscan_compile_error_alloc);
    rb_define_method(vClass, "message", rscan_compile_error_m_message, 0);
    return class_compile_error = vClass;
}

extern void rscan_compile_error_set(VALUE self, hs_compile_error_t* val) {
    hs_compile_error_t** ptr;
    Get_Compile_Error(self, ptr);
    *ptr = val;
}

/* HS::Compile::CompileError class functions */
static void rscan_compile_error_free(hs_compile_error_t **pointer) {
    if (pointer != NULL) {
        hs_free_compile_error(*pointer);
    }
    xfree(pointer);
}

static VALUE rscan_compile_error_alloc(VALUE klass) {
    hs_compile_error_t** error;
    VALUE self = Data_Make_Struct(klass, hs_compile_error_t*, NULL, rscan_compile_error_free, error);
    *error = NULL;
    return self;
}

static VALUE rscan_compile_error_m_message(VALUE self) {
    hs_compile_error_t **error;
    Get_Compile_Error(self, error);

    if (*error != NULL) {
        return rb_str_new_cstr((*error)->message);
    } else {
        return rb_str_new_cstr("object not initializzed");
    }
}

extern VALUE rscan_class_compile_error() {
    // TODO throw error if class is not loaded
    return class_compile_error;
}
