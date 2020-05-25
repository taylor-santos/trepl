#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

static void
delete(void *this) {
    ASTInt *ast = this;
    free(ast);
}

static int
exec(void *this, symbols_t *symbols, Value **ret_val) {
    (void)symbols;
    ASTInt *ast = this;
    ast->super.value = &ast->value;
    if (ret_val) {
        *ret_val = &ast->value;
    }
    return 0;
}

static int
typecmp(Value *v1, Value *v2) {
    (void)v1;
    return v2->type != VAL_INT;
}

static int
fprint(FILE *file, Value *v) {
    return fprintf(file, "%d", v->value.INT);
}

Value
new_Value_int(int value) {
    return (Value){
            VAL_INT, .value.INT = value,
            "int",
            typecmp,
            fprint
    };
}

ASTExpression *
new_ASTInt(int value) {
    ASTInt *ast = malloc(sizeof(*ast));
    if (ast == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *ast = (ASTInt){
            {
                    {
                            delete,
                            exec
                    },
                    NULL
            },
            new_Value_int(value)
    };
    return (ASTExpression *)ast;
}
