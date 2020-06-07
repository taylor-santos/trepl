#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

static void
delete(void *this) {
    ASTReturn *ast = this;
    ast->expr->super.delete(ast->expr);
    free(ast);
}

static int
type_check(void *this, ExecState *state, Type **ret_type) {
    (void)this;
    (void)state;
    ASTReturn *ast = this;
    if (state->func == NULL) {
        fprintf(stderr, "error: 'return' outside function\n");
        return 1;
    }
    if (ast->expr != NULL) {
        Type *type;
        if (ast->expr->super.type_check(ast->expr, state, &type)) {
            return 1;
        }
        ast->value.type = *type;
    } else {
        ast->value = new_Value_none();
    }

    if (state->func->type.FUNC.ret_type == NULL) {
        state->func->type.FUNC.ret_type = &ast->value.type;
    } else {
        fprintf(stderr, "TODO: multiple return statements\n");
    }
    return 0;
}

static int
exec(void *this, ExecState *state, Value **ret_val) {
    (void)this;
    (void)ret_val;
    ASTReturn *ast = this;
    if (state->func == NULL) {
        fprintf(stderr, "error: 'return' outside function\n");
        return 1;
    }
    if (ast->expr != NULL) {
        Value *val;
        if (ast->expr->super.exec(ast->expr, state, &val)) {
            return 1;
        }
        ast->value = *val;
    }
    state->ret = &ast->value;
    if (ret_val) {
        *ret_val = &ast->value;
    }
    return 0;
}

AST *
new_ASTReturn(ASTExpression *value) {
    ASTReturn *ast = malloc(sizeof(*ast));
    if (ast == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *ast = (ASTReturn){
            {
                    delete, exec, type_check
            }, value, new_Value_none()
    };
    return (AST *)ast;
}
