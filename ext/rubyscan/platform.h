/* scratch.c
 * wrapper class for Hyperscan scratch memory object
 * written by Alex Fulton
 * Sept 2016
 */
#ifndef RSCAN_PLATFORM_H
#define RSCAN_PLATFORM_H

extern VALUE rscan_platform_define(VALUE root);

extern VALUE rscan_class_platform();

#define Get_Platform(value,pointer) \
        Data_Get_Struct(value,hs_platform_info_t,pointer)
#endif
