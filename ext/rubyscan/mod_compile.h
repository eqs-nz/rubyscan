/* compiler.h
 * rubyscan pattern compiler
 * written by Alex Fulton
 * Sept 2016
 */
#ifndef RSCAN_MOD_COMPILE_H
#define RSCAN_MOD_COMPILE_H

#include "mod_common.h"
#include "compiler.h"
#include "expression.h"
#include "compile_error.h"

extern VALUE rscan_module_compile_define(VALUE root);
extern VALUE rscan_module_compile();

#endif