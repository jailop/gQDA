#ifndef _BACKEND_H
#define _BACKEND_H

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
    tag_active = -1;

char *file = NULL;

#endif // _BACKEND_H
