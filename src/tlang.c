#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OK_LIB_DEFINE
#include "parser.h"
#include "scanner.h"

#define NAME "tlang"
#define VERSION "0.0.1"

int TERMINATE_REPL = 0;

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
println_typecmp(const Value *v1, const Value *v2) {
    (void)v1;
    (void)v2;
    return 0;
}

static int
println_fprint(FILE *file, const Value *v) {
    (void)v;
    return fprintf(file, "<built-in func println>");
}

static Value
println_fn(value_v values) {
    int ret = 0;
    ok_vec_foreach(&values, const Value *val) {
            ret += val->fprint(stdout, val);
        }
    ret += printf("\n");
    return new_Value_int(ret);
}

static Func println = {
        FUNC_BUILTIN, .value.BUILTIN = println_fn
};

static struct {
    char *name;
    Value value;
} builtins[] = {
        {
                "println", {
                                   VAL_FUNC, .value.FUNC = &println, "func(...)->int", println_typecmp, println_fprint
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
    if (yylex_init(&scanner)) {
        exit(EXIT_FAILURE);
    }
    state = yy_create_buffer(stdin, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);
    while (!TERMINATE_REPL) {
        printf(">>> ");
        fflush(stdout);
        AST *root;
        yyparse(scanner, &root);
        if (root) {
            ok_vec_push(&lines, root);
            const Value *ret = NULL;
            root->exec(root, &symbols, &ret);
            if (ret) {
                ret->fprint(stdout, ret);
                fprintf(stdout, "\n");
            }
        }
    }
    yylex_destroy(scanner);
    ok_vec_foreach(&lines, AST *root) {
            root->delete(root);
        }
    ok_vec_deinit(&lines);
    ok_map_deinit(&symbols);
}
