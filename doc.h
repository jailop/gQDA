#ifndef _DOC_H
#define _DOC_H 0

#include <stdlib.h>

typedef struct {
    char *title;
    char *data;
    size_t len;  /* length */
    size_t cap;  /* capacity */
} doc_t;

typedef struct doclist {
    doc_t *doc;
    struct doclist *next;
    struct doclist *prev;
};

doc_t *doc_new();
doc_t *doc_new_from_str(const char *s);
void doc_set_data(doc_t *d, const char *s);
#define doc_get_data(d) (d->data)
#define doc_length(d) (d->length)
void  doc_free(doc_t *d);

#endif /* _DOC_H */
