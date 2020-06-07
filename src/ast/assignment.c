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

    /*
    Value *prev_val = ok_map_get(state->symbols, ident);
    if (prev_val &&
            prev_val->type.typecmp(&prev_val->type, &val->type)) {
        fprintf(stderr,
                "error: assigning to variable \"%s\" from type "
                "\"",
                ident);
        prev_val->type.fprint(stderr, &prev_val->type);
        fprintf(stderr, "\" to incompatible type \"");
        val->type.fprint(stderr, &val->type);
        fprintf(stderr, "\"\n");
        status = 1;
    } else {
        ok_map_put(state->symbols, ident, val);
    }
    return status;
     */
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
