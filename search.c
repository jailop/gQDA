#include "search.h"

extern struct gqda_app app;

gboolean on_search_changed(GtkWidget *widget, gpointer data)
{
    int found;
    const char *text;
    GtkTextBuffer *buffer;
    GtkTextView   *view;
    GtkTextIter start, match_start, match_end;
    text = gtk_entry_get_text(GTK_ENTRY(widget));
    if (!text)
        return FALSE;
    view = GTK_TEXT_VIEW(data);
    buffer = gtk_text_view_get_buffer(view);
    gtk_text_buffer_get_start_iter(buffer, &start);
    found = gtk_text_iter_forward_search(&start, text, 
            GTK_TEXT_SEARCH_CASE_INSENSITIVE,
            &match_start, &match_end, NULL);
    if (found) {
        gtk_text_buffer_select_range(buffer, &match_start, &match_end);
        gtk_text_view_scroll_to_iter(view, &match_start, 0.0, FALSE, 0.0, 0.0);
    }
    return FALSE;
}

gboolean on_search_forward(GtkWidget *widget, gpointer data)
{
    int found;
    const char *text;
    GtkTextBuffer *buffer;
    GtkTextView *view;
    GtkTextIter start, end, match_start, match_end;
    text = gtk_entry_get_text(GTK_ENTRY(data));
    if (!text)
        return FALSE;
    view = GTK_TEXT_VIEW(app.note_view);
    buffer = gtk_text_view_get_buffer(view);
    found = gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
    if (!found)
        return FALSE;
    found = gtk_text_iter_forward_search(&end, text,
            GTK_TEXT_SEARCH_CASE_INSENSITIVE,
            &match_start, &match_end, NULL);
    if (found) {
        gtk_text_buffer_select_range(buffer, &match_start, &match_end);
        gtk_text_view_scroll_to_iter(view, &match_start, 0.0, FALSE, 0.0, 0.0);
    }
    return FALSE;
}

gboolean on_search_backward(GtkWidget *widget, gpointer data)
{
    int found;
    const char *text;
    GtkTextBuffer *buffer;
    GtkTextView *view;
    GtkTextIter start, end, match_start, match_end;
    text = gtk_entry_get_text(GTK_ENTRY(data));
    if (!text)
        return FALSE;
    view = GTK_TEXT_VIEW(app.note_view);
    buffer = gtk_text_view_get_buffer(view);
    found = gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
    if (!found)
        return FALSE;
    found = gtk_text_iter_backward_search(&start, text,
            GTK_TEXT_SEARCH_CASE_INSENSITIVE,
            &match_start, &match_end, NULL);
    if (found) {
        gtk_text_buffer_select_range(buffer, &match_start, &match_end);
        gtk_text_view_scroll_to_iter(view, &match_start, 0.0, FALSE, 0.0, 0.0);
    }
    return FALSE;
}
