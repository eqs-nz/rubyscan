/* scan_unit.h
 * pattern scanner sub-unit class
 * written by Alex Fulton
 * Sept 2016
 */
#ifndef RSCAN_SCAN_UNIT_H
#define RSCAN_SCAN_UNIT_H

//TODO: add parent reference as scanners have 1->N relation to units
typedef struct scan_unit {
    pthread_mutex_t lock;
    pthread_cond_t ready;
    VALUE self;
    VALUE running;
    VALUE db;
    VALUE scratch;
    VALUE op_buffer;
    VALUE op_cache;
    VALUE match_buffer;
    VALUE op_th;
    VALUE match_th;
    match_event_handler match_func;
    VALUE handler_proc;
} rscan_scan_unit_t;

VALUE rscan_scan_unit_define(VALUE root);
VALUE rscan_class_scan_unit();

void rscan_scan_unit_running_set(rscan_scan_unit_t *unit, int running);
int rscan_scan_unit_running_get(rscan_scan_unit_t *unit);
void rscan_scan_unit_push_event(rscan_scan_unit_t *unit, struct scan_event *event);

#define Get_Scan_Unit(value,pointer) \
        Data_Get_Struct(value,rscan_scan_unit_t,pointer)

#endif
