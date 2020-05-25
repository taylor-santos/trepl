#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

static void
delete(void *this) {
    ASTFunc *ast = this;
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
typecmp(Value *v1, Value *v2) {
    (void)v1;
    (void)v2;
    return v2->type != VAL_FUNC; //TODO: implement func type comparison
}

static int
fprint(FILE *file, Value *v) {
    int val = fprintf(file, "<func()");
    if (ok_vec_count(v->value.FUNC.globals)) {
        char *sep = "[";
        ok_vec_foreach(v->value.FUNC.globals, char *global) {
                val += fprintf(file, "%s%s", sep, global);
                sep = ",";
            }
        val += fprintf(file, "]");
    }
    val += fprintf(file, ">");
    return val;
}

Value
new_Value_func(str_v *globals, ast_v *stmts) {
    return (Value){
            VAL_FUNC, .value.FUNC = {
                    FUNC_DEFINED,
                    globals, .value.DEFINED = stmts
            },
            "func",
            typecmp,
            fprint
    };
}

static int
exec(void *this, symbols_t *symbols, Value **ret_val) {
    (void)symbols;
    ASTFunc *ast = this;

    ast->super.value = &ast->value;
    if (ret_val) {
        *ret_val = &ast->value;
    }
    return 0;
}

ASTExpression *
new_ASTFunc(str_v globals, ast_v stmts) {
    ASTFunc *ast = malloc(sizeof(*ast));
    if (ast == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *ast = (ASTFunc){
            {
                    {
                            delete,
                            exec
                    },
                    &ast->value
            },
            globals,
            stmts,
            new_Value_func(&ast->globals, &ast->stmts)
    };
    return (ASTExpression *)ast;
}
