/* scan_op.h
 * scan operation event class
 * written by Alex Fulton
 * Jan 2017
 */
#ifndef RSCAN_SCANOP_H
#define RSCAN_SCANOP_H

struct scan_op {
    struct event head;
    struct scan_event src;
    struct scan_unit target;
}

extern VALUE rscan_scan_op_define(VALUE root);
extern VALUE rscan_class_scan_op();

#define Get_Scan_Op(value,pointer) \
        Data_Get_Struct(value,struct scan_op,pointer)
#endif
