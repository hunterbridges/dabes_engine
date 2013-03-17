#ifndef lcthw_List_h
#define lcthw_List_h

#include <stdlib.h>

struct ListNode;

typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *prev;
    void *value;
} ListNode;

typedef struct List {
    int count;
    ListNode *first;
    ListNode *last;
} List;

List *List_create();
void List_destroy(List *list);
void List_clear(List *list);
void List_clear_destroy(List *list);

#define List_count(A) ((A)->count)
#define List_first(A) ((A)->first != NULL ? (A)->first->value : NULL)
#define List_last(A) ((A)->last != NULL ? (A)->last->value : NULL)

void List_push(List *list, void *value);
void *List_pop(List *list);

void List_unshift(List *list, void *value);
void *List_shift(List *list);

void *List_remove(List *list, ListNode *node);

List *List_copy(List *list);
void List_join(List *left, List *right);
void List_split(List *list, ListNode *splitter, List **remainder, int recount);
void ListNode_swap(ListNode *a, ListNode *b);

// L - List to enumerate
// S - Which end of the list to start at (first, last)
// M - Which direction to go (next, prev)
// V - Name given to current ListNode
#define LIST_FOREACH(L, S, M, V) ListNode *_node = NULL; \
    ListNode *V = NULL; \
    for(V = _node = L->S; _node != NULL; V = _node = _node->M)

#endif
