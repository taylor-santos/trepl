#ifndef TYPE_H
#define TYPE_H

typedef struct type_v ok_vec_of(struct Type) type_v;
typedef struct ok_vec_of(char *) str_v;

typedef struct Type {
    unsigned char init: 1;
    enum {
        TYPE_CLASS, TYPE_OBJECT, TYPE_FUNC, TYPE_NONE
    } kind;
    union {
        struct {
            char *name;
        } OBJECT;
        struct {
            type_v args;
            struct Type *ret_type;
            unsigned char is_builtin: 1;
        } FUNC;
        struct {
            char *name;
        } CLASS;
    };
    int (*fprint)(FILE *file, struct Type *t);
    int (*typecmp)(struct Type *t1, struct Type *t2);
} Type;

typedef struct TypeAnnotation {
    str_v names;
    Type type;
} TA;

Type
new_object_Type(char *class_name);

Type
new_func_Type(type_v types, Type *ret_type);

Type
new_class_Type(char *name);

Type
new_none_Type(void);

struct Values;
int
VerifyType(Type *type, struct Values *symbols);

void
delete_type(Type *type);

#endif
