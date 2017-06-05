#ifndef _BASE_H
#define _BASE_H 1

#include <gtk/gtk.h>
#include "selection.h"

struct gqda_app {
    GtkWidget *window,
              *note_tree,
              *note_view,
              *tag_tree,
              *main_tree,
              *memo_view,
              *fragment_view;
    GtkTreeModel *note_model;
    GtkTreeModel *tree_model;
    GtkSourceBuffer *memo_buffer;
    GPtrArray *selections;
    unsigned int note_counter;
    unsigned int tag_counter;
    int note_active,
        tag_active,
        main_active;
    char *file;
};

enum {
    NOTE_NAME = 0,
    NOTE_CONTENT,
    NOTE_ID,
    NOTE_COLS
};

enum {
    TAG_NAME = 0,
    TAG_MEMO,
    TAG_ID,
    TAG_COLS
};

#endif /* _BASE_H */
