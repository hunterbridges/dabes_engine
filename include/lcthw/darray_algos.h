#ifndef darray_algos_h
#define darray_algos_h

#ifdef DABES_IOS
#else
#include <lcthw/darray.h>
#endif

typedef int (*DArray_compare)(const void *a, const void *b);

int DArray_qsort(DArray *array, DArray_compare cmp);

int DArray_heapsort(DArray *array, DArray_compare cmp);

int DArray_mergesort(DArray *array, DArray_compare cmp);

#endif
