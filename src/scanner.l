%top{
// "%option nounistd" does not affect the generated header file, so we need
// to manually define it at the top of the file.
#define YY_NO_UNISTD_H 1

#define OK_LIB_DECLARE
#include "ok_lib.h"

void
reset_scanner_indent(void *yyscanner);
}

%{
#include "parser.h"

YY_DECL;

void yyerror(void *scanner, void *result, const char *msg, ...);
double parse_double(const char *str);

%}

%option reentrant always-interactive
%option nounistd
%option nodefault nounput noyywrap noinput noyy_top_state
%option bison-bridge
%option stack
%option extra-type="int"

%s indent

%%

<indent>{
    [\n\r] {
        printf("... ");
        fflush(stdout);
        return TOK_NEWLINE;
    }
    "}" {
        int depth = yyget_extra(yyscanner);
        depth--;
        if (depth <= 0) {
            yy_pop_state(yyscanner);
            if (depth != 0) {
                fprintf(stderr, "DEPTH SOMEHOW LESS THAN ZERO\n");
            }
            depth = 0;
        }
        yyset_extra(depth, yyscanner);
        return *yytext;
    }
}

[ \t]+

func { return TOK_FUNC; }
"->" { return TOK_ARROW; }

";" {
    return *yytext;
}
[\n\r] {
    return TOK_NEWLINE;
}
-?[0-9]+ {
    char *end;
    yylval->ival = strtol(yytext, &end, 10);
    if (*end != '\0') {
        yyerror(NULL, NULL,
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
"{" {
    int depth = yyget_extra(yyscanner);
    if (depth == 0) {
        yy_push_state(indent, yyscanner);
    }
    depth++;
    yyset_extra(depth, yyscanner);
    return *yytext;
}
[=\-,\(\)\[\]] {
    return *yytext;
}
. {
    printf("LEX ERROR: '%d' (%c)\n", *yytext, *yytext);
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
        yyerror(NULL, NULL,
                "error: double constant %s is invalid",
                str);
        return 0;
    }
    return d;
}

void
reset_scanner_indent(void *yyscanner) {
    if (yyget_extra(yyscanner)) {
        yy_pop_state(yyscanner);
    }
    yyset_extra(0, yyscanner);
}
