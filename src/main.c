#include <stdio.h>

#include "tlang.h"
#include "util.h"

int
main(int argc, char *argv[]) {
    if (argc == 1) {
        startREPL();
    } else {
        (void)argv;
        INTERNAL_ERROR("file support not implemented\n");
        return 1;
    }
    return 0;
}
