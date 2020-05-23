%{
#include "parser.h"
YY_DECL;

void yyerror(void *scanner, const char *msg, ...);
double parse_double(const char *str);

%}

%option reentrant always-interactive
%option nodefault nounput noyywrap noinput nounistd
%option bison-bridge

%%

quit { return TOK_QUIT; }

[ \t]+
[\n\r] {
    return TOK_NEWLINE;
}
-?[0-9]+ {
    char *end;
    yylval->ival = strtol(yytext, &end, 10);
    if (*end != '\0') {
        yyerror(NULL,
                "error: integer constant %s is too large for its type",
                yytext);
        yylval->ival = 0;
    }
    return TOK_INT;
}
-?[0-9]+\.[0-9]* {
    yylval->dval = parse_double(yytext);
    return TOK_DOUBLE;
}
-?\.[0-9]+ {
    yylval->dval = parse_double(yytext);
    return TOK_DOUBLE;
}
-?[0-9]+(\.[0-9]+)?(e|E)-?[0-9]+ {
    yylval->dval = parse_double(yytext);
    return TOK_DOUBLE;
}
-?inf {
    yylval->dval = parse_double(yytext);
    return TOK_DOUBLE;
}
-?nan {
    yylval->dval = parse_double(yytext);
    return TOK_DOUBLE;
}
[a-zA-Z][a-zA-Z_]* {
    yylval->sval = strdup(yytext);
    if (yylval->sval == NULL) {
        perror("strdup");
        exit(EXIT_FAILURE);
    }
    return TOK_IDENT;
}
[=\-,] {
    return *yytext;
}
. {
    return *yytext;
}
<<EOF>> {
    return TOK_NEWLINE;
}

%%

double parse_double(const char *str) {
    char *end;
    double d = strtod(str, &end);
    if (*end != '\0') {
        yyerror(NULL,
                "error: double constant %s is invalid",
                str);
        return 0;
    }
    return d;
}
