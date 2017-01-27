/* scan_event.h
 * input buffer entry class
 * written by Alex Fulton
 * Jan 2017
 */
#ifndef RSCAN_SCAN_EVENT_H
#define RSCAN_SCAN_EVENT_H

struct scan_event {
    unsigned long long id;
    struct scanner *src;
    VALUE data;
    unsigned int matched;
    unsigned int handled;
};

extern VALUE rscan_scan_event_define(VALUE root);
extern VALUE rscan_class_scan_event();

#define Get_Scan_Event(value,pointer) \
        Data_Get_Struct(value,struct scan_event,pointer)
#endif
