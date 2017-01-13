/* compiler.c
 * pattern compiler
 * written by Alex Fulton
 * Sept 2016
 */
#include "mod_compile.h"

static VALUE class_expression;

static VALUE rscan_expression_alloc(VALUE self);
static VALUE rscan_expression_m_initialize(VALUE self, VALUE value);
static VALUE rscan_expression_m_setflags(int argc, VALUE* argv, VALUE self);
static VALUE rscan_expression_m_unsetflags(int argc, VALUE* argv, VALUE self);
static VALUE rscan_expression_m_getflag_p(VALUE self, VALUE flag);
static VALUE rscan_expression_m_getflags(VALUE self);
static VALUE rscan_expression_m_setopt(VALUE self, VALUE option, VALUE value);
static VALUE rscan_expression_m_unsetopt(VALUE self, VALUE option);
static VALUE rscan_expression_m_getopt(VALUE self, VALUE option);
static VALUE rscan_expression_m_getopts(VALUE self);
static unsigned int rscan_expression_flag_value(ID flagId);

VALUE rscan_expression_define(VALUE root) {
    rscan_expr_opt_minoffset = rb_intern("min_offset");
    rscan_expr_opt_maxoffset = rb_intern("max_offset");
    rscan_expr_opt_minlength = rb_intern("min_length");

    rscan_expr_flag_caseless = rb_intern("caseless");
    rscan_expr_flag_dotall = rb_intern("dotall");
    rscan_expr_flag_multiline = rb_intern("multiline");
    rscan_expr_flag_singlematch = rb_intern("singlematch");
    rscan_expr_flag_allowempty = rb_intern("allowempty");
    rscan_expr_flag_utf8 = rb_intern("utf8");
    rscan_expr_flag_ucp = rb_intern("ucp");
    rscan_expr_flag_prefilter = rb_intern("prefilter");
    rscan_expr_flag_som_leftmost = rb_intern("som_leftmost");

    /* init expression class */
    class_expression = rb_define_class_under(root, "Expression", rb_cObject);
    rb_define_alloc_func(class_expression, rscan_expression_alloc);
    rb_define_method(class_expression, "initialize", rscan_expression_m_initialize, 1);
    rb_define_method(class_expression, "set_flags", rscan_expression_m_setflags, -1);
    rb_define_method(class_expression, "unset_flags", rscan_expression_m_unsetflags, -1);
    rb_define_method(class_expression, "get_flag", rscan_expression_m_getflag_p, 1);
    rb_define_method(class_expression, "get_flags", rscan_expression_m_getflags, 0);
    rb_define_method(class_expression, "set_opt", rscan_expression_m_setopt, 2);
    rb_define_method(class_expression, "unset_opt", rscan_expression_m_unsetopt, 1);
    rb_define_method(class_expression, "get_opt", rscan_expression_m_getopt, 1);
    rb_define_method(class_expression, "get_opts", rscan_expression_m_getopts, 0);

    return class_expression;
}

void rscan_expression_setflags(rscan_expression_t *self, unsigned int flags) {
    self->flags = self->flags | flags;
}

void rscan_expression_unsetflags(rscan_expression_t *self, unsigned int flags) {
    self->flags = self->flags & ~flags;
}

void rscan_expression_setflag_caseless(rscan_expression_t *self, int value) {
    if (value) {
        rscan_expression_setflags(self, HS_FLAG_CASELESS);
    } else {
        rscan_expression_unsetflags(self, HS_FLAG_CASELESS);
    }
}

void rscan_expression_setflag_dotall(rscan_expression_t *self, int value) {
    if (value) {
        rscan_expression_setflags(self, HS_FLAG_DOTALL);
    } else {
        rscan_expression_unsetflags(self, HS_FLAG_DOTALL);
    }
}

void rscan_expression_setflag_multiline(rscan_expression_t *self, int value) {
    if (value) {
        rscan_expression_setflags(self, HS_FLAG_MULTILINE);
    } else {
        rscan_expression_unsetflags(self, HS_FLAG_MULTILINE);
    }
}

void rscan_expression_setflag_singlematch(rscan_expression_t *self, int value) {
    if (value) {
        rscan_expression_setflags(self, HS_FLAG_SINGLEMATCH);
    } else {
        rscan_expression_unsetflags(self, HS_FLAG_SINGLEMATCH);
    }
}

void rscan_expression_setflag_allowempty(rscan_expression_t *self, int value) {
    if (value) {
        rscan_expression_setflags(self, HS_FLAG_ALLOWEMPTY);
    } else {
        rscan_expression_unsetflags(self, HS_FLAG_ALLOWEMPTY);
    }
}

