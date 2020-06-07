#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

static void
delete(void *this) {
    ASTAssignment *ast = this;
    ast->lvalue->super.super.delete(ast->lvalue);
    ast->rvalue->super.delete(ast->rvalue);
    free(ast);
}

static int
type_check(void *this, ExecState *state, Type **ret_type) {
    (void)this;
    (void)state;
    ASTAssignment *ast = this;
    Type *type;
    if (ast->rvalue->super.type_check(ast->rvalue, state, &type)) {
        return 1;
    }
    if (ast->lvalue->check_assign(ast->lvalue, type, state)) {
        return 1;
    }
    if (ret_type) {
        *ret_type = type;
    }
    return 0;
}

static int
exec(void *this, ExecState *state, Value **ret_val) {
    ASTAssignment *ast = this;
    Value *val;
    if (ast->rvalue->super.exec(ast->rvalue, state, &val)) {
        return 1;
    }
    if (ast->lvalue->assign(ast->lvalue, val, state)) {
        return 1;
    }
    if (ret_val) {
        *ret_val = val;
    }
    return 0;
}

ASTExpression *
new_ASTAssignment(ASTLValue *lvalue, ASTExpression *rvalue) {
    ASTAssignment *ast = malloc(sizeof(*ast));
    if (ast == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *ast = (ASTAssignment){
            {
                    {
                            delete, exec, type_check
                    }, NULL
            }, lvalue, rvalue
    };
    return (ASTExpression *)ast;
}
