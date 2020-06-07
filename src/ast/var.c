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
type_check(void *this, ExecState *state, Type **ret_type) {
    (void)this;
    (void)state;
    ASTVar *ast = this;
    Value *val = ok_map_get(state->symbols, ast->ident);
    if (val == NULL) {
        fprintf(stderr, "error: name \"%s\" is not defined\n", ast->ident);
        return 1;
    }
    if (val->type.init == 0) {
        fprintf(stderr, "error: name \"%s\" used before initialization\n", ast->ident);
        return 1;
    }
    if (ret_type) {
        *ret_type = &val->type;
    }
    return 0;
}

static int
check_assign(void *this, Type *type, ExecState *state) {
    ASTVar *ast = this;
    Value *prev = ok_map_get(state->symbols, ast->ident);
    if (prev && prev->type.typecmp(&prev->type, type)) {
        fprintf(stderr, "error: assigning to variable \"%s\" from type \"", ast->ident);
        prev->type.fprint(stderr, &prev->type);
        fprintf(stderr, "\" to incompatible type \"");
        type->fprint(stderr, type);
        fprintf(stderr, "\"\n");
        return 1;
    }
    ok_map_put(state->symbols, ast->ident, (Value *)type);
    return 0;
}

static int
exec(void *this, ExecState *state, Value **ret_val) {
    ASTVar *ast = this;
    Value *val = ok_map_get(state->symbols, ast->ident);
    if (val == NULL) {
        fprintf(stderr, "error: name \"%s\" is not defined\n", ast->ident);
        return 1;
    } else if (val->type.init == 0) {
        fprintf(stderr, "error: name \"%s\" with type \"", ast->ident);
        val->type.fprint(stderr, &val->type);
        fprintf(stderr, "\" has not been initialized\n");
        return 1;
    } else {
        ast->super.super.value = val;
        if (ret_val) {
            *ret_val = ast->super.super.value;
        }
        return 0;
    }
}

static int
assign(void *this, Value *val, ExecState *state) {
    ASTVar *ast = this;
    Value *prev_val = ok_map_get(state->symbols, ast->ident);
    if (prev_val && prev_val->type.typecmp(&prev_val->type, &val->type)) {
        fprintf(stderr, "error: assigning to variable \"%s\" from type \"", ast->ident);
        prev_val->type.fprint(stderr, &prev_val->type);
        fprintf(stderr, "\" to incompatible type \"");
        val->type.fprint(stderr, &val->type);
        fprintf(stderr, "\"\n");
        return 1;
    }
    ok_map_put(state->symbols, ast->ident, val);
    ast->super.super.value = val;
    return 0;
}

ASTLValue *
new_ASTVar(char *ident) {
    ASTVar *ast = malloc(sizeof(*ast));
    if (ast == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *ast = (ASTVar){
            {
                    {
                            {
                                    delete, exec, type_check
                            }, NULL
                    }, assign, check_assign
            }, ident
    };
    return (ASTLValue *)ast;
}
