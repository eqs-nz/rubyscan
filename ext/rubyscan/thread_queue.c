/* thread_queue.c
 * queue type which handles threads
 * written by Alex Fulton
 * Jan 2017
 */
#include "mod_runtime.h"

static VALUE class_thread_queue;

/* prototype declarations */
static VALUE rscan_thread_queue_alloc(VALUE self);
static VALUE rscan_thread_queue_m_initialize(VALUE self);

/* public interface */
VALUE rscan_thread_queue_define(VALUE root) {
    VALUE klass = rb_define_class_under(root, "ThreadQueue", rscan_class_queue());
    rb_define_alloc_func(klass, rscan_thread_queue_alloc);
    rb_define_method(klass, "initialize", rscan_thread_queue_m_initialize, 0);
    return class_thread_queue = klass;
}

/* Scan::Runtime::ThreadQueue class functions */
static VALUE rscan_thread_queue_alloc(VALUE klass) {
    return rscan_queue_alloc(klass);
}

static VALUE rscan_thread_queue_m_initialize(VALUE self) {
    rb_call_super(0, NULL);
    return self;
}

extern VALUE rscan_class_queue() {
    // TODO throw error if class is not loaded
    return class_thread_queue;
}
