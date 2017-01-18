/* scratch.c
 * wrapper class for Hyperscan scratch memory object
 * written by Alex Fulton
 * Sept 2016
 */
#include "mod_common.h"

static VALUE class_scratch;

static VALUE rscan_scratch_alloc(VALUE self);
static VALUE rscan_scratch_m_initialize(VALUE self);

extern VALUE rscan_scratch_define(VALUE root) {
    VALUE klass = rb_define_class_under(root, "ScratchArea", rb_cObject);
    rb_define_alloc_func(klass, rscan_scratch_alloc);
    rb_define_method(klass, "initialize", rscan_scratch_m_initialize, 0);
    return class_scratch = klass;
}

/* rubyscan::Native::ScratchArea class functions */
static void rscan_scratch_free(rscan_scratch_t *pointer) {
    xfree(pointer);
}

static VALUE rscan_scratch_alloc(VALUE klass) {
    rscan_scratch_t* data;
    VALUE self = Data_Make_Struct(klass, rscan_scratch_t, NULL, rscan_scratch_free, data);
    return self;
}

static VALUE rscan_scratch_m_initialize(VALUE self) {
    rscan_scratch_t* pScratch;
    Get_Scratch(self, pScratch);
    pScratch->obj = NULL;
    return self;
}

extern VALUE rscan_class_scratch() {
    // TODO throw error if class is not loaded
    return class_scratch;
}
