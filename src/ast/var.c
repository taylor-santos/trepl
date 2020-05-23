#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

static void
delete(void *this) {
    ASTVar *ast = this;
    free(ast->ident);
    free(ast);
}

static int
exec(void *this, symbols_t *symbols, const Value **ret_val) {
    (void)symbols;
    ASTVar *ast = this;
    if (ok_map_contains(symbols, ast->ident)) {
        ast->super.value = ok_map_get(symbols, ast->ident);
        if (ret_val) {
            *ret_val = ast->super.value;
        }
        return 0;
    } else {
        fprintf(stderr, "error: name \"%s\" is not defined\n", ast->ident);
        return 1;
    }
}

ASTExpression *
new_ASTVar(char *ident) {
    ASTVar *ast = malloc(sizeof(*ast));
    if (ast == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *ast = (ASTVar){
            {
                    {
                            delete,
                            exec
                    },
                    NULL
            },
            ident
    };
    return (ASTExpression *)ast;
}
