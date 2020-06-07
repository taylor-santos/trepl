#include "value.h"

static int
none_fprint(FILE *file, struct Value *v) {
    (void)v;
    return fprintf(file, "none");
}

Value
new_Value_none(void) {
    return (Value){
            new_none_Type(), { { 0 } }, none_fprint
    };
}
