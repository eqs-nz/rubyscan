/* scan_op.h
 * scan operation event class
 * written by Alex Fulton
 * Jan 2017
 */
#ifndef RSCAN_SCAN_OP_H
#define RSCAN_SCAN_OP_H

struct scan_op {
    unsigned long long id;
    VALUE self;
    struct scan_event *src;
    struct scan_unit *target;
};

VALUE rscan_scan_op_define(VALUE root);
VALUE rscan_class_scan_op();

#define Get_Scan_Op(value,pointer) \
        Data_Get_Struct(value,struct scan_op,pointer)
#endif
