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
exec(void *this, symbols_t *symbols, Value **ret_val) {
    int status = 0;
    ASTCall *ast = this;
    ASTExpression *func = ast->func;
    if (func->super.exec(ast->func, symbols, NULL)) {
        return 1;
    }
    Value *fn_val = func->value;
    if (fn_val->type != VAL_FUNC) {
        fprintf(stderr,
                "error: \"%s\" object is not callable\n",
                fn_val->name);
        return 1;
    }
    value_v args;
    ok_vec_init(&args);
    ok_vec_foreach(&ast->args, ASTExpression *expr) {
            if (expr->super.exec(expr, symbols, NULL)) {
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
    struct Func fn = fn_val->value.FUNC;
    if (fn.type == FUNC_BUILTIN) {
        ast->ret = fn.value.BUILTIN(args);
    } else {
        symbols_t new_symbols;
        ok_map_init(&new_symbols);
        ok_map_put_all(&new_symbols, symbols);

        ast_v *stmts = fn.value.DEFINED;
        ok_vec_foreach(stmts, AST *stmt) {
                stmt->exec(stmt, &new_symbols, NULL);
            }
        ok_vec_foreach(fn.globals, char *global) {
                if (ok_map_contains(&new_symbols, global)) {
                    Value *val = ok_map_get(&new_symbols, global);
                    if (ok_map_contains(symbols, global)) {
                        **ok_map_get_ptr(symbols, global) = *val;
                    } else {
                        ok_map_put(symbols, global, val);
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
    return 0;
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
                            delete,
                            exec
                    },
                    NULL
            },
            func,
            args,
            { 0 }
    };
    return (ASTExpression *)ast;
}
