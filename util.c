#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

#define P_ARRAY_SIZE 1024

char *astrcpy(const char *s)
{
    int len;
    char *ret;
    if (s == NULL) {
        fprintf(stderr, "astrcpy: A NULL string was given to copy\n");
        return NULL;
    }
    len = strlen(s) + 1; /* memory lenght to be allocated */
    ret = malloc(sizeof(char) * len);
    if (ret == NULL) {
        fprintf(stderr, "astrcpy: Memory allocation failed\n");
        return NULL;
    }
    strcpy(ret, s); /* finally, string is copied */
    return ret;
}

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

void p_array_resize(parray_t *pa, int min)
{
    int i;
    int new_length; 
    void **aux;
    if (pa->len > min)
        return;
    new_length = pa->len * 2;
    while (min >= new_length)
        new_length *= 2; 
    aux = realloc(pa->data, sizeof(void *) * new_length);
    for (i = pa->len; i < new_length; i++)
        aux[i] = NULL;
    pa->data = aux;
    pa->len = new_length;
}

void p_array_set (parray_t *pa, int index, void *node)
{
    if (!pa) {
        fprintf(stderr, "parray_t: fail, pointer is NULL\n");
        return;
    }
    if (index < 0) {
        fprintf(stderr, "parray_t: index is negative\n");
        return;
    }
    if (index >= pa->len) 
        p_array_resize(pa, index); 
    pa->data[index] = node;
}

void *p_array_get (parray_t *pa, int index)
{
    if (index < 0) {
        fprintf(stderr, "parray_t: index is negative\n");
        return NULL;
    }
    if (index >= pa->len) 
        p_array_resize(pa, index);
    return pa->data[index];
}

void p_array_free(parray_t *pa)
{
    free(pa->data);
    free(pa);
}
