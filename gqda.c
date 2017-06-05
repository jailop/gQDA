#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "extension.h"
#include "base.h"
#include "xmlio.h"

GString *str;

struct gqda_app app;

gboolean extract_segment_from_note(GtkTreeModel *model, GtkTreePath *path,
                             GtkTreeIter *iter, gpointer data)
{
    int i;
    gchar *name;
    gchar *content;
    guint id, tag;
    struct selection *sel;
    gchar *segment;
    GtkTextIter start, end;
    tag = (guint) (*(guint *)data);
    gtk_tree_model_get(model, iter,
            NOTE_NAME, &name,
            NOTE_CONTENT, &content,
            NOTE_ID, &id,
            -1);
    GPtrArray *par = selection_get(app.selections, id, tag);
    if (par == NULL) 
        return FALSE;
    GtkTextBuffer *aux = gtk_text_buffer_new(NULL);
    gtk_text_buffer_set_text(aux, content, -1);
    gtk_text_buffer_get_start_iter(aux, &start);
    gtk_text_buffer_get_end_iter(aux, &end);
    for (i = 0; i < par->len; i++) {
        sel = g_ptr_array_index(par, i);
        if (sel == NULL)
            continue;
        gtk_text_iter_set_offset(&start, sel->x1);
        gtk_text_iter_set_offset(&end, sel->x2);
        segment = gtk_text_buffer_get_slice(aux, &start, &end, FALSE);
        g_string_append_printf(str, "Nota: %s\n\n%s\n\n\n", name, segment);
        free(segment);
    }
    g_object_unref(G_OBJECT(aux));
    return FALSE;
}

void extract_segments(guint id)
{
    GtkTextBuffer *buffer;
    str = g_string_new(NULL);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app.fragment_view));
    gtk_tree_model_foreach(app.note_model, extract_segment_from_note, &id);
    gtk_text_buffer_set_text(buffer, str->str, -1);
    g_string_free(str, TRUE);
}

void note_highlight_segment(GtkTextBuffer *buffer, GPtrArray *par, 
                            const char *hightype, gboolean update_view)
{
    int i;
    GtkTextIter start, end;
    struct selection *sel;
    if (par == NULL)
        return;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    for (i = 0; i < par->len; i++) {
        sel = (struct selection *) g_ptr_array_index(par, i);
        if (sel == NULL)
            continue;
        gtk_text_iter_set_offset(&start, sel->x1);
        gtk_text_iter_set_offset(&end, sel->x2);
        gtk_text_buffer_remove_all_tags(buffer, &start, &end);
        gtk_text_buffer_apply_tag_by_name(buffer, hightype , &start, &end);
        if (update_view)
            gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(app.note_view), &start, 
                                         0.3, FALSE, 0, 0.5);
    }
}

void note_highlight(int note_id, int tag_id)
{
    int i;
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    GPtrArray *note, *par;
    gboolean is_selected;
    par = selection_get(app.selections, note_id, tag_id);
    /* Anyway, the text highlighted is cleaned */
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app.note_view));
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_remove_all_tags(buffer, &start, &end);
    if (note_id < 0 || tag_id < 0)
        return;
    if (app.selections == NULL)
        return;
    note = g_ptr_array_index(app.selections, note_id);
    if (note == NULL)
        return;
    is_selected = gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
    for (i = 0; i < note->len; i++) {
        if (i == tag_id)
            continue;
        par = g_ptr_array_index(note, i); 
        note_highlight_segment(buffer, par, "shadowed", !is_selected);
    }
    par = g_ptr_array_index(note, tag_id);
    note_highlight_segment(buffer, par, "highlighted", !is_selected);
}

gboolean on_tree_row_activated(GtkTreeView *tree, GtkTreePath *path,
        gboolean is_main)
{
    guint id;
    char *memo = NULL;
    GtkTreeIter iter;
    GtkTreeModel *store =
        gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
    gtk_tree_model_get_iter(store, &iter, path);
    gtk_tree_model_get(store, &iter,
            TAG_MEMO, &memo,
            TAG_ID, &id,
            -1);
    app.tag_active = id;
    if (is_main) {
        if (!memo)
            memo = "";
        gtk_text_buffer_set_text(app.memo_buffer, memo, -1);
        extract_segments(id);
    }
    else
        note_highlight(app.note_active, app.tag_active);
    return FALSE;
}

