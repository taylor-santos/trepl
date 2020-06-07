#ifndef VALUE_H
#define VALUE_H

#include <stdio.h>

#define OK_LIB_DECLARE
#include "ok_lib.h"
#include "type.h"

typedef struct value_v ok_vec_of(struct Value*) value_v;

struct Object {
    enum {
        OBJ_BUILTIN, OBJ_DEFINED
    } kind;
    union {
        struct {
            enum {
                BUILTIN_INT, BUILTIN_DOUBLE
            } kind;
            union {
                int INT;
                double DOUBLE;
            };
        } BUILTIN;
    };
};

struct ta_v;

struct Func {
    enum {
        FUNC_BUILTIN, FUNC_USER
    } kind;
    struct ta_v *args;
    str_v *globals;

    union {
        struct {
            struct Value (*fn)(value_v args);
        } BUILTIN;

        struct {
            struct ast_v *stmts;
        } USER;
    };
};
typedef struct Value {
    Type type;
    union {
        struct Object OBJECT;
        struct Func FUNC;
    };
    int (*fprint)(FILE *file, struct Value *v);
} Value;

Value
new_Value_none(void);

#endif
