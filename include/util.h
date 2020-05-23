#ifndef UTIL_H
#define UTIL_H

#define INTERNAL_ERROR(...) (          \
    fprintf(stderr,                    \
    "%s:%d: internal error in %s(): ", \
    __FILE__,                          \
    __LINE__,                          \
    __func__),                         \
    fprintf(stderr, __VA_ARGS__))

#endif
