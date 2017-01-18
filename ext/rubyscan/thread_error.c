/* queue.c
 * threadsafe event queue for Ruby
 * written by Alex Fulton
 * Sept 2016
 */
#include "mod_common.h"

static VALUE class_thread_error = Qnil;

static VALUE rscan_thread_error_alloc(VALUE self);
static VALUE rscan_thread_error_m_value(VALUE self);

extern void rscan_thread_error_set(VALUE self, int val) {
    int* ptr;
    Get_Thread_Error(self, ptr);
    *ptr = val;
}

extern VALUE rscan_thread_error_define(VALUE root) {
    /* init rubyscan error class */
    VALUE klass;
    klass = rb_define_class_under(root, "ThreadError", rb_eRuntimeError);
    rb_define_alloc_func(klass, rscan_thread_error_alloc);
    rb_define_method(klass, "value", rscan_thread_error_m_value, 0);

    return class_thread_error = klass;
}

/* rubyscan::Native::ThreadError class functions */
static void rscan_thread_error_free(hs_error_t *pointer) {
    xfree(pointer);
}

static VALUE rscan_thread_error_alloc(VALUE klass) {
    int* pointer;
    VALUE self = Data_Make_Struct(klass, int, NULL, rscan_thread_error_free, pointer);
    return self;
}

static VALUE rscan_thread_error_m_value(VALUE self) {
    int *val;
    Data_Get_Struct(self, hs_error_t, val);
    return INT2NUM(*val);
}

extern VALUE rscan_class_thread_error() {
    // TODO throw error if class is not loaded
    return class_thread_error;
}
