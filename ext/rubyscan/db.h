/* db.c
 * wrapper class for rubyscan database object
 * written by Alex Fulton
 * Dec 2016
 */
#ifndef RSCAN_DB_H
#define RSCAN_DB_H

extern VALUE rscan_cDatabase;

typedef struct db {
    hs_database_t *obj;
} rscan_db_t;

extern VALUE rscan_db_define(VALUE root);
extern void rscan_db_set(VALUE self, hs_database_t *val);

extern VALUE rscan_class_db();

#define Get_Db(value,pointer) \
        Data_Get_Struct(value,rscan_db_t,pointer)
#endif
