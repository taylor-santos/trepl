#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

static void
delete(void *this) {
    ASTInt *ast = this;
    free(ast);
}

static int
type_check(void *this, ExecState *state, Type **ret_type) {
    (void)state;
    ASTInt *ast = this;
    if (ret_type) {
        *ret_type = &ast->value.type;
    }
    return 0;
}

static int
exec(void *this, ExecState *state, Value **ret_val) {
    (void)state;
    ASTInt *ast = this;
    ast->super.value = &ast->value;
    if (ret_val) {
        *ret_val = &ast->value;
    }
    return 0;
}

static int
fprint(FILE *file, Value *v) {
    return fprintf(file, "%d", v->OBJECT.BUILTIN.INT);
}

Value
new_Value_int(int value) {
    return (Value){
            new_object_Type("int"), .OBJECT={
                    OBJ_BUILTIN, .BUILTIN={
                            BUILTIN_INT, .INT=value
                    }
            }, fprint
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
                            delete, exec, type_check
                    }, NULL
            }, new_Value_int(value)
    };
    ast->value.type.init = 1;
    return (ASTExpression *)ast;
}
