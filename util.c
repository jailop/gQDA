#include <stdlib.h>
#include "util.h"

#define P_ARRAY_SIZE 32

parray_t *p_array_new()
{
    int i;
    parray_t *ret = malloc(sizeof(parray_t));
    ret->data = malloc(sizeof(void *) * P_ARRAY_SIZE);
    ret->len  = P_ARRAY_SIZE;
    for (i = 0; i < ret->len; i++)
        ret->data[i] = NULL;
    return ret;
}

void p_array_set (parray_t *pa, int index, void *node)
{
    int i;
    int new_length; 
    void **aux;
    if (!pa)
        return;
    while (index >= pa->len) {
        new_length = pa->len * 2; 
        aux = realloc(pa->data, sizeof(void *) * new_length);
        pa->data = aux;
        for (i = pa->len; i < new_length; i++)
            pa->data[i] = NULL;
        pa->len = new_length;
    }
    pa->data[index] = node;
}

void *p_array_get (parray_t *pa, int index)
{
    if (index >= pa->len)
        return NULL;
    return pa->data[index];
}

void p_array_free(parray_t *pa)
{
    free(pa->data);
    free(pa);
}
