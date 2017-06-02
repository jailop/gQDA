#ifndef _BASE_H
#define _BASE_H

#include <gtk/gtk.h>
#include "selection.h"

GtkWidget *window,
          *note_tree,
          *note_view,
          *tag_tree,
          *main_tree,
          *fragment_view;

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

struct note_t {
    char *label;
    char *content;
    char *id;
    char *pos;
};

GPtrArray *selections = NULL;

unsigned int note_counter = 0;
unsigned int tag_counter = 0;
int note_active = -1,
    tag_active = -1,
    main_active = -1;

char *file = NULL;

#endif /* _BASE_H */
