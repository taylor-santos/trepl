#ifndef LIST_H
#define LIST_H

#include <stddef.h>

extern size_t LIST_INDEX;

/* Add 'item' to the end of the list, resizing if necessary. Note: the
 * list must be defined as a pointer to 'item's type, to allow for proper
 * type checking.
 */
#define list_append(list, item) ( \
        LIST_INDEX = list_grow((void**)&(list), sizeof((item))), \
        (list)[LIST_INDEX] = (item) \
    )
#define list_length(list) (list_size(list) / sizeof(*list))
#define list_concat(l1, l2) v_concat((void**)&l1, l2)
void
l_concat(void **list1_ptr, const void *list2);
void *
new_list(size_t capacity);
void *
init_list(size_t count, size_t size);
void *
copy_list(const void *);
void
delete_list(void *);
/* Add 'size' to the list's length, reallocating if necessary. Returns the
 * list's old length divided by 'size'. This can be used to index into the
 * list at the newly added space. (see: list_append())
 */
size_t
list_grow(void **list_ptr, size_t size);
size_t
list_size(const void *);

#endif//LIST_H
