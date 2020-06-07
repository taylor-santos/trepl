#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

static void
delete(void *this) {
    ASTReturn *ast = this;
    ast->value->super.delete(ast->value);
    free(ast);
}

static int
type_check(void *this, ExecState *state, Type **ret_type) {
    (void)this;
    (void)state;
    fprintf(stderr, "error: return type check not implemented\n");
    return 1;
}

static int
exec(void *this, ExecState *state, Value **ret_val) {
    (void)this;
    (void)ret_val;
    if (state->func == NULL) {
        fprintf(stderr, "error: 'return' outside function\n");
        return 1;
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
            }, value
    };
    return (AST *)ast;
}
