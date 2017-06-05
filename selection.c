#include <stdio.h>
#include <stdlib.h>
#include "selection.h"

void selection_add(GPtrArray **sel, int note_id, int tag_id, int start, int end)
{
    GPtrArray *ptr, *tag, *par;
    ptr = *sel;
    int i;
    if (note_id < 0 || tag_id < 0)
        return;
    if (ptr == NULL)
        *sel = ptr = g_ptr_array_new();
    if (ptr->len <= note_id)
        for (i = ptr->len; i <= note_id; i++)
            g_ptr_array_insert(ptr, i, NULL);
    tag = g_ptr_array_index(ptr, note_id);
    if (tag == NULL) {
        tag = g_ptr_array_new();
        ptr->pdata[note_id] = tag;
    }
    if (tag->len <= tag_id)
        for (i = tag->len; i <= tag_id; i++)
            g_ptr_array_insert(tag, i, NULL);
    par = g_ptr_array_index(tag, tag_id);
    if (par == NULL) {
        par = g_ptr_array_new();
        tag->pdata[tag_id] = par;
    }
    struct selection *this = malloc(sizeof(struct selection));
    this->x1 = start;
    this->x2   = end;
    g_ptr_array_add(par, this);
}

GPtrArray *selection_get(GPtrArray *sel, int note_id, int tag_id)
{
    int i;
    GPtrArray *tag;
    if (sel == NULL)
        return NULL;
    if (sel->len <= note_id)
        for (i = sel->len; i <= note_id; i++)
            g_ptr_array_insert(sel, i, NULL);
    tag = g_ptr_array_index(sel, note_id);
    if (tag == NULL)
        return NULL;
    if (tag->len <= tag_id)
        for (i = tag->len; i <= tag_id; i++)
            g_ptr_array_insert(tag, i, NULL);
    return g_ptr_array_index(tag, tag_id);
}

void selection_free(GPtrArray *sel)
{
    GPtrArray *tag, *par;
    int i, j, k;
    for (i = 0; i < sel->len; i++) {
        tag = g_ptr_array_index(sel, i);
        for (j = 0; j < tag->len; j++) {
            par = g_ptr_array_index(tag, j);
            for (k = 0; k < par->len; k++)
                free(g_ptr_array_index(par, k));
            free(par);
        }
        g_ptr_array_free(tag, FALSE);
    }
    g_ptr_array_free(sel, FALSE);
}

void iter_array_add(GPtrArray **array, int index, GtkTreeIter *iter)
{
    int i;
    GtkTreeIter *cpy;
    GPtrArray *sel = *array;
    if (sel == NULL)
        *array = sel = g_ptr_array_new();
    if (sel->len <= index) {
        for (i = sel->len; i <= index; i++)
            g_ptr_array_add(sel, NULL);
    }
    cpy = gtk_tree_iter_copy(iter); 
    g_ptr_array_insert(sel, index, cpy);
}

GtkTreeIter *iter_array_get(GPtrArray *array, int index)
{
    if (array == NULL)
        return NULL;
    else
        return g_ptr_array_index(array, index);
}

void iter_array_free(GPtrArray *array)
{
    int i;
    GtkTreeIter *iter;
    for (i = 0; i < array->len; i++) {
        iter = g_ptr_array_index(array, i);
        if (iter != NULL)
            free(iter);
    }
    g_ptr_array_free(array, FALSE);
}

struct selection *selection_remove(GPtrArray *sel, int note_id, int tag_id, 
                                   int cursor_position)
{
    gint i;
    struct selection *ret;
    GPtrArray *it;
    it = g_ptr_array_index(sel, note_id);
    if (!it) return NULL;
    it = g_ptr_array_index(it, tag_id);
    if (!it) return NULL;
    for (i = 0; it->len; i++) {
        ret = g_ptr_array_index(it, i);
        if (ret)
            if (cursor_position >= ret->x1 && cursor_position <= ret->x2) {
                it->pdata[i] = NULL;
                return ret;
            }
    }
    return NULL;
}
