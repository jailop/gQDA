#ifndef _UTIL_H
#define _UTIL_H 1

typedef struct {
    void **data;
    int len;
} parray_t;

parray_t *p_array_new();
void      p_array_set (parray_t *pa, int index, void *node);
void     *p_array_get (parray_t *pa, int index);
void      p_array_free(parray_t *pa);

#endif // _UTIL_H 
