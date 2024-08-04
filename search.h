#ifndef _SEARCH_H
#define _SEARCH_H

#include "base.h"

/**
 * on_search_changed is called when the content entry area for searching has
 * changed. In response, the first ocurrent of the search content is selected
 * int the view moved to that area.
 *
 * Arguments:
 *
 *     widget: The entry search area widget
 *     data: a void pointer to the full text view widget
 */
gboolean on_search_changed(GtkWidget *widget, gpointer data);

gboolean on_search_forward(GtkWidget *widget, gpointer data);
gboolean on_search_backward(GtkWidget *widget, gpointer data);

#endif // _SEARCH_H
