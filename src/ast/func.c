#include <stdlib.h>
#include <stdio.h>

#include "ast.h"
#include "type.h"

static void
delete(void *this) {
    ASTFunc *ast = this;
    ok_vec_deinit(&ast->value.type.FUNC.args);
    ok_vec_foreach(&ast->args, TA arg) {
            ok_vec_foreach(&arg.names, char *name) {
                    free(name);
                }
            ok_vec_deinit(&arg.names);
            delete_type(&arg.type);
        }
    ok_vec_deinit(&ast->args);
    ok_vec_foreach(&ast->globals, char *str) {
            free(str);
        }
    ok_vec_deinit(&ast->globals);
    ok_vec_foreach(&ast->stmts, AST *stmt) {
            stmt->delete(stmt);
        }
    ok_vec_deinit(&ast->stmts);
    free(ast);
}

static int
fprint(FILE *file, Value *v) {
    int val = fprintf(file, "<func(");
    char *sep = "";
    ok_vec_foreach(v->FUNC.args, TA arg) {
            ok_vec_foreach(&arg.names, char *name) {
                    fprintf(file, "%s%s:", sep, name);
                    sep = ",";
                    arg.type.fprint(file, &arg.type);
                }
        }
    fprintf(file, ")");
    if (ok_vec_count(v->FUNC.globals)) {
        char *sep = "[";
        ok_vec_foreach(v->FUNC.globals, char *global) {
                val += fprintf(file, "%s%s", sep, global);
                sep = ",";
            }
        val += fprintf(file, "]");
    }
    val += fprintf(file, ">");
    return val;
}

Value
new_Value_func(ta_v *args, str_v *globals, ast_v *stmts) {
    type_v types;
    ok_vec_init(&types);
    ok_vec_foreach(args, TA arg) {
            size_t count = ok_vec_count(&arg.names);
            for (size_t i = 0; i < count; i++) {
                ok_vec_push(&types, arg.type);
            }
        }

    return (Value){
            new_func_Type(types), .FUNC = {
                    FUNC_USER, args, globals, .USER = {
                            stmts
                    }
            }, fprint
    };
}

static void
put_arg(TA *arg, Values *new_map) {
    ok_vec_foreach(&arg->names, char *name) {
            ok_map_put(new_map, name, (Value *)&arg->type);
        }
}

static int
type_check(void *this, ExecState *state, Type **ret_type) {
    int status = 0;
    ASTFunc *ast = this;

    Values new_map;
    ok_map_init(&new_map);
    ok_map_put_all(&new_map, state->symbols);

    ok_vec_foreach(&ast->globals, char *global) {
            if (!ok_map_contains(state->symbols, global)) {
                fprintf(stderr, "error: unrecognized name \"%s\"\n", global);
                status = 1;
                continue;
            }
            Value *val = ok_map_get(state->symbols, global);
            ok_map_put(&new_map, global, val);
        }
    type_v argTypes;
    ok_vec_init(&argTypes);
    ok_vec_foreach_ptr(&ast->args, TA *arg) {
            if (VerifyType(&arg->type, state->symbols)) {
                status = 1;
                continue;
            }
            arg->type.init = 1;
            put_arg(arg, &new_map);
            size_t count = ok_vec_count(&arg->names);
            for (size_t i = 0; i < count; i++) {
                ok_vec_push(&argTypes, arg->type);
            }
        }
    if (status) {
        ok_map_deinit(&new_map);
        return 1;
    }
    ExecState newState = {
            &new_map, &ast->value
    };
    ok_vec_foreach(&ast->stmts, AST *stmt) {
            if (stmt->type_check(stmt, &newState, NULL)) {
                status = 1;
                break;
            }
        }
    ok_map_deinit(&new_map);
    if (status) {
        return 1;
    }

    if (ret_type) {
        *ret_type = ast->type;
    }
    return 0;
}

static int
exec(void *this, ExecState *state, Value **ret_val) {
    int status = 0;
    ASTFunc *ast = this;

    if (ast->super.super.type_check(ast, state, NULL)) {
        return 1;
    }

    type_v g_types;
    ok_vec_init(&g_types);
    ok_vec_foreach(&ast->globals, char *global) {
            if (!ok_map_contains(state->symbols, global)) {
                fprintf(stderr, "error: unrecognized name \"%s\"\n", global);
                status = 1;
                continue;
            }
            Value *val = ok_map_get(state->symbols, global);
            ok_vec_push(&g_types, val->type);
        }
    if (status) {
        return 1;
    }
    ast->value = new_Value_func(&ast->args, &ast->globals, &ast->stmts);

    ast->value.type.init = 1;

    ast->super.value = &ast->value;
    if (ret_val) {
        *ret_val = &ast->value;
    }
    return 0;
}

ASTExpression *
new_ASTFunc(ta_v args, str_v globals, ast_v stmts) {
    ASTFunc *ast = malloc(sizeof(*ast));
    if (ast == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    ast->args = args;
    ast->globals = globals;
    ast->stmts = stmts;
    *ast = (ASTFunc){
            {
                    {
                            delete, exec, type_check
                    }, &ast->value
            }, args, globals, stmts,
            new_Value_func(&ast->args, &ast->globals, &ast->stmts),
            &ast->value.type
    };
    ast->value.type.init = 1;
    return (ASTExpression *)ast;
}
