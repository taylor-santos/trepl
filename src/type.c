#include "ast.h"

static int
object_fprint(FILE *file, struct Type *t) {
    return fprintf(file, "<class %s>", t->OBJECT.name);
}

static int
object_typecmp(Type *t1, Type *t2) {
    if (t2->kind == TYPE_OBJECT) {
        return strcmp(t1->OBJECT.name, t2->OBJECT.name);
    }
    if (t2->kind == TYPE_CLASS) {
    }
    return 1;
}

Type
new_object_Type(char *class_name) {
    return (Type){
            0, TYPE_OBJECT, .OBJECT= { class_name }, object_fprint, object_typecmp
    };
}

static int
func_fprint(FILE *file, struct Type *t) {
    int val = fprintf(file, "<func(");
    char *sep = "";
    ok_vec_foreach(&t->FUNC.args, Type type) {
            val += fprintf(file, "%s", sep);
            val += type.fprint(file, &type);
            sep = ",";
        }
    val += fprintf(file, ")>");
    return val;
}

static int
func_typecmp(Type *t1, Type *t2) {
    (void)t1;
    if (t2->kind != TYPE_FUNC) {
        return 1;
    }
    size_t n1 = ok_vec_count(&t1->FUNC.args);
    size_t n2 = ok_vec_count(&t2->FUNC.args);
    if (n1 != n2) {
        return 1;
    }
    for (size_t i = 0; i < n1; i++) {
        Type arg1 = ok_vec_get(&t1->FUNC.args, i), arg2 = ok_vec_get(&t2->FUNC.args, i);
        if (arg1.typecmp(&arg1, &arg2)) {
            return 1;
        }
    }
    return 0;
}

Type
new_func_Type(type_v types) {
    return (Type){
            0, TYPE_FUNC, .FUNC = { types, 0 }, func_fprint, func_typecmp
    };
}

static int
class_fprint(FILE *file, struct Type *t) {
    int val = fprintf(file, "<class '%s'>", t->CLASS.name);
    return val;
}

static int
class_typecmp(Type *t1, Type *t2) {
    (void)t1;
    (void)t2;
    fprintf(stderr, "TODO: implement class comparison\n");
    return 1;
}

Type
new_class_Type(char *name) {
    return (Type){
            TYPE_CLASS, .CLASS = { name }, class_fprint, class_typecmp
    };
}

static int
none_fprint(FILE *file, Type *t) {
    (void)t;
    return fprintf(file, "<none>");
}

static int
none_typecmp(Type *t1, Type *t2) {
    (void)t1;
    return t2->kind != TYPE_NONE;
}

Type

new_none_Type(void) {
    return (Type){
            0, TYPE_NONE, { { 0 } }, none_fprint, none_typecmp
    };
}

int
VerifyType(Type *type, Values *symbols) {
    Value *symbol;
    int status = 0;
    switch (type->kind) {
        case TYPE_NONE:
            return 0;
        case TYPE_FUNC:
            ok_vec_foreach_ptr(&type->FUNC.args, Type *arg) {
                    if (VerifyType(arg, symbols)) {
                        status = 1;
                    }
                }
            return status;
        case TYPE_OBJECT:
            symbol = ok_map_get(symbols, type->OBJECT.name);
            if (symbol == NULL) {
                fprintf(stderr, "error: name \"%s\" is not defined\n", type->OBJECT.name);
                return 1;
            }
            if (symbol->type.kind != TYPE_CLASS) {
                fprintf(stderr, "error: type \"%s\" is not the name of a valid "
                                "type", type->OBJECT.name);
                return 1;
            }
            return 0;
        case TYPE_CLASS:
            fprintf(stderr, "TODO: Class Verify\n");
            return 1;
    }
    return 1;
}

void
delete_type(Type *type) {
    switch (type->kind) {
        case TYPE_OBJECT:
            free(type->OBJECT.name);
            break;
        case TYPE_CLASS:
        case TYPE_FUNC:
        case TYPE_NONE:
            break;
    }
}
