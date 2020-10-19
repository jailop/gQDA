#include "doc.h"
#include <string.h>

doc_t *doc_new()
{
    doc_t *d = malloc(sizeof(doc_t));
    d->title = NULL;
    d->data = NULL;
    d->len = 0;
    d->cap = 0;
    return d;
}

void doc_set_data(doc_t *d, const char *s)
{
    if (d->data != NULL)
        free(d->data);
    d->len = strlen(s);
    d->data = malloc(sizeof(char) * d->len + 1);
    d->cap = d->len;
    strcpy(d->data, s);
}

doc_t *doc_new_from_str(const char *s)
{
    doc_t *d = doc_new();
    doc_set_data(d, s);
    return d;
}

void  doc_free(doc_t *d)
{
    free(d->data);
    free(d);
}
