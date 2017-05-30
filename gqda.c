#include <string.h>
#include <gtk/gtk.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>
#include <libxml/encoding.h>
#include "extension.h"
#include "base.h"

/*
#define XML_ENCODING "ISO-8859-1"
*/
#define XML_ENCODING "UTF-8"

int xml_read_note(xmlTextReaderPtr reader, GtkListStore *store)
{
    gchar *name;
    gchar *content;
    GtkTreeIter iter;
    int id;
    id = atoi((char *) xmlTextReaderGetAttribute(reader, BAD_CAST "id"));
    xmlTextReaderRead(reader);
    xmlTextReaderRead(reader);
    name = astrcpy((char *) xmlTextReaderReadString(reader));
    xmlTextReaderRead(reader);
    xmlTextReaderRead(reader);
    xmlTextReaderRead(reader);
    content = astrcpy((char *) xmlTextReaderValue(reader));
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
        NOTE_NAME, name,
        NOTE_CONTENT, content,
        NOTE_ID, id,
        -1);
    note_counter++;
    return 1;
}

int xml_read_tag(xmlTextReaderPtr reader, GtkTreeStore *store)
{
    int i;
    gchar *name;
    gchar *memo;
    GtkTreeIter iter; 
    GtkTreeIter *iter_parent;
    static GPtrArray *iterarray = NULL;
    xmlChar *tmp;
    int id;
    int parent;
    tmp = xmlTextReaderGetAttribute(reader, BAD_CAST "id");
    id = atoi((char *) tmp);
    xmlFree(tmp);
    tmp = xmlTextReaderGetAttribute(reader, BAD_CAST "parent");
    if (tmp) {
        parent = atoi((char *) tmp);
        xmlFree(tmp);
    }
    else
        parent = -1;
    xmlTextReaderRead(reader);
    xmlTextReaderRead(reader);
    name = astrcpy((char *) xmlTextReaderReadString(reader));
    memo = "";
    if (parent == -1) 
        gtk_tree_store_append(store, &iter, NULL);
    else { 
        iter_parent = g_ptr_array_index(iterarray, parent); 
        if (iter_parent != NULL)
            gtk_tree_store_append(store, &iter, iter_parent);
    }
    if (iterarray == NULL)
        iterarray = g_ptr_array_new();
    if (iterarray->len <= id)
        for (i = iterarray->len; i <= id; i++)
            g_ptr_array_insert(iterarray, i, NULL);
    g_ptr_array_insert(iterarray, id, gtk_tree_iter_copy(&iter));
    gtk_tree_store_set(store, &iter,
        TAG_NAME, name,
        TAG_MEMO, memo,
        TAG_ID, id,
        -1);
    tag_counter++;
    /*
    for (i = 0; i < iterarray->len; i++) {
        iter_parent = g_ptr_array_index(iterarray, i);
        if (iter_parent)
            free(iter_parent);
    }
    g_ptr_array_free(iterarray, FALSE);
    */
    return 1;
}

