/* hs_error.h
 * wrapper for Hyperscan error type
 * written by Alex Fulton
 * Dec 2016
 */
#ifndef RSCAN_HS_ERROR_H
#define RSCAN_HS_ERROR_H

extern void rscan_hs_error_define();
extern VALUE rscan_class_hs_error();

void rscan_hs_error_set(VALUE self, hs_error_t val);
extern const char* rscan_hs_error_message(hs_error_t error);

#define Get_HS_Error(value,pointer) \
        Data_Get_Struct(value,hs_error_t,pointer)

#endif
