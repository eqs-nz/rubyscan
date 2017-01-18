/* scratch.c
 * wrapper class for Hyperscan scratch memory object
 * written by Alex Fulton
 * Sept 2016
 */
#include "mod_common.h"

static VALUE class_platform;

static VALUE rscan_platform_alloc(VALUE self);
static VALUE rscan_platform_m_initialize(VALUE self);
static VALUE rscan_platform_m_populate(VALUE self);
static void rscan_platform_populate(hs_platform_info_t *pPlatform);

extern VALUE rscan_platform_define(VALUE root) {
    VALUE klass = rb_define_class_under(root, "PlatformInfo", rb_cObject);
    rb_define_alloc_func(klass, rscan_platform_alloc);
    rb_define_method(klass, "initialize", rscan_platform_m_initialize, 0);
    rb_define_method(klass, "populate", rscan_platform_m_populate, 0);
    return class_platform = klass;
}

/* rubyscan::Native::PlatformInfo class functions */
static void rscan_platform_free(hs_platform_info_t *pointer) {
    xfree(pointer);
}

static VALUE rscan_platform_alloc(VALUE klass) {
    hs_platform_info_t* pPlatform;
    VALUE self = Data_Make_Struct(klass, hs_platform_info_t, NULL, rscan_platform_free, pPlatform);
    return self;
}

static VALUE rscan_platform_m_initialize(VALUE self) {
    hs_platform_info_t *pPlatform;
    Get_Platform(self, pPlatform);
    rscan_platform_populate(pPlatform);
    return self;
}

static VALUE rscan_platform_m_populate(VALUE self) {
    hs_platform_info_t *pPlatform;
    Get_Platform(self, pPlatform);
    rscan_platform_populate(pPlatform);
    return self;
}

static void rscan_platform_populate(hs_platform_info_t *pPlatform) {
    hs_error_t status;
    VALUE oError;
    if (status = hs_populate_platform(pPlatform)) {
        oError = rb_funcall(rscan_class_hs_error(), rb_intern("new"), 1, INT2NUM(status));
        rb_raise(oError, "hs_populate_platform failed: %s", rscan_hs_error_message(status));
    }
}

extern VALUE rscan_class_platform() {
    // TODO throw error if class is not loaded
    return class_platform;
}