int xml_open_file(const char *filename)
{
    char *tagname;
    int ret;
    GtkListStore *note_store;
    GtkTreeStore *tag_store;
    note_store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(note_tree)));
    gtk_list_store_clear(note_store);
    note_counter = 0;
    tag_store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tag_tree)));
    gtk_tree_store_clear(tag_store);
    tag_counter = 0;
    xmlTextReaderPtr reader = xmlReaderForFile(filename, NULL, 0);
    ret = xmlTextReaderRead(reader);
    while (ret == 1) {
        tagname = (char *) xmlTextReaderConstName(reader);
        if (strcmp(tagname, "note") == 0 &&
            xmlTextReaderNodeType(reader) == 1) {
            xml_read_note(reader, note_store);
        }
        else if (strcmp(tagname, "tag") == 0 &&
                 xmlTextReaderNodeType(reader) == 1) {
            xml_read_tag(reader, tag_store);
        }
        else if (strcmp(tagname, "selection") == 0 &&
                 xmlTextReaderNodeType(reader) == 1) {
            int note_id, tag_id, start, end;
            note_id = atoi((char *) xmlTextReaderGetAttribute(reader,
                        BAD_CAST "note_id"));
            tag_id = atoi((char *) xmlTextReaderGetAttribute(reader,
                        BAD_CAST "tag_id"));
            start = atoi((char *) xmlTextReaderGetAttribute(reader,
                        BAD_CAST "start"));
            end = atoi((char *) xmlTextReaderGetAttribute(reader,
                        BAD_CAST "end"));
            selection_add(&selections, note_id, tag_id, start, end);
        }
        ret = xmlTextReaderRead(reader);
    }
    xmlFreeTextReader(reader);
    if (!tree_activate_first_row(GTK_TREE_VIEW(note_tree)))
        fprintf(stderr, "First row of note tree could not be selected\n");
    if (!tree_activate_first_row(GTK_TREE_VIEW(tag_tree)))
        fprintf(stderr, "First row of tag tree could not be selected\n");
    return 1;
}

gboolean xml_write_each_note(GtkTreeModel *model, GtkTreePath *path,
                             GtkTreeIter *iter, gpointer data)
{
    int id;
    gchar *name;
    gchar *content;
    xmlTextWriterPtr writer;
    writer = (xmlTextWriterPtr) data;
    gtk_tree_model_get(model, iter,
            NOTE_NAME, &name,
            NOTE_CONTENT, &content,
            NOTE_ID, &id,
            -1);
#ifdef DEBUG
    printf("xml_write_each_node: it is ready to start writing\n");
    printf("xml_write_each_node: name %s id %d\n", name, id);
#endif
    xmlTextWriterStartElement(writer, BAD_CAST "note");
    xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "id", "%d", id);
    xmlTextWriterWriteElement(writer, BAD_CAST "name", BAD_CAST name);
    xmlTextWriterWriteElement(writer, BAD_CAST "content", BAD_CAST content);
    xmlTextWriterEndElement(writer);
#ifdef DEBUG
    printf("xml_write_each_node: it is finishing to write\n");
#endif
    return FALSE;
}

int xml_write_notes(xmlTextWriterPtr writer)
{
    GtkTreeModel *store;
    
    store = gtk_tree_view_get_model(GTK_TREE_VIEW(note_tree));
    
#ifdef DEBUG
    printf("xml_write_notes: it is ready to start writing\n");
#endif
    xmlTextWriterStartElement(writer, BAD_CAST "notes");
    gtk_tree_model_foreach(store, xml_write_each_note, writer);
    xmlTextWriterEndElement(writer);
#ifdef DEBUG
    printf("xml_write_notes: it is finishing to write\n");
#endif
    
    return 1;
}

gboolean xml_write_each_tag(GtkTreeModel *model, GtkTreePath *path, 
                            GtkTreeIter *iter, gpointer data)
{
    int id;
    gchar *name;
    gchar *memo;
    guint tag_parent;
    gboolean has_parent;
    GtkTreeIter parent;
    xmlTextWriterPtr writer;
    writer = (xmlTextWriterPtr) data;
    gtk_tree_model_get(model, iter,
            TAG_NAME, &name,
            TAG_MEMO, &memo,
            TAG_ID, &id,
            -1);
    tag_parent = -1;
    has_parent = gtk_tree_model_iter_parent(model, &parent, iter);
    if (has_parent)
        gtk_tree_model_get(model, &parent, TAG_ID, &tag_parent, -1);
#ifdef DEBUG
    printf("xml_write_each tag: name %s id %d\n", name, id);
#endif

    xmlTextWriterStartElement(writer, BAD_CAST "tag");
    xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "id", "%d", id);
    xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "parent", "%d", tag_parent);
    xmlTextWriterWriteElement(writer, BAD_CAST "name", BAD_CAST name);
    xmlTextWriterWriteElement(writer, BAD_CAST "memo", BAD_CAST memo);
    xmlTextWriterEndElement(writer);
    return FALSE;
}

