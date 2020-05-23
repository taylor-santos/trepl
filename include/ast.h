#ifndef AST_H
#define AST_H

#include <stdio.h>

#define OK_LIB_DECLARE
#include "ok_lib.h"

typedef struct Value {
    enum {
        VAL_INT, VAL_DOUBLE, VAL_FUNC
    } type;
    union {
        int INT;
        double DOUBLE;
        struct Func *FUNC;
    } value;
    char *name;
    int (*typecmp)(const struct Value *v1, const struct Value *v2);
    int (*fprint)(FILE *file, const struct Value *v);
} Value;

typedef struct ok_vec_of(const Value*) value_v;

typedef struct Func {
    enum {
        FUNC_BUILTIN
    } type;

    union {
        Value (*BUILTIN)(value_v args);
    } value;
} Func;

typedef struct ok_map_of(const char *, const Value*) symbols_t;

typedef struct {
    void (*delete)(void *this);
    int (*exec)(void *this, symbols_t *symbols, const Value **ret_val);
} AST;

typedef struct {
    AST super;
    const Value *value;
} ASTExpression;

typedef struct ok_vec_of(ASTExpression*) expr_vec;

typedef struct {
    AST super;
    char **idents;
    ASTExpression *expr;
} ASTAssignment;
AST *
new_ASTAssignment(char **idents, ASTExpression *value);

typedef struct {
    ASTExpression super;
    ASTExpression *func;
    expr_vec args;
    Value ret;
} ASTCall;
ASTExpression *
new_ASTCall(ASTExpression *func, expr_vec args);

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

#endif
