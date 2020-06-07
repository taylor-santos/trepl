#ifndef AST_H
#define AST_H

#include <stdio.h>

#include "value.h"

typedef struct Values ok_map_of(const char *, Value*) Values;
typedef struct Types ok_map_of(const char *, Type*) Types;
typedef struct ta_v ok_vec_of(TA) ta_v;

typedef struct {
    Values *symbols;
    Value *func;
    Value *ret;
} ExecState;

typedef struct {
    void (*delete)(void *this);
    int (*exec)(void *this, ExecState *state, Value **ret_val);
    int (*type_check)(void *this, ExecState *state, Type **ret_type);
} AST;

typedef struct {
    AST super;
    Value *value;
} ASTExpression;

typedef struct {
    ASTExpression super;
    int (*assign)(void *this, Value *value, ExecState *state);
    int (*check_assign)(void *this, Type *type, ExecState *state);
} ASTLValue;

typedef struct ast_v ok_vec_of(AST*) ast_v;

typedef struct ok_vec_of(ASTExpression*) expr_v;

typedef struct {
    AST super;
    ASTExpression *expr;
    Value value;
} ASTReturn;
AST *
new_ASTReturn(ASTExpression *value);

typedef struct {
    ASTExpression super;
    ASTLValue *lvalue;
    ASTExpression *rvalue;
} ASTAssignment;
ASTExpression *
new_ASTAssignment(ASTLValue *lvalue, ASTExpression *rvalue);

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
    ASTLValue super;
    char *ident;
} ASTVar;
ASTLValue *
new_ASTVar(char *ident);

typedef struct {
    ASTExpression super;
    TA type_def;
} ASTTypeDef;
ASTExpression *
new_ASTTypeDef(TA type_def);

typedef struct {
    ASTExpression super;
    ta_v args;
    str_v globals;
    ast_v stmts;
    Value value;
    Type *type;
} ASTFunc;
ASTExpression *
new_ASTFunc(ta_v args, str_v globals, ast_v stmts);

#endif
