#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "scanner.h"

#define NAME "tlang"
#define VERSION "0.0.5"

static int TLANG_QUIT = 0;

static struct {
    const char *cmd;
} cmds[] = {
        { "help" }, { "license" }
};

typedef struct ok_vec_of(AST*) ast_vec;

static void
print_tlang_info(void) {
    size_t i;
    char *sep = "";

    printf(NAME " " VERSION " (" __DATE__ ", " __TIME__ ")\n");
    printf("Type ");
    for (i = 0; i < sizeof(cmds) / sizeof(*cmds) - 1; i++) {
        printf("%s\"%s\"", sep, cmds[i].cmd);
        sep = ", ";
    }
    printf(" or \"%s\" for more information.\n", cmds[i].cmd);
}

static int
println_fprint(FILE *file, Value *v) {
    (void)v;
    return fprintf(file, "<built-in func println>");
}

static int
quit_fprint(FILE *file, Value *v) {
    (void)v;
    return fprintf(file, "<built-in func quit>");
}

static int
int_fprint(FILE *file, Value *v) {
    (void)v;
    return fprintf(file, "<class 'int'>");
}

static int
double_fprint(FILE *file, Value *v) {
    (void)v;
    return fprintf(file, "<class 'double'>");
}

static Value
println(value_v values) {
    ok_vec_foreach(&values, Value *val) {
            val->fprint(stdout, val);
        }
    printf("\n");
    return new_Value_none();
}

static Value
quit(value_v values) {
    (void)values;
    TLANG_QUIT = 1;
    return new_Value_none();
}

struct builtin {
    char *name;
    Value value;
};
static struct builtin builtin_funcs[] = {
        {
                "println", {
                { 0 }, .FUNC = {
                        FUNC_BUILTIN, .BUILTIN = { println }
                }, println_fprint
        }
        }, {
                "quit", {
                        { 0 }, .FUNC = {
                                FUNC_BUILTIN, .BUILTIN = { quit }
                        }, quit_fprint
                }
        }
};
static struct builtin builtin_types[] = {
        {
                "int", {
                { 0 }, {
                        {
                                0
                        }
                }, int_fprint
        }
        }, {
                "double", {
                        {
                                0
                        }, {
                                {
                                        0
                                }
                        }, double_fprint
                }
        }
};

static void
add_builtins(Values *symbols) {
    for (size_t i = 0; i < sizeof(builtin_funcs) / sizeof(*builtin_funcs); i++) {
        type_v args;
        ok_vec_init(&args);
        builtin_funcs[i].value.type = new_func_Type(args);
        builtin_funcs[i].value.type.FUNC.is_builtin = 1;
        builtin_funcs[i].value.type.init = 1;
        ok_map_put(symbols, builtin_funcs[i].name, &builtin_funcs[i].value);
    }
    for (size_t i = 0; i < sizeof(builtin_types) / sizeof(*builtin_types); i++) {
        builtin_types[i].value.type = new_class_Type(builtin_types[i].name);
        builtin_types[i].value.type.init = 1;
        ok_map_put(symbols, builtin_types[i].name, &builtin_types[i].value);
    }
}

void
startREPL(void) {
    yyscan_t scanner;
    YY_BUFFER_STATE state;
    Values symbols;
    ast_vec lines;

    ok_map_init(&symbols);
    ok_vec_init(&lines);
    add_builtins(&symbols);
    print_tlang_info();
    fflush(stdout);
    if (yylex_init_extra(0, &scanner)) {
        perror("yylex_init_extra");
        exit(EXIT_FAILURE);
    }
    state = yy_create_buffer(stdin, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);
    while (!TLANG_QUIT) {
        printf(">>> ");
        fflush(stdout);
        ast_v result;
        if (yyparse(scanner, &result)) {
            continue;
        }
        ExecState execState = {
                &symbols, NULL
        };
        ok_vec_foreach(&result, AST *stmt) {
                if (stmt == NULL) {
                    continue;
                }
                Value *ret = NULL;
                stmt->exec(stmt, &execState, &ret);
                if (ret && ret->type.init == 1 && ret->type.kind != TYPE_NONE) {
                    ret->fprint(stdout, ret);
                    fprintf(stdout, "\n");
                }
            }
        ok_vec_push_all(&lines, &result);
        ok_vec_deinit(&result);
    }
    yylex_destroy(scanner);
    ok_vec_foreach(&lines, AST *root) {
            root->delete(root);
        }
    ok_vec_deinit(&lines);
    ok_map_deinit(&symbols);
}