gboolean on_main_row_activated(GtkTreeView *tree, GtkTreePath *path,
        GtkTreeViewColumn *column, gpointer userdata)
{
    on_tree_row_activated(tree, path, TRUE);
    return FALSE;
}

gboolean on_tag_row_activated(GtkTreeView *tree, GtkTreePath *path,
        GtkTreeViewColumn *column, gpointer userdata)
{
    on_tree_row_activated(tree, path, FALSE);
    return FALSE;
}

int main(int argc, char **argv)
{
    GtkTextBuffer *buffer;

    gtk_init(&argc, &argv);
    GtkBuilder *builder = gtk_builder_new_from_resource("/org/falible/gQDA/gqda.ui");

    // Loading global objects
    app.window = GTK_WIDGET(gtk_builder_get_object(builder, "Window"));
    app.note_tree = GTK_WIDGET(gtk_builder_get_object(builder, "NoteTree"));
    app.note_view = GTK_WIDGET(gtk_builder_get_object(builder, "NoteView"));
    app.tag_tree = GTK_WIDGET(gtk_builder_get_object(builder, "TagTree"));
    app.main_tree = GTK_WIDGET(gtk_builder_get_object(builder, "MainTree"));
    app.memo_view = GTK_WIDGET(gtk_builder_get_object(builder, "MemoView"));
    app.fragment_view = GTK_WIDGET(gtk_builder_get_object(builder, "FragmentView"));
    
    app.selections = NULL;
    app.note_counter = 0;
    app.tag_counter = 0;
    app.note_active = -1;
    app.tag_active = -1;
    app.main_active = -1;
    app.file = NULL;

    app.note_model = gtk_tree_view_get_model(GTK_TREE_VIEW(app.note_tree));
    app.tree_model = gtk_tree_view_get_model(GTK_TREE_VIEW(app.main_tree));
    app.memo_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app.memo_view)); 

    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(G_OBJECT(builder));

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app.note_view));
    gtk_text_buffer_create_tag(buffer, "highlighted",
            "background", "yellow", NULL);
    gtk_text_buffer_create_tag(buffer, "shadowed",
            "background", "lightyellow", NULL);

    gtk_widget_show_all(app.window);

    /* if a filename was given on the command line
     * then the file will be openned
     */
    if (argc > 0) {
        app.file = argv[1];
        xml_open(&app);
    }

    gtk_main();
    return 0;
}

gboolean on_note_add(GtkWidget *widget, gpointer userdata)
{
    GtkWidget *file_chooser = gtk_file_chooser_dialog_new("Open file",
        GTK_WINDOW(app.window), GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Accept", GTK_RESPONSE_ACCEPT,
        "_Cancel", GTK_RESPONSE_CANCEL,
        NULL);
    int res = gtk_dialog_run(GTK_DIALOG(file_chooser));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filepath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser));
        GString *s = g_string_new(NULL);
        FILE *fin = fopen(filepath, "r");
        char c;
        while ((c = fgetc(fin)) != EOF)
            g_string_append_c(s, c);
        fclose(fin);

        GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app.note_view));
        gtk_text_buffer_set_text(tb, s->str, s->len);

        GtkListStore *note_store =
            GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(app.note_tree)));
        GtkTreeIter iter;
        char *filename = strrchr(filepath, '/') + 1;
        gtk_list_store_append(note_store, &iter);
        app.note_active = app.note_counter;
        gtk_list_store_set(note_store, &iter,
                NOTE_NAME, filename,
                NOTE_CONTENT, s->str,
                NOTE_ID, app.note_counter++,
                -1);
        tree_activate_row(GTK_TREE_VIEW(app.note_tree), &iter);
    }
    gtk_widget_destroy(file_chooser);
    return FALSE;
}

