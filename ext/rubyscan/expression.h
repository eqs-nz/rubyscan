/* compiler.h
 * rubyscan pattern compiler
 * written by Alex Fulton
 * Sept 2016
 */
#ifndef RSCAN_EXPRESSION_H
#define RSCAN_EXPRESSION_H

#define EXPR_ID_DEFAULT 0

typedef struct expression {
    VALUE value;
    unsigned int id;
    unsigned int flags;
    hs_expr_ext_t* ext;
} rscan_expression_t;

VALUE rscan_expression_define(VALUE root);

VALUE rscan_class_expression();

void rscan_expression_setflags(rscan_expression_t *self, unsigned int flags);
void rscan_expression_unsetflags(rscan_expression_t *self, unsigned int flags);
void rscan_expression_setflag_caseless(rscan_expression_t *self, int value);
void rscan_expression_setflag_dotall(rscan_expression_t *self, int value);
void rscan_expression_setflag_multiline(rscan_expression_t *self, int value);
void rscan_expression_setflag_singlematch(rscan_expression_t *self, int value);
void rscan_expression_setflag_allowempty(rscan_expression_t *self, int value);
void rscan_expression_setflag_utf8(rscan_expression_t *self, int value);
void rscan_expression_setflag_ucp(rscan_expression_t *self, int value);
void rscan_expression_setflag_prefilter(rscan_expression_t *self, int value);
void rscan_expression_setflag_som_leftmost(rscan_expression_t *self, int value);
unsigned int rscan_expression_getflags(rscan_expression_t *self);
int rscan_expression_getflag_p(rscan_expression_t *self, unsigned int flag);
int rscan_expression_getflag_caseless_p(rscan_expression_t *self);
int rscan_expression_getflag_dotall_p(rscan_expression_t *self);
int rscan_expression_getflag_multiline_p(rscan_expression_t *self);
int rscan_expression_getflag_singlematch_p(rscan_expression_t *self);
int rscan_expression_getflag_allowempty_p(rscan_expression_t *self);
int rscan_expression_getflag_utf8_p(rscan_expression_t *self);
int rscan_expression_getflag_ucp_p(rscan_expression_t *self);
int rscan_expression_getflag_prefilter_p(rscan_expression_t *self);
int rscan_expression_getflag_som_leftmost_p(rscan_expression_t *self);

void rscan_expression_setopt_minoffset(rscan_expression_t *self, unsigned long long value);
void rscan_expression_setopt_maxoffset(rscan_expression_t *self, unsigned long long value);
void rscan_expression_setopt_minlength(rscan_expression_t *self, unsigned long long value);
void rscan_expression_unsetopt_minoffset(rscan_expression_t *self);
void rscan_expression_unsetopt_maxoffset(rscan_expression_t *self);
void rscan_expression_unsetopt_minlength(rscan_expression_t *self);
int rscan_expression_getopt_minoffset_p(rscan_expression_t *self);
int rscan_expression_getopt_maxoffset_p(rscan_expression_t *self);
int rscan_expression_getopt_minlength_p(rscan_expression_t *self);
unsigned long long rscan_expression_getopt_minoffset(rscan_expression_t *self);
unsigned long long rscan_expression_getopt_maxoffset(rscan_expression_t *self);
unsigned long long rscan_expression_getopt_minlength(rscan_expression_t *self);

ID rscan_expr_opt_minoffset;
ID rscan_expr_opt_maxoffset;
ID rscan_expr_opt_minlength;

ID rscan_expr_flag_caseless;
ID rscan_expr_flag_dotall;
ID rscan_expr_flag_multiline;
ID rscan_expr_flag_singlematch;
ID rscan_expr_flag_allowempty;
ID rscan_expr_flag_utf8;
ID rscan_expr_flag_ucp;
ID rscan_expr_flag_prefilter;
ID rscan_expr_flag_som_leftmost;

#define Get_Expression(value,pointer) \
        Data_Get_Struct(value,rscan_expression_t,pointer)
#endif
