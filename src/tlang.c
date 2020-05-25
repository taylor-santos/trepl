#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "scanner.h"

#define NAME "tlang"
#define VERSION "0.0.2"

static int TLANG_QUIT = 0;

static struct {
    const char *cmd;
} cmds[] = {
        { "help" },
        { "license" }
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
builtin_typecmp(Value *v1, Value *v2) {
    (void)v1;
    (void)v2;
    return 0;
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

Value
new_Value_none(void) {
    return (Value){
            VAL_NONE,
            { 0 },
            "none",
            NULL,
            NULL
    };
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

static struct {
    char *name;
    Value value;
} builtins[] = {
        {
                "println", {
                                   VAL_FUNC, .value.FUNC = {
                        FUNC_BUILTIN, .value.BUILTIN = println
                }, "func(...)->none", builtin_typecmp, println_fprint
                           }
        },
        {
                "quit",    {
                                   VAL_FUNC, .value.FUNC = {
                        FUNC_BUILTIN, .value.BUILTIN = quit
                }, "func()->none",    builtin_typecmp, quit_fprint
                           }
        }
};

static void
add_builtins(symbols_t *symbols) {
    for (size_t i = 0; i < sizeof(builtins) / sizeof(*builtins); i++) {
        ok_map_put(symbols, builtins[i].name, &builtins[i].value);
    }
}

void
startREPL(void) {
    yyscan_t scanner;
    YY_BUFFER_STATE state;
    symbols_t symbols;
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
        ok_vec_foreach(&result, AST *stmt) {
                if (stmt == NULL) {
                    continue;
                }
                Value *ret = NULL;
                stmt->exec(stmt, &symbols, &ret);
                if (ret && ret->type != VAL_NONE) {
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