gboolean on_quit(GtkWidget *widget, gpointer userdata)
{
    gtk_main_quit();
    return FALSE;
}

gboolean on_note_tree_row_activated(GtkTreeView *tree, GtkTreePath *path,
        GtkTreeViewColumn *column, gpointer userdata)
{
    gchar *name;
    gchar *content;
    guint id;
    GtkTreeModel *note_store;
    GtkTreeIter iter;
    GtkTextBuffer *text_buffer;

    note_store = gtk_tree_view_get_model(GTK_TREE_VIEW(app.note_tree));
    gtk_tree_model_get_iter(note_store, &iter, path);

    gtk_tree_model_get(note_store, &iter,
            NOTE_NAME, &name,
            NOTE_CONTENT, &content,
            NOTE_ID, &id,
            -1);

    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app.note_view));
    gtk_text_buffer_set_text(text_buffer, content, strlen(content));
    app.note_active = id;
    note_highlight(app.note_active, app.tag_active);
    return FALSE;
}

gboolean on_project_new(GtkWidget *widget, gpointer data)
{
    return FALSE;
}

gboolean on_project_open(GtkWidget *widget, gpointer data)
{
    GtkWidget *file_chooser = gtk_file_chooser_dialog_new("Open project",
        GTK_WINDOW(app.window), GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Accept", GTK_RESPONSE_ACCEPT,
        "_Cancel", GTK_RESPONSE_CANCEL,
        NULL);
    int res = gtk_dialog_run(GTK_DIALOG(file_chooser));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        if (app.file)
            g_free(app.file);
        app.file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser));
        if (app.file == NULL) {
            fprintf(stderr, "Any filename project was given\n");
            return FALSE;
        }
        xml_open(&app);
    }
    gtk_widget_destroy(file_chooser);
    return FALSE;
}

gboolean on_project_save_as(GtkWidget *widget, gpointer data)
{
    int res;
    GtkWidget *file_chooser_widget;
    GtkFileChooser *file_chooser;
    /* Configuring file chooser dialog */ 
    file_chooser_widget = gtk_file_chooser_dialog_new("Save project",
        GTK_WINDOW(app.window), GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Accept", GTK_RESPONSE_ACCEPT,
        "_Cancel", GTK_RESPONSE_CANCEL,
        NULL);
    file_chooser = GTK_FILE_CHOOSER(file_chooser_widget);
    gtk_file_chooser_set_do_overwrite_confirmation(file_chooser, TRUE);
    /* Cheching if there is a filename assigned */ 
    if (app.file)
         gtk_file_chooser_set_current_name(file_chooser, app.file);
    else
        gtk_file_chooser_set_current_name(file_chooser, "Untitle.gqda");
    /* Goint to show the file chooser dialog */ 
    res = gtk_dialog_run(GTK_DIALOG(file_chooser_widget));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        if (app.file)
            g_free(app.file);
        app.file = gtk_file_chooser_get_filename(file_chooser);
        xml_write(&app);  /* All is ok, so write it */
    }
    /* Cleaning up */
    gtk_widget_destroy(file_chooser_widget);
    return FALSE;
}

gboolean on_project_save(GtkWidget *widget, gpointer data)
{
    if (app.file)
        xml_write(&app);
    else
        on_project_save_as(widget, data);
    return FALSE;
}

gboolean on_text_selection(GtkWidget *widget, gpointer data)
{
    gboolean is_selected;
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    gint a, b;
    if (app.note_active < 0 || app.tag_active < 0)
        return FALSE;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app.note_view));
    is_selected = gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
    if (is_selected) {
        a = gtk_text_iter_get_offset(&start);
        b = gtk_text_iter_get_offset(&end);
        selection_add(&(app.selections), app.note_active, app.tag_active, a, b);
        gtk_text_buffer_apply_tag_by_name(buffer, "highlighted" , &start, &end);
        /* Unselecting text and moving cursor to end position */
        gtk_text_buffer_place_cursor(buffer, &end);
    }
    return FALSE;
}

