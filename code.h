#ifndef _CODE_H
#define _CODE_H 0

#include "doc.h"

typedef struct {
    doc_t *doc;
    int x_start = 0;
    int x_end = 0;
    int y_start = 0;
    int y_end = 0;
} mark_t;

typedef struct {
    int id;
    char *label;
    char *desc;
    struct mark *marks;
} code_t;

typedef struct {
    int id_1;
    int id_2;
    char *label;
    char *desc;
    double rank;
} link_t;

typedef struct codetree {
    code_t *code;
    struct codetree *parent;
    struct codetree *first_children;
    struct codetree *last_children;
    struct codetree *prev;
    struct codetree *next;
} codetree_h;

typedef struct codegraph {
    code_t *codes;
    link_t *links;
} codegraph_t;

#endif /* _CODE_H */