int xml_write_tags(xmlTextWriterPtr writer)
{
    GtkTreeModel *store;
    store = gtk_tree_view_get_model(GTK_TREE_VIEW(tag_tree));
    xmlTextWriterStartElement(writer, BAD_CAST "tags");
    gtk_tree_model_foreach(store, xml_write_each_tag, writer);
    xmlTextWriterEndElement(writer);
#ifdef DEBUG
    printf("xml_write_tags: it is finishing to write\n");
#endif
    return 1;
}

int xml_write_selections(xmlTextWriterPtr writer)
{
    int i, j, k;
    GPtrArray *tag, *sel;
    struct selection *s;
    if (selections == NULL)
        return 0;
    xmlTextWriterStartElement(writer, BAD_CAST "selections");
    for (i = 0; i < selections->len; i++) {
        tag = g_ptr_array_index(selections, i);
        if (tag == NULL)
            continue;
        for (j = 0; j < tag->len; j++) {
            sel = g_ptr_array_index(tag, j);
            if (sel == NULL)
                continue;
            for (k = 0; k < sel->len; k++) {
                s = g_ptr_array_index(sel, k);
                xmlTextWriterStartElement(writer, BAD_CAST "selection");
                xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "note_id", "%d", i);
                xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "tag_id", "%d", j);
                xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "start", "%d", s->x1);
                xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "end", "%d", s->x2);
                xmlTextWriterEndElement(writer);
            }
        }
    }
    xmlTextWriterEndElement(writer);
    return 1;
}

int xml_write_file(const char *filename)
{
    xmlTextWriterPtr writer;

    if (filename == NULL) {
        fprintf(stderr, "xml_write_file: filename is NULL\n");
        return 0;
    }
    writer = xmlNewTextWriterFilename(filename, 0);
    if (writer == NULL) {
        fprintf(stderr, "xml_write_file: error creating writer\n");
        return 0;
    }
    
#ifdef DEBUG
    printf("xml_write_file: it is ready to start writing\n");
#endif
    xmlTextWriterStartDocument(writer, NULL, XML_ENCODING, NULL);
    xmlTextWriterStartElement(writer, BAD_CAST "data");
    
    if (note_counter > 0) 
        xml_write_notes(writer);
    if (tag_counter > 0) 
        xml_write_tags(writer);
    xml_write_selections(writer);

#ifdef DEBUG
    printf("xml_write_file: it is finishing to write\n");
#endif
    xmlTextWriterEndElement(writer);
    xmlTextWriterEndDocument(writer);
    
    xmlFreeTextWriter(writer);
    return 1;
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
            gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(note_view), &start, 
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
    par = selection_get(selections, note_id, tag_id);
    /* Anyway, the text highlighted is cleaned */
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note_view));
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_remove_all_tags(buffer, &start, &end);
    if (note_id < 0 || tag_id < 0)
        return;
    if (selections == NULL)
        return;
    note = g_ptr_array_index(selections, note_id);
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

gboolean on_tag_tree_row_activated(GtkTreeView *tree, GtkTreePath *path,
        GtkTreeViewColumn *column, gpointer userdata)
{
    GtkTreeModel *store =
        gtk_tree_view_get_model(GTK_TREE_VIEW(tag_tree));
    GtkTreeIter iter;
    gtk_tree_model_get_iter(store, &iter, path);
    tag_iter = iter;
    tag_iter_set = TRUE;

    gchar *name;
    gchar *memo;
    guint id;
    gtk_tree_model_get(store, &iter,
            TAG_NAME, &name,
            TAG_MEMO, &memo,
            TAG_ID, &id,
            -1);
    tag_active = id;
    note_highlight(note_active, tag_active);
    return FALSE;
}

