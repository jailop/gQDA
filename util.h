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

char *astrcpy(const char *s);
/* Allocates space in memory and copies an string in it.
 * User must free allocated memory when the copied string
 * will be not used anymore.
 *
 * @param s : the string to be copied
 * @return  : the string copied in a new memory space
 */

#endif // _UTIL_H 
