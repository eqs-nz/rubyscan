/* db.c
 * wrapper class for rubyscan database object
 * written by Alex Fulton
 * Dec 2016
 */
#include "mod_common.h"

static VALUE class_db;

static VALUE rscan_db_alloc(VALUE self);
static VALUE rscan_db_m_initialize(VALUE self);

extern VALUE rscan_db_define(VALUE root) {
    class_db = rb_define_class_under(root, "Database", rb_cObject);
    rb_define_alloc_func(class_db, rscan_db_alloc);
    rb_define_method(class_db, "initialize", rscan_db_m_initialize, 0);
    return class_db;
}

/* rubyscan::Native::Database class functions */
static void rscan_db_free(rscan_db_t *pDb) {
    if (pDb->obj != NULL) {
        hs_free_database(pDb->obj);
    }
    xfree(pDb);
}

static VALUE rscan_db_alloc(VALUE klass) {
    rscan_db_t *pointer;
    VALUE self = Data_Make_Struct(klass, rscan_db_t, NULL, rscan_db_free, pointer);
    return self;
}

static VALUE rscan_db_m_initialize(VALUE self) {
    rscan_db_t *pDb;
    Get_Db(self, pDb);
    pDb->obj = NULL;
    return self;
}

extern void rscan_db_set(VALUE self, hs_database_t *val) {
    rscan_db_t *pDb;
    Get_Db(self, pDb);
    pDb->obj = val;
}

extern VALUE rscan_class_db() {
    // TODO throw error if class is not loaded
    return class_db;
}