int main(int argc, char **argv)
{
    GtkTextBuffer *buffer;

    gtk_init(&argc, &argv);
    GtkBuilder *builder = gtk_builder_new_from_file("gqda.glade");

    // Loading global objects
    window = GTK_WIDGET(gtk_builder_get_object(builder, "Window"));
    note_tree = GTK_WIDGET(gtk_builder_get_object(builder, "NoteTree"));
    note_view = GTK_WIDGET(gtk_builder_get_object(builder, "NoteView"));
    tag_tree = GTK_WIDGET(gtk_builder_get_object(builder, "TagTree"));

    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(G_OBJECT(builder));

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note_view));
    gtk_text_buffer_create_tag(buffer, "highlighted",
            "background", "yellow", NULL);
    gtk_text_buffer_create_tag(buffer, "shadowed",
            "background", "lightyellow", NULL);

    gtk_widget_show_all(window);

    /* if a filename was given on the command line
     * then the file will be openned
     */
    if (argc > 0) {
        file = argv[1];
        xml_open_file(file);
    }

    gtk_main();
    return 0;
}

gboolean on_note_add(GtkWidget *widget, gpointer userdata)
{
    GtkWidget *file_chooser = gtk_file_chooser_dialog_new("Open file",
        GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN,
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

        GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note_view));
        gtk_text_buffer_set_text(tb, s->str, s->len);

        GtkListStore *note_store =
            GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(note_tree)));
        GtkTreeIter iter;
        char *filename = strrchr(filepath, '/') + 1;
        gtk_list_store_append(note_store, &iter);
        note_active = note_counter;
        gtk_list_store_set(note_store, &iter,
                NOTE_NAME, filename,
                NOTE_CONTENT, s->str,
                NOTE_ID, note_counter++,
                -1);
        tree_activate_row(GTK_TREE_VIEW(note_tree), &iter);
    }
    gtk_widget_destroy(file_chooser);
    return FALSE;
}

gboolean on_quit(GtkWidget *widget, gpointer userdata)
{
    gtk_main_quit();
    return FALSE;
}

/*
gboolean on_text_view_selection(GtkWidget *widget, gpointer userdata)
{
    GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note_view));
    GtkTextIter start, end;
    gboolean selected = gtk_text_buffer_get_selection_bounds(tb, &start, &end);
    if (selected)
        printf("%d, %d\n",
                gtk_text_iter_get_offset(&start),
                gtk_text_iter_get_offset(&end));
    return FALSE;
}
*/

gboolean on_note_tree_row_activated(GtkTreeView *tree, GtkTreePath *path,
        GtkTreeViewColumn *column, gpointer userdata)
{
    gchar *name;
    gchar *content;
    guint id;
    GtkTreeModel *note_store;
    GtkTreeIter iter;
    GtkTextBuffer *text_buffer;

    note_store = gtk_tree_view_get_model(GTK_TREE_VIEW(note_tree));
    gtk_tree_model_get_iter(note_store, &iter, path);

    gtk_tree_model_get(note_store, &iter,
            NOTE_NAME, &name,
            NOTE_CONTENT, &content,
            NOTE_ID, &id,
            -1);

    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note_view));
    gtk_text_buffer_set_text(text_buffer, content, strlen(content));
    note_active = id;
    note_highlight(note_active, tag_active);
    return FALSE;
}

gboolean on_project_new(GtkWidget *widget, gpointer data)
{
    return FALSE;
}

