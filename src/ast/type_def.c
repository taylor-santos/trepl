#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

static void
delete(void *this) {
    ASTTypeDef *ast = this;
    ok_vec_foreach(&ast->type_def.names, char *name) {
            free(name);
        }
    delete_type(&ast->type_def.type);
    ok_vec_deinit(&ast->type_def.names);
    free(ast);
}

static int
type_check(void *this, ExecState *state, Type **ret_type) {
    (void)this;
    (void)state;
    fprintf(stderr, "Type Def type checker not implemented\n");
    return 1;
}

static int
exec(void *this, ExecState *state, Value **ret_val) {
    ASTTypeDef *ast = this;
    int status = 0;
    if (VerifyType(&ast->type_def.type, state->symbols)) {
        return 1;
    }
    ok_vec_foreach(&ast->type_def.names, const char *name) {
            Value *prev_val = ok_map_get(state->symbols, name);
            if (prev_val == NULL) {
                ok_map_put(state->symbols, name, (Value *)&ast->type_def.type);
            } else {
                if (ast->type_def.type.typecmp(&ast->type_def.type, &prev_val->type)) {
                    fprintf(stderr, "error: assigning to variable \"%s\" from type \"", name);
                    prev_val->type.fprint(stderr, &prev_val->type);
                    fprintf(stderr, "\" to incompatible type \"");
                    ast->type_def.type.fprint(stderr, &ast->type_def.type);
                    fprintf(stderr, "\"\n");
                    status = 1;
                }
            }
        }
    if (status) {
        return 1;
    }
    if (ret_val) {
        *ret_val = (Value *)&ast->type_def.type;
    }
    return 0;
}

ASTExpression *
new_ASTTypeDef(TA type_def) {
    ASTTypeDef *ast = malloc(sizeof(*ast));
    if (ast == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *ast = (ASTTypeDef){
            {
                    {
                            delete, exec, type_check
                    }, NULL
            }, type_def
    };
    return (ASTExpression *)ast;
}
