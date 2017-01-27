/* scanner.h
 * pattern scanner object class
 * written by Alex Fulton
 * Sept 2016
 */
#ifndef RSCAN_SCANNER_H
#define RSCAN_SCANNER_H

typedef struct scanner {
    unsigned long long next_id;
    VALUE self;
    VALUE in_buffer;
    VALUE in_cache;
    VALUE running;
    VALUE phases;
    VALUE manager_th;
} rscan_scanner_t;

VALUE rscan_scanner_define(VALUE root);
VALUE rscan_class_scanner();

inline unsigned long long rscan_scanner_id_gen_atomic(rscan_scanner_t *scanner) {
    return __atomic_fetch_add(&scanner->next_id, 1, __ATOMIC_ACQ_REL);
}

#define Get_Scanner(value,pointer) \
        Data_Get_Struct(value,rscan_scanner_t,pointer)

#endif
