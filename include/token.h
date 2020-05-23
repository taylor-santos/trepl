#ifndef TOKEN_H
#define TOKEN_H

typedef enum token_type {
    TOK_INT, TOK_NEWLINE, TOK_EOF
} token_type;

typedef union token_val {
    int ival;
} token_val;

#endif
