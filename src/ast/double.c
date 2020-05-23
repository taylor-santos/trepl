#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

static void
delete(void *this) {
    ASTDouble *ast = this;
    free(ast);
}

static int
exec(void *this, symbols_t *symbols, const Value **ret_val) {
    (void)symbols;
    ASTDouble *ast = this;
    ast->super.value = &ast->value;
    if (ret_val) {
        *ret_val = &ast->value;
    }
    return 0;
}

static int
typecmp(const Value *v1, const Value *v2) {
    (void)v1;
    return v2->type == VAL_INT;
}

static int
fprint(FILE *file, const Value *v) {
    return fprintf(file, "%f", v->value.DOUBLE);
}

static Value
new_double_val(double value) {
    return (Value){
            VAL_DOUBLE, .value.DOUBLE = value,
            "double",
            typecmp,
            fprint
    };
}

ASTExpression *
new_ASTDouble(double value) {
    ASTDouble *ast = malloc(sizeof(*ast));
    if (ast == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *ast = (ASTDouble){
            {
                    {
                            delete,
                            exec
                    },
                    NULL
            },
            new_double_val(value)
    };
    return (ASTExpression *)ast;
}
