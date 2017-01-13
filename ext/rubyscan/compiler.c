/* compiler.c
 * rubyscan pattern compiler
 * written by Alex Fulton
 * Sept 2016
 */
#include "mod_compile.h"

static VALUE class_compiler = Qnil;

static VALUE rscan_compiler_alloc(VALUE self);
static VALUE rscan_compiler_m_initialize(VALUE self);
static VALUE rscan_compiler_m_mode_get(VALUE self);
static VALUE rscan_compiler_m_mode_set(VALUE self, VALUE value);
static VALUE rscan_compiler_m_platform_get(VALUE self);
static VALUE rscan_compiler_m_platform_set(VALUE self, VALUE value);
static VALUE rscan_compiler_m_compile(VALUE self, VALUE arg);
static VALUE rscan_compiler_compile_single(VALUE self, VALUE expression);
static VALUE rscan_compiler_compile_multi(VALUE self, VALUE expressions);

static void rscan_compiler_raise_compile_error(const char* pattern, hs_compile_error_t *error);
static void rscan_compiler_raise_hs_error(hs_error_t error);

static unsigned int rscan_compiler_mode_value(ID modeId);
static ID rscan_compiler_mode_id(unsigned int value);

extern VALUE rscan_compiler_define(VALUE root) {
    VALUE vClass;
    rscan_compiler_mode_undef = rb_intern("undef");
    rscan_compiler_mode_block = rb_intern("block");
    rscan_compiler_mode_nostream = rb_intern("nostream");
    rscan_compiler_mode_stream = rb_intern("stream");
    rscan_compiler_mode_vectored = rb_intern("vectored");
    rscan_compiler_mode_som_horizon_large = rb_intern("som_horizon_large");
    rscan_compiler_mode_som_horizon_medium = rb_intern("som_horizon_medium");
    rscan_compiler_mode_som_horizon_small = rb_intern("som_horizon_small");

    vClass = rb_define_class_under(root, "Compiler", rb_cObject);
    rb_define_alloc_func(vClass, rscan_compiler_alloc);
    rb_define_method(vClass, "initialize", rscan_compiler_m_initialize, 0);
    rb_define_method(vClass, "compile", rscan_compiler_m_compile, 1);
    rb_define_method(vClass, "mode", rscan_compiler_m_mode_get, 0);
    rb_define_method(vClass, "mode=", rscan_compiler_m_mode_set, 1);
    rb_define_method(vClass, "platform", rscan_compiler_m_platform_get, 0);
    rb_define_method(vClass, "platform=", rscan_compiler_m_platform_set, 1);

    return class_compiler = vClass;
}

/* HS::Compiler::Compiler class functions */
static void rscan_compiler_free(rscan_compiler_t *pointer) {
    free(pointer);
}

static void rscan_compiler_mark(rscan_compiler_t *compiler) {
    rb_gc_mark(compiler->platform);
}

static VALUE rscan_compiler_alloc(VALUE klass) {
    rscan_compiler_t* pCompiler;
    VALUE self = Data_Make_Struct(klass, rscan_compiler_t, rscan_compiler_mark, rscan_compiler_free, pCompiler);
    return self;
}

static VALUE rscan_compiler_m_initialize(VALUE self) {
    rscan_compiler_t *pCompiler;
    Get_Compiler(self, pCompiler);
    pCompiler->mode = HS_MODE_BLOCK;
    pCompiler->platform = rb_funcall(rscan_class_platform(), rb_intern("new"), 0);

    return self;
}

static VALUE rscan_compiler_m_mode_get(VALUE self) {
    rscan_compiler_t *pCompiler;
    Get_Compiler(self, pCompiler);

    return ID2SYM(rscan_compiler_mode_id(pCompiler->mode));
}

static VALUE rscan_compiler_m_mode_set(VALUE self, VALUE mode) {
    rscan_compiler_t *pCompiler;
    Get_Compiler(self, pCompiler);
    Check_Type(mode, T_SYMBOL);
    pCompiler->mode = rscan_compiler_mode_value(SYM2ID(mode));

    return mode;
}

static VALUE rscan_compiler_m_platform_get(VALUE self) {
    rscan_compiler_t *pCompiler;
    Get_Compiler(self, pCompiler);

    return pCompiler->platform;
}

static VALUE rscan_compiler_m_platform_set(VALUE self, VALUE value) {
    rscan_compiler_t *pCompiler;
    Get_Compiler(self, pCompiler);
    // TODO type check value
    pCompiler->platform = value;

    return value;
}

