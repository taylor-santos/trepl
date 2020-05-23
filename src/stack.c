#include <stdlib.h>

size_t INTERNAL_STACK_INDEX;

typedef struct data {
    size_t size;
    size_t capacity;
    char data[];
} data;

static data *
get_data(void *stack) {
    return &((data *)stack)[-1];
}

static const data *
get_const_data(const void *stack) {
    return &((const data *)stack)[-1];
}

size_t
INTERNAL_stack_top(const void *stack, size_t size) {
    const data *s = get_const_data(stack);
    if (s->size == 0) {
        return 0;
    }
    return s->size / size - 1;
}

int
INTERNAL_stack_pop(void *stack, size_t size) {
    data *s = get_data(stack);
    if (s->size < size) {
        return 1;
    }
    s->size -= size;
    return 0;
}

static void
stack_realloc(data **s_ptr, size_t capacity) {
    void *new_s = realloc(*s_ptr, sizeof(data) + capacity);
    if (new_s == NULL) {
        free(*s_ptr);
        perror("realloc");
        exit(EXIT_FAILURE);
    }
    *s_ptr = new_s;
    (*s_ptr)->capacity = capacity;
}

size_t
INTERNAL_stack_grow(void **stack_ptr, size_t size) {
    data *s = get_data(*stack_ptr);
    if (s->size + size > s->capacity) {
        stack_realloc(&s, s->capacity * 2 + s->size + size);
    }
    s->size += size;
    *stack_ptr = s->data;
    return s->size / size - 1;
}

void *
new_stack(size_t capacity) {
    data *s = malloc(sizeof(*s) + capacity);
    if (s == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    s->size = 0;
    s->capacity = capacity;
    return s->data;
}

void
delete_stack(void *stack) {
    data *s = get_data(stack);
    free(s);
}
