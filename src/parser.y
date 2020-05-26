%{
    #include <stdarg.h>

    #include "parser.h"
    #include "scanner.h"

    #define YYERROR_VERBOSE
    void yyerror(void *scanner, void *result, const char *msg, ...);

    //int yydebug = 1;
%}

%code provides{
    #define YY_DECL int yylex (YYSTYPE *yylval_param, \
        void *yyscanner)
    YY_DECL;
}

%code requires{
    #include "ast.h"
};

%define api.pure full
%define parse.error verbose
%define parse.trace
%param { void *scanner }
%parse-param { ast_v *result }

%union {
    int ival;
    double dval;
    char *sval;
    AST *ast;
    ast_v ast_v;
    ASTExpression *expr;
    expr_v expr_v;
    str_v str_v;
}

%token<ival>  TOK_INT     "int"
              TOK_INDENT  "indent"
              TOK_OUTDENT "outdent"
%token<dval>  TOK_DOUBLE  "double"
%token<sval>  TOK_IDENT   "identifier"
%token        TOK_NEWLINE "end of line"
              TOK_FUNC    "func"
              TOK_ARROW   "->"

%type<ast_v>  statements_line
              statements
%type<ast>    statement
              assignment
%type<expr>   expression
              unary_expression
              postfix_expression
              primary_expression
              func_def
%type<expr_v> opt_arg_expr_list
              arg_expr_list
%type<str_v>  opt_idents_list idents_list

%start line

%%

line
    : statements_line TOK_NEWLINE {
        *result = $1;
        YYACCEPT;
    }

statements_line
    : statement {
        ok_vec_init(&$$);
        if ($1) {
            ok_vec_push(&$$, $1);
        }
    }
    | error {
        reset_scanner_indent(scanner);
        ok_vec_init(&$$);
    }
    | statements_line ';' statement {
        $$ = $1;
        if ($3) {
            ok_vec_push(&$$, $3);
        }
    }
    | statements_line ';' error {
        reset_scanner_indent(scanner);
        $$ = $1;
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
        str_v idents;
        ok_vec_init(&idents);
        ok_vec_push(&idents, $1);
        $$ = new_ASTAssignment(idents, $3);
    }
    | TOK_IDENT '=' assignment {
        ASTAssignment *val = (void*)$3;
        ok_vec_push(&val->idents, $1);
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
    | func_def

func_def
    : TOK_FUNC '(' opt_args ')' opt_idents_list '{' statements '}' {
        $$ = new_ASTFunc($5, $7);
    }

opt_idents_list
    : %empty {
        ok_vec_init(&$$);
    }
    | '[' idents_list ']' {
        $$ = $2;
    }

idents_list
    : TOK_IDENT {
        ok_vec_init(&$$);
        ok_vec_push(&$$, $1);
    }
    | idents_list ',' TOK_IDENT {
        $$ = $1;
        ok_vec_push(&$$, $3);
    }

opt_args
    : %empty
    | args

args
    : arg
    | args ',' arg

arg
    : TOK_IDENT ':' type

type
    : TOK_IDENT

statements
    : statement {
        ok_vec_init(&$$);
        if ($1) {
            ok_vec_push(&$$, $1);
        }
    }
    | statements ';' statement {
        $$ = $1;
        if ($3) {
            ok_vec_push(&$$, $3);
        }
    }
    | statements TOK_NEWLINE statement {
        $$ = $1;
        if ($3) {
            ok_vec_push(&$$, $3);
        }
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

void yyerror(void *scanner, void *result, const char *msg, ...) {
    va_list args;
    (void)scanner;
    (void)result;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, "\n");
}
