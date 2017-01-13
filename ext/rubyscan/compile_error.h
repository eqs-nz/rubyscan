/* compile_error.h
 * rubyscan compile error wrapper
 * written by Alex Fulton
 * Sept 2016
 */
#ifndef RSCAN_COMPILE_ERROR_H
#define RSCAN_COMPILE_ERROR_H

VALUE rscan_compile_error_define(VALUE root);
VALUE rscan_class_compile_error();

void rscan_compile_error_set(VALUE self, hs_compile_error_t* val);

#define Get_Compile_Error(value,pointer) \
        Data_Get_Struct(value,hs_compile_error_t*,pointer)
#endif