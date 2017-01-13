/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "rubyscan.h"

static VALUE module_hs = Qnil;

extern VALUE rscan_module_hs() {
    return module_hs;
}

extern void Init_rubyscan() {
    VALUE module = rb_define_module("Scan");
    /* init common components */
    rscan_module_common_define(module);
    /* init compiler class */
    rscan_module_compile_define(module);
    /* init scanner class */
    rscan_module_runtime_define(module);

    module_hs = module;
}
