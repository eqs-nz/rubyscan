/* hs_error.h
 * wrapper for Hyperscan error type
 * written by Alex Fulton
 * Dec 2016
 */
#include "mod_common.h"

static VALUE class_hs_error = Qnil;

static VALUE rscan_hs_error_alloc(VALUE self);
static VALUE rscan_hs_error_m_initialize(VALUE self, VALUE errorNum);
static VALUE rscan_hs_error_m_value(VALUE self);
static VALUE rscan_hs_error_m_message(VALUE self);

extern void rscan_hs_error_define(VALUE root) {
    /* init hyperscan error class */
    VALUE vClass;
    vClass = rb_define_class_under(root, "HyperscanError", rb_eRuntimeError);
    rb_define_alloc_func(vClass, rscan_hs_error_alloc);
    rb_define_method(vClass, "initialize", rscan_hs_error_m_initialize, 1);
    rb_define_method(vClass, "value", rscan_hs_error_m_value, 0);
    rb_define_method(vClass, "message", rscan_hs_error_m_message, 0);

    class_hs_error = vClass;
}

void rscan_hs_error_set(VALUE self, hs_error_t val) {
    hs_error_t* ptr;
    Get_HS_Error(self, ptr);
    *ptr = val;
}

/* Hyperscan::Native::HyperscanError class functions */
static void rscan_hs_error_free(hs_error_t *pointer) {
    free(pointer);
}

static VALUE rscan_hs_error_alloc(VALUE klass) {
    hs_error_t* pointer;
    VALUE self = Data_Make_Struct(klass, hs_error_t, NULL, rscan_hs_error_free, pointer);
    return self;
}

static VALUE rscan_hs_error_m_initialize(VALUE self, VALUE errorNum) {
    hs_error_t *pError;
    Data_Get_Struct(self, hs_error_t, pError);
    *pError = NUM2INT(errorNum);
    return self;
}

static VALUE rscan_hs_error_m_value(VALUE self) {
    hs_error_t *pError;
    Data_Get_Struct(self, hs_error_t, pError);
    return INT2NUM(*pError);
}

static VALUE rscan_hs_error_m_message(VALUE self) {
    hs_error_t *pError;
    Data_Get_Struct(self, hs_error_t, pError);
    return rb_str_new_cstr(rscan_hs_error_message(*pError));
}

extern const char* rscan_hs_error_message(hs_error_t error) {
    const char *message = "undefined error";
    switch (error) {
        case HS_SUCCESS:
            message = "HS_SUCCESS: operation successful";
            break;
        case HS_INVALID:
            message = "HS_INVALID: a supplied argument was invalid";
            break;
        case HS_NOMEM:
            message = "HS_NOMEM: out of memory";
            break;
        case HS_SCAN_TERMINATED:
            message = "HS_SCAN_TERMINATED: scanning terminated after a match was found";
            break;
        case HS_COMPILER_ERROR:
            message = "HS_COMPILER_ERROR: the pattern compiler returned an error";
            break;
        case HS_DB_VERSION_ERROR:
            message = "HS_DB_VERSION_ERROR: supplied database is from a different hyperscan compiler version";
            break;
        case HS_DB_PLATFORM_ERROR:
            message = "HS_DB_PLATFORM_ERROR: supplied database was compiled for different hardware";
            break;
        case HS_BAD_ALIGN:
            message = "HS_BAD_ALIGN: a supplied argument was not correctly aligned";
            break;
        case HS_BAD_ALLOC:
            message = "HS_BAD_ALLOC: an allocator function returned misaligned memory";
        case HS_SCRATCH_IN_USE:
            message = "HS_SCRATCH_IN_USE: another thread is using the supplied scratch memory";
            break;
    }
    return message;
}

extern VALUE rscan_class_hs_error() {
    // TODO throw error if class is not loaded
    return class_hs_error;
}