void rscan_expression_setflag_utf8(rscan_expression_t *self, int value) {
    if (value) {
        rscan_expression_setflags(self, HS_FLAG_UTF8);
    } else {
        rscan_expression_unsetflags(self, HS_FLAG_UTF8);
    }
}

void rscan_expression_setflag_ucp(rscan_expression_t *self, int value) {
    if (value) {
        rscan_expression_setflags(self, HS_FLAG_UCP);
    } else {
        rscan_expression_unsetflags(self, HS_FLAG_UCP);
    }
}

void rscan_expression_setflag_prefilter(rscan_expression_t *self, int value) {
    if (value) {
        rscan_expression_setflags(self, HS_FLAG_PREFILTER);
    } else {
        rscan_expression_unsetflags(self, HS_FLAG_PREFILTER);
    }
}

void rscan_expression_setflag_som_leftmost(rscan_expression_t *self, int value) {
    if (value) {
        rscan_expression_setflags(self, HS_FLAG_SOM_LEFTMOST);
    } else {
        rscan_expression_unsetflags(self, HS_FLAG_SOM_LEFTMOST);
    }
}

unsigned int rscan_expression_getflags(rscan_expression_t *self) {
    return self->flags;
}

int rscan_expression_getflag_p(rscan_expression_t *self, unsigned int flag) {
    return (int) (self->flags & flag);
}

int rscan_expression_getflag_caseless_p(rscan_expression_t *self) {
    return (int) (self->flags & HS_FLAG_CASELESS);
}

int rscan_expression_getflag_dotall_p(rscan_expression_t *self) {
    return (int) (self->flags & HS_FLAG_DOTALL);
}

int rscan_expression_getflag_multiline_p(rscan_expression_t *self) {
    return (int) (self->flags & HS_FLAG_MULTILINE);
}

int rscan_expression_getflag_singlematch_p(rscan_expression_t *self) {
    return (int) (self->flags & HS_FLAG_SINGLEMATCH);
}

int rscan_expression_getflag_allowempty_p(rscan_expression_t *self) {
    return (int) (self->flags & HS_FLAG_ALLOWEMPTY);
}

int rscan_expression_getflag_utf8_p(rscan_expression_t *self) {
    return (int) (self->flags & HS_FLAG_UTF8);
}

int rscan_expression_getflag_ucp_p(rscan_expression_t *self) {
    return (int) (self->flags & HS_FLAG_UCP);
}

int rscan_expression_getflag_prefilter_p(rscan_expression_t *self) {
    return (int) (self->flags & HS_FLAG_PREFILTER);
}

int rscan_expression_getflag_som_leftmost_p(rscan_expression_t *self) {
    return (int) (self->flags & HS_FLAG_SOM_LEFTMOST);
}

void rscan_expression_setopt_minoffset(rscan_expression_t *self, unsigned long long value) {
    self->ext->flags = self->ext->flags | HS_EXT_FLAG_MIN_OFFSET;
    self->ext->min_offset = value;
}

void rscan_expression_setopt_maxoffset(rscan_expression_t *self, unsigned long long value) {
    self->ext->flags = self->ext->flags | HS_EXT_FLAG_MAX_OFFSET;
    self->ext->max_offset = value;
}

void rscan_expression_setopt_minlength(rscan_expression_t *self, unsigned long long value) {
    self->ext->flags = self->ext->flags | HS_EXT_FLAG_MIN_LENGTH;
    self->ext->min_length = value;
}

void rscan_expression_unsetopt_minoffset(rscan_expression_t *self) {
    self->ext->flags = self->ext->flags & ~HS_EXT_FLAG_MIN_OFFSET;
    self->ext->min_offset = 0;
}

void rscan_expression_unsetopt_maxoffset(rscan_expression_t *self) {
    self->ext->flags = self->ext->flags & ~HS_EXT_FLAG_MAX_OFFSET;
    self->ext->max_offset = 0;
}

void rscan_expression_unsetopt_minlength(rscan_expression_t *self) {
    self->ext->flags = self->ext->flags & ~HS_EXT_FLAG_MIN_LENGTH;
    self->ext->min_length = 0;
}

int rscan_expression_getopt_minoffset_p(rscan_expression_t *self) {
    return self->ext->flags & HS_EXT_FLAG_MIN_OFFSET;
}

int rscan_expression_getopt_maxoffset_p(rscan_expression_t *self) {
    return self->ext->flags & HS_EXT_FLAG_MAX_OFFSET;

}

int rscan_expression_getopt_minlength_p(rscan_expression_t *self) {
    return self->ext->flags & HS_EXT_FLAG_MIN_LENGTH;
}

