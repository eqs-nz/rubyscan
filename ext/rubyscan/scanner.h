/* scanner.h
 * pattern scanner object class
 * written by Alex Fulton
 * Sept 2016
 */
#ifndef RSCAN_SCANNER_H
#define RSCAN_SCANNER_H

typedef struct scanner {
    VALUE running;
    VALUE phases;
} rscan_scanner_t;

VALUE rscan_scanner_define(VALUE root);
VALUE rscan_class_scanner();

#define Get_Scanner(value,pointer) \
        Data_Get_Struct(value,rscan_scanner_t,pointer)

#endif
