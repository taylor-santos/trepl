#ifndef STACK_H
#define STACK_H

extern size_t INTERNAL_STACK_INDEX;

//Warning: does not check if stack is empty
#define stack_top(stack) stack[INTERNAL_stack_top(stack, sizeof(*stack))]
size_t
INTERNAL_stack_top(const void *, size_t);

#define stack_pop(stack) INTERNAL_stack_pop(stack, sizeof(*stack))
int
INTERNAL_stack_pop(void *, size_t);

#define stack_push(stack, element) (                \
        INTERNAL_STACK_INDEX = INTERNAL_stack_grow( \
            (void**)&(stack),                       \
            sizeof(element)                         \
        ),                                          \
        (stack)[INTERNAL_STACK_INDEX] = (element)   \
    )
size_t
INTERNAL_stack_grow(void **, size_t);

void *
new_stack(size_t capacity);

void
delete_stack(void *stack);

#endif