unsigned long long rscan_expression_getopt_minoffset(rscan_expression_t *self) {
    return self->ext->min_offset;
}

unsigned long long rscan_expression_getopt_maxoffset(rscan_expression_t *self) {
    return self->ext->max_offset;
}

unsigned long long rscan_expression_getopt_minlength(rscan_expression_t *self) {
    return self->ext->min_length;
}

char* rscan_expression_value(rscan_expression_t *self) {
    return StringValueCStr(self->value);
}

/* rubyscan::Native::Expression class functions */
static void rscan_expression_free(rscan_expression_t *expr) {
    xfree(expr->ext);
    xfree(expr);
}

static void rscan_expression_mark(rscan_expression_t *expr) {
    rb_gc_mark(expr->value);
}

static VALUE rscan_expression_alloc(VALUE klass) {
    rscan_expression_t* expr;
    VALUE self = Data_Make_Struct(klass, rscan_expression_t, rscan_expression_mark, rscan_expression_free, expr);
    expr->ext = ALLOC(hs_expr_ext_t);
    return self;
}

static VALUE rscan_expression_m_initialize(VALUE self, VALUE value) {
    rscan_expression_t* expr;
    Get_Expression(self, expr);
    SafeStringValue(value);
    expr->value = value;
    expr->flags = 0;
    return self;
}

static VALUE rscan_expression_m_setflags(int argc, VALUE* argv, VALUE self) {
    rscan_expression_t *expr;
    VALUE flag;
    ID flagId;
    int i;
    unsigned int flags = 0;
    Get_Expression(self, expr);
    for (i = 0; i < argc; i++) {
        flag = argv[i];
        Check_Type(flag, T_SYMBOL);
        flagId = SYM2ID(flag);
        flags = flags | rscan_expression_flag_value(flagId);
    }
    expr->flags = expr->flags | flags;
    return self;
}

static VALUE rscan_expression_m_getflag_p(VALUE self, VALUE flag) {
    rscan_expression_t *expr;
    ID flagId;
    Get_Expression(self, expr);
    Check_Type(flag, T_SYMBOL);
    flagId = SYM2ID(flag);
    
    if(rscan_expression_getflag_p(expr,rscan_expression_flag_value(flagId))) {
        return Qtrue;
    } else {
        return Qfalse;
    }
}

static VALUE rscan_expression_m_getflags(VALUE self) {
    rscan_expression_t *expr;
    VALUE result;
    
    Get_Expression(self,expr);
    result = rb_ary_new_capa(9);
    
    if (rscan_expression_getflag_caseless_p(expr)) {
        rb_ary_push(result,ID2SYM(rscan_expr_flag_caseless));
    }
    if (rscan_expression_getflag_dotall_p(expr)) {
        rb_ary_push(result,ID2SYM(rscan_expr_flag_dotall));
    }
    if (rscan_expression_getflag_multiline_p(expr)) {
        rb_ary_push(result,ID2SYM(rscan_expr_flag_multiline));
    }
    if (rscan_expression_getflag_singlematch_p(expr)) {
        rb_ary_push(result,ID2SYM(rscan_expr_flag_singlematch));
    }
    if (rscan_expression_getflag_allowempty_p(expr)) {
        rb_ary_push(result,ID2SYM(rscan_expr_flag_allowempty));
    }
    if (rscan_expression_getflag_utf8_p(expr)) {
        rb_ary_push(result,ID2SYM(rscan_expr_flag_utf8));
    }
    if (rscan_expression_getflag_ucp_p(expr)) {
        rb_ary_push(result,ID2SYM(rscan_expr_flag_ucp));
    }
    if (rscan_expression_getflag_prefilter_p(expr)) {
        rb_ary_push(result,ID2SYM(rscan_expr_flag_prefilter));
    }
    if (rscan_expression_getflag_som_leftmost_p(expr)) {
        rb_ary_push(result,ID2SYM(rscan_expr_flag_caseless));
    }
    return result;
}

static VALUE rscan_expression_m_setopt(VALUE self, VALUE option, VALUE value) {
    rscan_expression_t *expr;
    ID optionId;
    unsigned long long cValue;
    Get_Expression(self, expr);
    Check_Type(option, T_SYMBOL);
    optionId = SYM2ID(option);
    cValue = NUM2ULL(value);
    if (optionId == rscan_expr_opt_minoffset) {
        rscan_expression_setopt_minoffset(expr, cValue);
    } else if (optionId == rscan_expr_opt_maxoffset) {
        rscan_expression_setopt_maxoffset(expr, cValue);
    } else if (optionId == rscan_expr_opt_minlength) {
        rscan_expression_setopt_minlength(expr, cValue);
    } else {
        rb_raise(rb_eArgError, "unrecognized option :%s", rb_id2name(optionId));
        return Qundef;
    }
    return value;
}

