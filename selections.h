#ifndef _SELECTIONS_H
#define _SELECTIONS_H

#include <glib.h>

struct selection {
    int start;
    int end;
};

void       selection_add(GPtrArray **sel, int note_id, int tag_id, int start, int end);
GPtrArray *selection_get(GPtrArray *sel, int note_id, int tag_id);
void       selection_free(GPtrArray *sel);

#endif /* _SELECTIONS_H */

