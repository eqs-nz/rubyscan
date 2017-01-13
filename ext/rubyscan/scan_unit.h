/* scanner.h
 * pattern scanner object class
 * written by Alex Fulton
 * Sept 2016
 */
#ifndef RSCAN_SCAN_UNIT_H
#define RSCAN_SCAN_UNIT_H

typedef struct scan_unit {
    pthread_mutex_t lock;
    pthread_cond_t ready;
    VALUE running;
    VALUE db;
    VALUE scratch;
    VALUE queue;
    match_event_handler match;
    VALUE handler;
} rscan_scan_unit_t;

VALUE rscan_scan_unit_define(VALUE root);
VALUE rscan_class_scan_unit();

hs_error_t rscan_scan_unit_invoke(rscan_scan_unit_t *unit, char *data, int len);
void rscan_scan_unit_running_set(rscan_scan_unit_t *unit, int running);
int rscan_scan_unit_running_get(rscan_scan_unit_t *unit);

#define Get_Scan_Unit(value,pointer) \
        Data_Get_Struct(value,rscan_scan_unit_t,pointer)

#endif
