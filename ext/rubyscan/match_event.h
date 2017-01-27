/* match_event.h
 * pattern match event class
 * written by Alex Fulton
 * Sept 2016
 */
#ifndef RSCAN_MATCH_EVENT_H
#define RSCAN_MATCH_EVENT_H

struct match_event {
    unsigned long long id;
    VALUE self;
    struct scan_op *src;
    struct match_data *data;
};

struct match_data {
    unsigned int pattern_id;
    unsigned int flags;
    unsigned long long from;
    unsigned long long to;
};

extern VALUE rscan_match_event_define(VALUE root);
extern VALUE rscan_class_match_event();

#define Get_Match_Event_Ptr(value,pointer) \
        Data_Get_Struct(value,struct match_event*,pointer)
#endif
