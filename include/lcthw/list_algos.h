#ifndef lcthw_list_algos_h
#define lcthw_list_algos_h

#include "list.h"

typedef int (*List_compare)(void *a, void *b);

int List_bubble_sort(List *list, List_compare comparator);
List *List_merge_sort(List *list, List_compare comparator);

#endif
