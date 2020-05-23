#include <stdlib.h>
#include <stdio.h>

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
exec(void *this, symbols_t *symbols, const Value **ret_val) {
    (void)ret_val;
    int status = 0;
    ASTCall *ast = this;
    ASTExpression *func = ast->func;
    if (func->super.exec(ast->func, symbols, NULL)) {
        return 1;
    }
    const Value *fn_val = func->value;
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
    const Func *fn = fn_val->value.FUNC;
    if (fn->type == FUNC_BUILTIN) {
        ast->ret = fn->value.BUILTIN(args);
        ast->super.value = &ast->ret;
        if (ret_val) {
            *ret_val = &ast->ret;
        }
    } else {
        fprintf(stderr, "error: user-defined functions not implemented\n");
        return 1;
    }
    ok_vec_deinit(&args);
    return 0;
}

ASTExpression *
new_ASTCall(ASTExpression *func, expr_vec args) {
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
