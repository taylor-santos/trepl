#ifndef AST_H
#define AST_H

#include <stdio.h>

#define OK_LIB_DECLARE
#include "ok_lib.h"

struct Value;

typedef struct ok_vec_of(char *) str_v;
typedef struct ok_map_of(const char *, struct Value*) symbols_t;

typedef struct {
    void (*delete)(void *this);
    int (*exec)(void *this, symbols_t *symbols, struct Value **ret_val);
} AST;

typedef struct ast_v ok_vec_of(AST*) ast_v;
typedef struct value_v ok_vec_of(struct Value*) value_v;

typedef struct Value {
    enum {
        VAL_NONE, VAL_INT, VAL_DOUBLE, VAL_FUNC
    } type;
    union {
        int INT;
        double DOUBLE;
        struct Func {
            enum {
                FUNC_BUILTIN, FUNC_DEFINED
            } type;
            str_v *globals;

            union {
                struct Value (*BUILTIN)(value_v args);
                struct ast_v *DEFINED;
            } value;
        } FUNC;
    } value;
    char *name;
    int (*typecmp)(struct Value *v1, struct Value *v2);
    int (*fprint)(FILE *file, struct Value *v);
} Value;

Value
new_Value_none(void);

typedef struct {
    AST super;
    Value *value;
} ASTExpression;

typedef struct ok_vec_of(ASTExpression*) expr_v;

typedef struct {
    AST super;
    str_v idents;
    ASTExpression *expr;
} ASTAssignment;
AST *
new_ASTAssignment(str_v idents, ASTExpression *value);

typedef struct {
    ASTExpression super;
    ASTExpression *func;
    expr_v args;
    Value ret;
} ASTCall;
ASTExpression *
new_ASTCall(ASTExpression *func, expr_v args);

typedef struct {
    ASTExpression super;
    Value value;
} ASTInt;
ASTExpression *
new_ASTInt(int value);
Value
new_Value_int(int value);

typedef struct {
    ASTExpression super;
    Value value;
} ASTDouble;
ASTExpression *
new_ASTDouble(double value);

typedef struct {
    ASTExpression super;
    char *ident;
} ASTVar;
ASTExpression *
new_ASTVar(char *ident);

typedef struct {
    ASTExpression super;
    str_v globals;
    ast_v stmts;
    Value value;
} ASTFunc;
ASTExpression *
new_ASTFunc(str_v globals, ast_v stmts);

#endif