static unsigned int rscan_compiler_mode_value(ID modeId) {
    if (modeId == rscan_compiler_mode_block) {
        return HS_MODE_BLOCK;
    } else if (modeId == rscan_compiler_mode_nostream) {
        return HS_MODE_NOSTREAM;
    } else if (modeId == rscan_compiler_mode_stream) {
        return HS_MODE_STREAM;
    } else if (modeId == rscan_compiler_mode_vectored) {
        return HS_MODE_VECTORED;
    } else if (modeId == rscan_compiler_mode_som_horizon_large) {
        return HS_MODE_SOM_HORIZON_LARGE;
    } else if (modeId == rscan_compiler_mode_som_horizon_medium) {
        return HS_MODE_SOM_HORIZON_MEDIUM;
    } else if (modeId == rscan_compiler_mode_som_horizon_small) {
        return HS_MODE_SOM_HORIZON_SMALL;
    } else {
        rb_raise(rb_eArgError, "Compiler mode not recognized: :%s", rb_id2name(modeId));
        return 0;
    }
}

static ID rscan_compiler_mode_id(unsigned int value) {
    if (value == HS_MODE_BLOCK) {
        return rscan_compiler_mode_block;
    } else if (value == HS_MODE_NOSTREAM) {
        return rscan_compiler_mode_nostream;
    } else if (value == HS_MODE_STREAM) {
        return rscan_compiler_mode_stream;
    } else if (value == HS_MODE_VECTORED) {
        return rscan_compiler_mode_vectored;
    } else if (value == HS_MODE_SOM_HORIZON_LARGE) {
        return rscan_compiler_mode_som_horizon_large;
    } else if (value == HS_MODE_SOM_HORIZON_MEDIUM) {
        return rscan_compiler_mode_som_horizon_medium;
    } else if (value == HS_MODE_SOM_HORIZON_SMALL) {
        return rscan_compiler_mode_som_horizon_small;
    } else {
        return rscan_compiler_mode_undef;
    }
}

static VALUE rscan_compiler_m_compile(VALUE self, VALUE arg) {
    if (RB_TYPE_P(arg, T_ARRAY)) {
        return rscan_compiler_compile_multi(self, arg);
    } else {
        return rscan_compiler_compile_single(self, arg);
    }
}

static VALUE rscan_compiler_compile_single(VALUE self, VALUE expression) {
    rscan_expression_t* expr;
    char* pattern;
    VALUE oDb;
    hs_database_t* db;
    hs_error_t status;
    hs_compile_error_t* pError;
    hs_platform_info_t* pPlatform;
    rscan_compiler_t *pCompiler;

    Get_Compiler(self, pCompiler);
    Get_Platform(pCompiler->platform, pPlatform);
    Get_Expression(expression, expr);
    pattern = StringValueCStr(expr->value);

    status = hs_compile(pattern, expr->flags, pCompiler->mode,
            pPlatform, &db, &pError);
    if (status) {
        if (status == HS_COMPILER_ERROR) {
            /* compile error exception handler */
            rscan_compiler_raise_compile_error(pattern, pError);
            return Qundef;
        } else {
            /* default exception handler */
            rscan_compiler_raise_hs_error(status);
            return Qundef;
        }
    }
    oDb = rb_funcall(rscan_class_db(), rb_intern("new"), 0);
    rscan_db_set(oDb, db);

    return oDb;
}

static VALUE rscan_compiler_compile_multi(VALUE self, VALUE expressions) {

    return self;
}

static void rscan_compiler_raise_compile_error(const char* pattern, hs_compile_error_t *error) {
    ///////////////////////////////
    // FIXME MEMORY LEAK: *error //
    ///////////////////////////////
    VALUE message = rb_sprintf(
            "Hyperscan failed to compile expression /%s/: %s", pattern, error->message);
    VALUE vError = rb_exc_new(
            rscan_class_compile_error(), StringValuePtr(message), rb_str_strlen(message));
    rscan_compile_error_set(vError, error);
    rb_exc_raise(vError);
}

static void rscan_compiler_raise_hs_error(hs_error_t error) {
    VALUE vError, message;
    message = rb_sprintf("rubyscan compiler failed to run: %s", rscan_hs_error_message(error));
    vError = rb_exc_new(
            rscan_class_hs_error(), StringValuePtr(message), rb_str_strlen(message));
    rscan_hs_error_set(vError, error);
    rb_exc_raise(vError);
}

extern VALUE rscan_class_compiler() {
    // TODO throw error if class is not loaded
    return class_compiler;
}
