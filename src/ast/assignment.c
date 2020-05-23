#include <stdlib.h>
#include <stdio.h>

#include "ast.h"
#include "list.h"

static void
delete(void *this) {
    ASTAssignment *ast = this;
    ast->expr->super.delete(ast->expr);
    size_t len = list_length(ast->idents);
    for (size_t i = 0; i < len; i++) {
        free(ast->idents[i]);
    }
    delete_list(ast->idents);
    free(ast);
}

static int
exec(void *this, symbols_t *symbols, const Value **ret_val) {
    (void)ret_val;
    int status = 0;
    ASTAssignment *ast = this;
    if (ast->expr->super.exec(ast->expr, symbols, NULL)) {
        return 1;
    }
    const Value *val = ast->expr->value;
    size_t n = list_length(ast->idents);
    for (size_t i = 0; i < n; i++) {
        char *ident = ast->idents[i];
        const Value *prev_val = ok_map_get(symbols, ident);
        if (prev_val && prev_val->typecmp(prev_val, val)) {
            fprintf(stderr,
                    "error: assigning to variable \"%s\" from type "
                    "\"%s\" to incompatible type \"%s\"\n",
                    ident,
                    prev_val->name,
                    val->name);
            status = 1;
        } else {
            ok_map_put(symbols, ident, val);
        }
    }
    return status;
}

AST *
new_ASTAssignment(char **idents, ASTExpression *value) {
    ASTAssignment *ast = malloc(sizeof(*ast));
    if (ast == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *ast = (ASTAssignment){
            {
                    delete,
                    exec
            },
            idents,
            value
    };
    return (AST *)ast;
}