gboolean on_text_unselection(GtkWidget *widget, gpointer data)
{
    int a, b;
    int cursor_position;
    struct selection *sel;
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app.note_view));
    g_object_get(G_OBJECT(buffer), "cursor-position", &cursor_position, NULL);
    sel = selection_remove(app.selections, app.note_active, app.tag_active, 
                           cursor_position);
    if (sel) {
        a = sel->x1;
        b = sel->x2;
        free(sel);
        gtk_text_buffer_get_iter_at_offset(buffer, &start, a);
        gtk_text_buffer_get_iter_at_offset(buffer, &end, b);
        gtk_text_buffer_remove_tag_by_name(buffer, "highlighted", &start, &end);
    }
    return FALSE;
}

void on_note_cell_edited(GtkCellRendererText *renderer,
                         gchar *path,
                         gchar *new_text,
                         gpointer user_data)
{

}

gboolean tag_add(GtkWidget *widget, gpointer data, 
                 GtkTreeView *tree_view, gboolean is_child)
{
    const gchar *text;
    GtkTreeStore *store;
    GtkTreeIter parent;
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkEntry *entry;
    GtkTreePath *path;
    GtkTreeSelection *selected_row;
    gint depth = 0;


    entry = GTK_ENTRY (data);
    text = gtk_entry_get_text (entry);


    if (strlen(text) > 0) {
        model = gtk_tree_view_get_model(tree_view);
        store = GTK_TREE_STORE(model);
       
        selected_row = gtk_tree_view_get_selection(tree_view);
        gtk_tree_selection_get_selected(selected_row, &model, &iter);

        if (is_child) {
            parent = iter;
        }
        else {
            path = gtk_tree_model_get_path(model, &iter);
            depth = gtk_tree_path_get_depth(path);  
            if (depth > 1) {
                gtk_tree_path_up(path); 
                gtk_tree_model_get_iter(model, &parent, path);
            }
        }
        if (depth > 1 || is_child)
            gtk_tree_store_append(store, &iter, &parent);
        else 
            gtk_tree_store_append(store, &iter, NULL);


        gtk_tree_store_set(store, &iter,
                TAG_NAME, text,
                TAG_MEMO, "",
                TAG_ID, app.tag_counter,
                -1);
        app.tag_counter++;

        gtk_entry_set_text(entry, "");
        tree_activate_row(tree_view, &iter);
    }
    return TRUE;
}

gboolean on_tag_add(GtkWidget *widget, gpointer data)
{
    return tag_add(widget, data, GTK_TREE_VIEW(app.tag_tree), FALSE);
}

gboolean on_tag_child_add(GtkWidget *widget, gpointer data)
{
    return tag_add(widget, data, GTK_TREE_VIEW(app.tag_tree), TRUE);
}

gboolean on_main_add(GtkWidget *widget, gpointer data)
{
    return tag_add(widget, data, GTK_TREE_VIEW(app.main_tree), FALSE);
}

gboolean on_main_child_add(GtkWidget *widget, gpointer data)
{
    return tag_add(widget, data, GTK_TREE_VIEW(app.main_tree), TRUE);
}

gboolean tag_tree_view_changed(GtkWidget *widget, gpointer data)
{
    GtkAdjustment *adj;
    adj = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(widget));
    gtk_adjustment_set_value(adj, 
            gtk_adjustment_get_upper(adj) -
            gtk_adjustment_get_page_size(adj));
    return FALSE;
}

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

gboolean on_memo_changed(GtkTextBuffer *buffer, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeSelection *selected_row;
    const char *text;
    GtkTextIter start, end;
    GtkTreeStore *store;
    selected_row = gtk_tree_view_get_selection(GTK_TREE_VIEW(app.main_tree));
    if (!selected_row)
        return FALSE;
    gtk_tree_selection_get_selected(selected_row, &(app.tree_model), &iter);
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(app.main_tree)));
    gtk_tree_store_set(store, &iter,
                TAG_MEMO, text,
                -1);
    return FALSE;
}
