#include <stdlib.h>
#include <stdio.h>

#define OK_LIB_DEFINE
#include "ast.h"

static void
delete(void *this) {
    ASTCall *ast = this;
    ast->func->super.delete(ast->func);
    ok_vec_foreach(&ast->args, ASTExpression *expr) {
            expr->super.delete(expr);
        }
    ok_vec_deinit(&ast->args);
    free(ast);
}

static int
type_check(void *this, ExecState *state, Type **ret_type) {
    (void)this;
    (void)state;
    int status = 0;
    ASTCall *ast = this;
    Type *type;
    if (ast->func->super.type_check(ast->func, state, &type)) {
        return 1;
    }
    if (type->kind != TYPE_FUNC) {
        fprintf(stderr, "error: \"");
        type->fprint(stderr, type);
        fprintf(stderr, "\" object is not callable\n");
        return 1;
    }
    size_t argCount = ok_vec_count(&ast->args);
    if (!type->FUNC.is_builtin) {
        if (argCount != ok_vec_count(&type->FUNC.args)) {
            fprintf(stderr, "error: incompatible number of arguments\n");
            return 1;
        }
    }
    for (size_t i = 0; i < argCount; i++) {
        ASTExpression *arg = ok_vec_get(&ast->args, i);
        Type *argType;
        if (arg->super.type_check(arg, state, &argType)) {
            status = 1;
            continue;
        }
        if (!type->FUNC.is_builtin) {
            Type expType = ok_vec_get(&type->FUNC.args, i);
            if (argType->typecmp(argType, &expType)) {
                fprintf(stderr, "error: incompatible argument type\n");
                status = 1;
                continue;
            }
        }
    }
    if (ret_type) {
        *ret_type = &ast->ret.type;
    }
    return status;
}

static int
exec(void *this, ExecState *state, Value **ret_val) {
    int status = 0;
    ASTCall *ast = this;
    ASTExpression *func = ast->func;
    if (func->super.exec(ast->func, state, NULL)) {
        return 1;
    }
    Value *fn_val = func->value;
    if (fn_val->type.kind != TYPE_FUNC) {
        fprintf(stderr, "error: \"");
        fn_val->type.fprint(stderr, &fn_val->type);
        fprintf(stderr, "\" object is not callable\n");
        return 1;
    }
    value_v args;
    ok_vec_init(&args);
    ok_vec_foreach(&ast->args, ASTExpression *expr) {
            if (expr->super.exec(expr, state, NULL)) {
                status = 1;
            }
            if (!status) {
                ok_vec_push(&args, expr->value);
            }
        }
    if (status) {
        ok_vec_deinit(&args);
        return status;
    }
    struct Func fn = fn_val->FUNC;

    if (fn.kind == FUNC_BUILTIN) {
        ast->ret = fn.BUILTIN.fn(args);
    } else {
        Values new_symbols;
        ok_map_init(&new_symbols);
        ok_map_put_all(&new_symbols, state->symbols);
        size_t arg_count = ok_vec_count(&args);
        size_t def_count = 0;
        ok_vec_foreach(fn.args, TA arg) {
                def_count += ok_vec_count(&arg.names);
            }
        if (arg_count != def_count) {
            fprintf(stderr, "error: incompatible number of arguments\n");
            ok_vec_deinit(&args);
            ok_map_deinit(&new_symbols);
            return 1;
        }
        size_t arg_index = 0;
        ok_vec_foreach(fn.args, TA arg) {
                ok_vec_foreach(&arg.names, char *name) {
                        Value *val = ok_vec_get(&args, arg_index);
                        if (val->type.typecmp(&val->type, &arg.type)) {
                            fprintf(stderr,
                                    "error: incompatible argument type\n");
                            status = 1;
                        } else {
                            ok_map_put(&new_symbols, name, val);
                        }
                        arg_index++;
                    }
            }
        if (status) {
            ok_vec_deinit(&args);
            ok_map_deinit(&new_symbols);
            return status;
        }
        ExecState new_state = {
                &new_symbols, NULL
        };
        ok_vec_foreach(fn.USER.stmts, AST *stmt) {
                if (stmt->exec(stmt, &new_state, NULL)) {
                    status = 1;
                    break;
                }
            }
        if (!status) {
            ok_vec_foreach(fn.globals, char *global) {
                    if (ok_map_contains(&new_symbols, global)) {
                        Value *val = ok_map_get(&new_symbols, global);
                        if (ok_map_contains(state->symbols, global)) {
                            **ok_map_get_ptr(state->symbols, global) = *val;
                        } else {
                            ok_map_put(state->symbols, global, val);
                        }
                    }
                }
        }
        ok_map_deinit(&new_symbols);
        ast->ret = new_Value_none();
    }
    ast->super.value = &ast->ret;
    if (ret_val) {
        *ret_val = &ast->ret;
    }
    ok_vec_deinit(&args);
    return status;
}

ASTExpression *
new_ASTCall(ASTExpression *func, expr_v args) {
    ASTCall *ast = malloc(sizeof(*ast));
    if (ast == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *ast = (ASTCall){
            {
                    {
                            delete, exec, type_check
                    }, NULL
            }, func, args, new_Value_none()
    };
    return (ASTExpression *)ast;
}
