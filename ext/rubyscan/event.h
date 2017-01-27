/* scan_event.h
 * pattern match event class
 * written by Alex Fulton
 * Sept 2016
 */
#ifndef RSCAN_EVENT_H
#define RSCAN_EVENT_H

struct event {
    unsigned long long id;
    VALUE self;
    void *src;
    void *data;
};

extern VALUE rscan_event_define(VALUE root);
extern VALUE rscan_class_event();

#define Get_Event(value,pointer) \
        Data_Get_Struct(value,struct event*,pointer)
#endif
