#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

static void
delete(void *this) {
    ASTDouble *ast = this;
    free(ast);
}

static int
type_check(void *this, ExecState *state, Type **ret_type) {
    (void)state;
    ASTDouble *ast = this;
    if (ret_type) {
        *ret_type = &ast->value.type;
    }
    return 0;
}

static int
exec(void *this, ExecState *state, Value **ret_val) {
    (void)state;
    ASTDouble *ast = this;
    ast->super.value = &ast->value;
    if (ret_val) {
        *ret_val = &ast->value;
    }
    return 0;
}

static int
fprint(FILE *file, Value *v) {
    return fprintf(file, "%f", v->OBJECT.BUILTIN.DOUBLE);
}

static Value
new_Value_double(double value) {
    return (Value){
            new_object_Type("double"), .OBJECT={
                    OBJ_BUILTIN, .BUILTIN={
                            BUILTIN_DOUBLE, .DOUBLE=value
                    }
            }, fprint
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
                            delete, exec, type_check
                    }, NULL
            }, new_Value_double(value)
    };
    ast->value.type.init = 1;
    return (ASTExpression *)ast;
}
