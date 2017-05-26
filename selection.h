#ifndef _SELECTION_H
#define _SELECTION_H

#include <glib.h>

struct selection {
    int x1;        /* Offset of the starting position */
    int x2;        /* Offset of the ending position */
    /* Below fields are reserved for future usage */
    int y1;        /* Alternate value of the starting position */
    int y2;        /* Alternate value of the ending position */
    int page;      /* Page of the segment text */
    int section;   /* Section of the segment text */
};

void selection_add(GPtrArray **sel, int note_id, int tag_id, int start, int end);
/* The selection structure ables to store text segments in a note with tags.
 * To start this structure, the first time this function is called, the first
 * parameter must be NULL.
 *
 * @param sel     : Pointer to the selection structure
 * @param note_id : Note identifier
 * @param tag_id  : Tag identifier
 * @param start   : Offset of the starting position
 * @param end     : Offset of the ending position
 */

GPtrArray *selection_get(GPtrArray *sel, int note_id, int tag_id);
/* Returns an array with pointer to the selection marks, i.e. starting
 * and ending offsets of each text segment.
 *
 * @param sel     : Pointer to the selection structure
 * @param note_id : Note identifier
 * @param tag_id  : Tag identifier
 * @return          Array with selection pointers
 */

void selection_free(GPtrArray *sel);
/* Frees memory used by a selection structure
 *
 * @param sel : Pointer to the selection structure
 */

#endif /* SELECTION_H */

