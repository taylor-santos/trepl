%{
    #include <stdarg.h>

    #include "parser.h"
    #include "scanner.h"

    #define YYERROR_VERBOSE
    void yyerror(void *scanner, AST **root, const char *msg, ...);
%}

%code provides{
    #define YY_DECL int yylex (YYSTYPE *yylval_param, \
        void *yyscanner)
    YY_DECL;
    extern int TERMINATE_REPL;
}

%code requires{
    #include "ast.h"
    #include "list.h"
};

%define api.pure full
%define parse.error verbose
%define parse.trace
%param { void *scanner }
%parse-param { AST **root }

%union {
    int ival;
    double dval;
    char *sval;
    AST *ast;
    ASTExpression *expr;
    expr_vec expr_v;
}

%token<ival>  TOK_INT     "int"
%token<dval>  TOK_DOUBLE  "double"
%token<sval>  TOK_IDENT   "identifier"
%token        TOK_NEWLINE "end of line"
              TOK_QUIT    "quit"

%type<ast>    statement
              assignment
%type<expr>   expression
              unary_expression
              postfix_expression
              primary_expression
%type<expr_v> opt_arg_expr_list
              arg_expr_list

%start line

%%

line
    : statement TOK_NEWLINE {
        *root = $1;
        YYACCEPT;
    }
    | error TOK_NEWLINE {
        *root = NULL;
        YYACCEPT;
    }
    | TOK_QUIT TOK_NEWLINE {
        *root = NULL;
        TERMINATE_REPL = 1;
        YYACCEPT;
    }

statement
    : assignment
    | expression {
        $$ = (AST*)$1;
    }
    | %empty {
        $$ = NULL;
    }

assignment
    : TOK_IDENT '=' expression {
        char **idents = new_list(1);
        list_append(idents, $1);
        $$ = new_ASTAssignment(idents, $3);
    }
    | TOK_IDENT '=' assignment {
        ASTAssignment *val = (void*)$3;
        list_append(val->idents, $1);
        $$ = $3;
    }

expression
    : unary_expression

unary_expression
    : postfix_expression
    | unary_operator postfix_expression {
        $$ = $2;
    }

postfix_expression
    : primary_expression
    | postfix_expression '(' opt_arg_expr_list ')' {
        $$ = new_ASTCall($1, $3);
    }

primary_expression
    : TOK_INT {
        $$ = new_ASTInt($1);
    }
    | TOK_DOUBLE {
        $$ = new_ASTDouble($1);
    }
    | TOK_IDENT {
        $$ = new_ASTVar($1);
    }

unary_operator
    : '-'

opt_arg_expr_list
    : %empty {
        ok_vec_init(&$$);
    }
    | arg_expr_list

arg_expr_list
    : expression {
        ok_vec_init(&$$);
        ok_vec_push(&$$, $1);
    }
    | arg_expr_list ',' expression {
        $$ = $1;
        ok_vec_push(&$$, $3);
    }

%%

void yyerror(void *scanner, AST **root, const char *msg, ...) {
    va_list args;
    (void)scanner;
    (void)root;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, "\n");
    //int tok;
    // Clear remaining tokens on line
    //while ((tok = yylex(NULL, scanner)) != TOK_NEWLINE);
}