gboolean on_project_open(GtkWidget *widget, gpointer data)
{
    GtkWidget *file_chooser = gtk_file_chooser_dialog_new("Open project",
        GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Accept", GTK_RESPONSE_ACCEPT,
        "_Cancel", GTK_RESPONSE_CANCEL,
        NULL);
    int res = gtk_dialog_run(GTK_DIALOG(file_chooser));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        if (file)
            g_free(file);
        file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser));
        if (file == NULL) {
            fprintf(stderr, "Any filename project was given\n");
            return FALSE;
        }
        xml_open_file(file);
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
        GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Accept", GTK_RESPONSE_ACCEPT,
        "_Cancel", GTK_RESPONSE_CANCEL,
        NULL);
    file_chooser = GTK_FILE_CHOOSER(file_chooser_widget);
    gtk_file_chooser_set_do_overwrite_confirmation(file_chooser, TRUE);
    /* Cheching if there is a filename assigned */ 
    if (file)
         gtk_file_chooser_set_current_name(file_chooser, file);
    else
        gtk_file_chooser_set_current_name(file_chooser, "Untitle.gqda");
    /* Goint to show the file chooser dialog */ 
    res = gtk_dialog_run(GTK_DIALOG(file_chooser_widget));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        if (file)
            g_free(file);
        file = gtk_file_chooser_get_filename(file_chooser);
        #ifdef DEBUG
        printf("function: on_project save as: filename %s\n", file);
        #endif
        xml_write_file(file);  /* All is ok, so write it */
    }
    /* Cleaning up */
    gtk_widget_destroy(file_chooser_widget);
    return FALSE;
}

gboolean on_project_save(GtkWidget *widget, gpointer data)
{
    if (file)
        xml_write_file(file);
    else
        on_project_save_as(widget, data);
    return FALSE;
}

gboolean on_text_highlight(GtkWidget *widget, gpointer data)
{
    gboolean is_selected;
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    int a, b;
    if (note_active < 0 || tag_active < 0)
        return FALSE;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note_view));
    is_selected = gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
    if (is_selected) {
        a = gtk_text_iter_get_offset(&start);
        b = gtk_text_iter_get_offset(&end);
        selection_add(&selections, note_active, tag_active, a, b);
        gtk_text_buffer_apply_tag_by_name(buffer, "highlighted" , &start, &end);
    }
    return FALSE;
}

void on_note_cell_edited(GtkCellRendererText *renderer,
                         gchar *path,
                         gchar *new_text,
                         gpointer user_data)
{

}

gboolean tag_add(GtkWidget *widget, gpointer data, gboolean is_child)
{
    const gchar *tag_text;
    GtkTreeStore *store;
    GtkTreeIter iter, parent;
    GtkTreeModel *model;
    GtkEntry *entry;
    GtkTreePath *path;
    gint depth;

    #ifdef DEBUG
    printf("function: on_add_tag: %p %p\n", widget, data);
    #endif

    entry = GTK_ENTRY (data);
    tag_text = gtk_entry_get_text (entry);

    #ifdef DEBUG
    printf("on_add_tag: tag_text %s\n", tag_text);
    #endif

    if (strlen(tag_text) > 0) {
        model = gtk_tree_view_get_model(GTK_TREE_VIEW(tag_tree));
        store = GTK_TREE_STORE(model);

        if (tag_iter_set) {
            if (is_child) {
                parent = tag_iter;
            }
            else {
                path = gtk_tree_model_get_path(model, &tag_iter);
                depth = gtk_tree_path_get_depth(path);  
                if (depth > 1) {
                    gtk_tree_path_up(path); 
                    gtk_tree_model_get_iter(model, &parent, path);
                }
            }
            if (depth > 1)
                gtk_tree_store_append(store, &iter, &parent);
            else 
                gtk_tree_store_append(store, &iter, NULL);
        }
        else
            gtk_tree_store_append(store, &iter, NULL);


        gtk_tree_store_set(store, &iter,
                TAG_NAME, tag_text,
                TAG_MEMO, "",
                TAG_ID, tag_counter,
                -1);
        tag_active = tag_counter;
        tag_counter++;

        #ifdef DEBUG
        printf("on_add_tag: tag_active %d\n", tag_active);
        #endif

        gtk_entry_set_text(entry, "");
        tree_activate_row(GTK_TREE_VIEW(tag_tree), &iter);
    }
    return TRUE;
}

gboolean on_tag_add(GtkWidget *widget, gpointer data)
{
    return tag_add(widget, data, FALSE);
}

gboolean on_tag_child_add(GtkWidget *widget, gpointer data)
{
    return tag_add(widget, data, TRUE);
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
