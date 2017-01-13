/* compiler.h
 * rubyscan pattern compiler
 * written by Alex Fulton
 * Sept 2016
 */
#ifndef RSCAN_COMPILER_H
#define RSCAN_COMPILER_H

typedef struct rscan_compiler {
    unsigned int mode; /* HS_MODE_BLOCK, HS_MODE_STREAM, etc */
    VALUE platform;
} rscan_compiler_t;

VALUE rscan_compiler_define(VALUE root);
VALUE rscan_class_compiler();

ID rscan_compiler_mode_undef;
ID rscan_compiler_mode_block;
ID rscan_compiler_mode_nostream;
ID rscan_compiler_mode_stream;
ID rscan_compiler_mode_vectored;
ID rscan_compiler_mode_som_horizon_large;
ID rscan_compiler_mode_som_horizon_medium;
ID rscan_compiler_mode_som_horizon_small;

#define Get_Compiler(value,pointer) \
        Data_Get_Struct(value,rscan_compiler_t,pointer)
#endif
