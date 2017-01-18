/* scan_event.h
 * input buffer entry class
 * written by Alex Fulton
 * Jan 2017
 */
#ifndef RSCAN_SCAN_EVENT_H
#define RSCAN_SCAN_EVENT_H

struct scan_event {
    rscan_event_t head;
    VALUE data;
    unsigned int matched;
    unsigned int handled;
} rscan_scan_event_t;

extern VALUE rscan_scan_event_define(VALUE root);
extern VALUE rscan_class_scan_event();

#define Get_Scan_Event(value,pointer) \
        Data_Get_Struct(value,rscan_scan_event_t,pointer)
#endif
