/* scratch.c
 * wrapper class for Hyperscan scratch memory object
 * written by Alex Fulton
 * Sept 2016
 */
#ifndef RSCAN_SCRATCH_H
#define RSCAN_SCRATCH_H

typedef struct scratch {
    hs_scratch_t *obj;
} rscan_scratch_t;

extern VALUE rscan_scratch_define(VALUE root);

extern VALUE rscan_class_scratch();

#define Get_Scratch(value,pointer) \
        Data_Get_Struct(value,rscan_scratch_t,pointer)

#endif