static VALUE rscan_expression_m_getopt(VALUE self, VALUE option) {
    rscan_expression_t *expr;
    ID optionId;
    Check_Type(option, T_SYMBOL);
    Get_Expression(self, expr);
    optionId = SYM2ID(option);

    if (optionId == rscan_expr_opt_minoffset) {
        if (rscan_expression_getopt_minoffset_p(expr)) {
            return ULL2NUM(expr->ext->min_offset);
        }
    } else if (optionId == rscan_expr_opt_maxoffset) {
        if (rscan_expression_getopt_maxoffset_p(expr)) {
            return ULL2NUM(expr->ext->max_offset);
        }
    } else if (optionId == rscan_expr_opt_minlength) {
        if (rscan_expression_getopt_minlength_p(expr)) {
            return ULL2NUM(expr->ext->min_length);
        }
    }

    rb_raise(rb_eArgError, "unrecognized option :%s", rb_id2name(optionId));
    return Qundef;
}

static VALUE rscan_expression_m_getopts(VALUE self) {
    rscan_expression_t *expr;
    VALUE opts;
    Get_Expression(self, expr);
    opts = rb_hash_new();

    if (expr->ext->flags & HS_EXT_FLAG_MIN_OFFSET) {
        rb_hash_aset(opts, ID2SYM(rscan_expr_opt_minoffset), ULL2NUM(expr->ext->min_offset));
    }
    if (expr->ext->flags & HS_EXT_FLAG_MAX_OFFSET) {
        rb_hash_aset(opts, ID2SYM(rscan_expr_opt_maxoffset), ULL2NUM(expr->ext->max_offset));
    }
    if (expr->ext->flags & HS_EXT_FLAG_MIN_LENGTH) {
        rb_hash_aset(opts, ID2SYM(rscan_expr_opt_minlength), ULL2NUM(expr->ext->min_length));
    }
    return rb_hash_freeze(opts);
}

static VALUE rscan_expression_m_unsetflags(int argc, VALUE* argv, VALUE self) {
    rscan_expression_t *expr;
    VALUE flag;
    ID flagId;
    int i;
    unsigned int flags = 0;
    Get_Expression(self, expr);
    for (i = 0; i < argc; i++) {
        flag = argv[i];
        Check_Type(flag, T_SYMBOL);
        flagId = SYM2ID(flag);
        flags = flags | rscan_expression_flag_value(flagId);
    }
    expr->flags = expr->flags & ~flags;

    return self;
}

static VALUE rscan_expression_m_unsetopt(VALUE self, VALUE option) {
    rscan_expression_t *expr;
    ID optionId;
    Get_Expression(self, expr);
    Check_Type(option, T_SYMBOL);
    optionId = SYM2ID(option);

    if (optionId == rscan_expr_opt_minoffset) {
        rscan_expression_unsetopt_minoffset(expr);
    } else if (optionId == rscan_expr_opt_maxoffset) {
        rscan_expression_unsetopt_maxoffset(expr);
    } else if (optionId == rscan_expr_opt_minlength) {
        rscan_expression_unsetopt_minlength(expr);
    }
    return self;
}

static unsigned int rscan_expression_flag_value(ID flagId) {
    if (flagId == rscan_expr_flag_caseless) {
        return HS_FLAG_CASELESS;
    } else if (flagId == rscan_expr_flag_dotall) {
        return HS_FLAG_DOTALL;
    } else if (flagId == rscan_expr_flag_multiline) {
        return HS_FLAG_MULTILINE;
    } else if (flagId == rscan_expr_flag_singlematch) {
        return HS_FLAG_SINGLEMATCH;
    } else if (flagId == rscan_expr_flag_allowempty) {
        return HS_FLAG_ALLOWEMPTY;
    } else if (flagId == rscan_expr_flag_utf8) {
        return HS_FLAG_UTF8;
    } else if (flagId == rscan_expr_flag_ucp) {
        return HS_FLAG_UCP;
    } else if (flagId == rscan_expr_flag_prefilter) {
        return HS_FLAG_PREFILTER;
    } else if (flagId == rscan_expr_flag_som_leftmost) {
        return HS_FLAG_SOM_LEFTMOST;
    } else {
        VALUE oError;
        oError = rb_funcall(rscan_class_hs_error(), rb_intern("new"), 0);
        rb_raise(oError, "Flag not recognized: %s", rb_id2name(flagId));
        return 0;
    }
}

VALUE rscan_class_expression() {
    // TODO throw error if class is not loaded
    return class_expression;
}
