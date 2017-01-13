/* common.h
 * common types and routines for rubyscan.rb
 * written by Alex Fulton
 * Sept 2016
 */
#ifndef RSCAN_THREAD_ERROR_H
#define RSCAN_THREAD_ERROR_H

VALUE rscan_eThreadError;

VALUE rscan_thread_error_define(VALUE root);
VALUE rscan_class_thread_error();

void rscan_thread_error_set(VALUE self, int val);

#define Get_Thread_Error(value,pointer) \
        Data_Get_Struct(value,int,pointer)

#endif
