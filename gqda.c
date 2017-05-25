#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>
#include <libxml/encoding.h>

// define XML_ENCODING "ISO-8859-1"
#define XML_ENCODING "UTF-8"

GtkWidget *window,
          *note_tree,
          *note_view,
          *tag_tree;


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

struct selection {
    int start;
    int end;
};

char *astrcpy(const char *s)
{
    char *ret;
    int len = strlen(s) + 1;
    ret = malloc(sizeof(char) * len);
    strcpy(ret, s);
    return ret;
}

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
    this->start = start;
    this->end   = end;
    g_ptr_array_add(par, this);
}

GPtrArray *selection_get(GPtrArray *sel, int note_id, int tag_id)
{
    int i;
    GPtrArray *tag;
    if (sel == NULL) {
        fprintf(stderr, "Selection pointer is NULL\n");
        return NULL;
    }
    if (sel->len <= note_id)
        for (i = sel->len; i <= note_id; i++)
            g_ptr_array_insert(sel, i, NULL);
    tag = g_ptr_array_index(sel, note_id);
    if (tag == NULL) {
        fprintf(stderr, "Par pointer is NULL\n");
        return NULL;
    }
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
        free(tag);
    }
    free(sel);
}

gboolean tree_select_first_row(GtkTreeView *tree_view)
{
    GtkTreePath *path;
    GtkTreeViewColumn *column;
    path = gtk_tree_path_new_from_string("0");
    if (!path) {
        fprintf(stderr, "tree_select_first: path new first getting failed\n");
        return FALSE;
    }
    column = gtk_tree_view_get_column(tree_view, 0);
    if (!column) {
        fprintf(stderr, "tree_select_first: getting first coloumn failed\n");
        return FALSE;
    }
     gtk_tree_view_set_cursor(tree_view, path, column, TRUE);
    gtk_tree_view_row_activated(tree_view, path, column);
    gtk_tree_path_free(path);
    return TRUE;
}

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

int xml_read_tag(xmlTextReaderPtr reader, GtkListStore *store)
{
    gchar *name;
    gchar *memo;
    GtkTreeIter iter;
    int id;
    id = atoi((char *) xmlTextReaderGetAttribute(reader, BAD_CAST "id"));
    xmlTextReaderRead(reader);
    xmlTextReaderRead(reader);
    name = astrcpy((char *) xmlTextReaderReadString(reader));
    memo = "";
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 
        TAG_NAME, name, 
        TAG_MEMO, memo, 
        TAG_ID, id, 
        -1);
    tag_counter++;
    return 1;
}

int xml_open_file(const char *filename)
{
    char *tagname;
    int ret;
    GtkListStore *note_store, *tag_store;
    note_store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(note_tree))); 
    gtk_list_store_clear(note_store);
    note_counter = 0;
    tag_store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tag_tree))); 
    gtk_list_store_clear(tag_store);
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
    if (!tree_select_first_row(GTK_TREE_VIEW(note_tree)))
        fprintf(stderr, "First row of note tree could not be selected\n");
    if (!tree_select_first_row(GTK_TREE_VIEW(tag_tree)))
        fprintf(stderr, "First row of tag tree could not be selected\n");
    return 1;
}

gboolean xml_write_each_note(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
    int id;
    gchar *name;
    gchar *content;
    xmlTextWriterPtr writer = (xmlTextWriterPtr) data;
    gtk_tree_model_get(model, iter,
            NOTE_NAME, &name,
            NOTE_CONTENT, &content,
            NOTE_ID, &id,
            -1);
    xmlTextWriterStartElement(writer, BAD_CAST "note");
    xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "id", "%d", id);
    xmlTextWriterWriteElement(writer, BAD_CAST "name", BAD_CAST name);
    xmlTextWriterWriteElement(writer, BAD_CAST "content", BAD_CAST content);
    xmlTextWriterEndElement(writer);
    return FALSE;
}

int xml_write_notes(xmlTextWriterPtr writer)
{
    GtkTreeModel *store = gtk_tree_view_get_model(GTK_TREE_VIEW(note_tree)); 
    xmlTextWriterStartElement(writer, BAD_CAST "notes");
    gtk_tree_model_foreach(store, xml_write_each_note, writer); 
    xmlTextWriterEndElement(writer);
    return 1;
}

gboolean xml_write_each_tag(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
    xmlTextWriterPtr writer = (xmlTextWriterPtr) data;
    gchar *name;
    gchar *memo;
    int id;
    gtk_tree_model_get(model, iter,
            TAG_NAME, &name,
            TAG_MEMO, &memo,
            TAG_ID, &id,
            -1);
    xmlTextWriterStartElement(writer, BAD_CAST "tag");
    xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "id", "%d", id);
    xmlTextWriterWriteElement(writer, BAD_CAST "name", BAD_CAST name);
    xmlTextWriterWriteElement(writer, BAD_CAST "memo", BAD_CAST memo);
    xmlTextWriterEndElement(writer);
    return FALSE;
}

int xml_write_tags(xmlTextWriterPtr writer)
{
    GtkTreeModel *store = gtk_tree_view_get_model(GTK_TREE_VIEW(tag_tree)); 
    xmlTextWriterStartElement(writer, BAD_CAST "tags");
    gtk_tree_model_foreach(store, xml_write_each_tag, writer); 
    xmlTextWriterEndElement(writer);
    return 1;
}

int xml_write_selections(xmlTextWriterPtr writer)
{
    int i, j, k;
    GPtrArray *tag, *sel;
    struct selection *s;
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
                xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "start", "%d", s->start);
                xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "end", "%d", s->end);
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
    writer = xmlNewTextWriterFilename(filename, 0);
    if (writer == NULL) {
        fprintf(stderr, "xmlWriter failure: error creating writer\n");
        return 0;
    }
    xmlTextWriterStartDocument(writer, NULL, XML_ENCODING, NULL);
    xmlTextWriterStartElement(writer, BAD_CAST "data");
    xml_write_notes(writer);
    xml_write_tags(writer);
    xml_write_selections(writer);
    xmlTextWriterEndElement(writer);
    xmlTextWriterEndDocument(writer);
    xmlFreeTextWriter(writer);
    return 1;
}



void note_highlight(int note_id, int tag_id)
{
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    struct selection *sel;
    int i;
    if (note_id < 0 || tag_id < 0)
        return;
    if (selections == NULL)
        return;
    GPtrArray *par = selection_get(selections, note_id, tag_id);
    if (par == NULL)
        return;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note_view));
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_remove_all_tags(buffer, &start, &end);
    for (i = 0; i < par->len; i++) {
        sel = (struct selection *) g_ptr_array_index(par, i);
        if (sel == NULL) {
            fprintf(stderr, "Par Selection is NULL.\n");
            continue;
        }
        gtk_text_iter_set_offset(&start, sel->start);
        gtk_text_iter_set_offset(&end, sel->end);
        gtk_text_buffer_apply_tag_by_name(buffer, "highlighted" , &start, &end);
    }
}

gboolean on_tag_tree_row_activated(GtkTreeView *tree, GtkTreePath *path, 
        GtkTreeViewColumn *column, gpointer userdata)
{
    GtkTreeModel *store =
        gtk_tree_view_get_model(GTK_TREE_VIEW(tag_tree)); 
    GtkTreeIter iter;
    gtk_tree_model_get_iter(store, &iter, path);
    
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

gboolean on_note_open(GtkWidget *widget, gpointer userdata)
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
        // note_active = note_counter;
        gtk_list_store_set(note_store, &iter, 
                NOTE_NAME, filename, 
                NOTE_CONTENT, s->str, 
                NOTE_ID, note_counter++, 
                -1);  
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
        file = astrcpy(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser)));
        xml_open_file(file);
    }
    gtk_widget_destroy(file_chooser);
    return FALSE;
}

gboolean on_project_save_as(GtkWidget *widget, gpointer data)
{
    GtkWidget *file_chooser = gtk_file_chooser_dialog_new("Save project",
        GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Accept", GTK_RESPONSE_ACCEPT,
        "_Cancel", GTK_RESPONSE_CANCEL,
        NULL);
    gtk_file_chooser_set_do_overwrite_confirmation 
        (GTK_FILE_CHOOSER(file_chooser), TRUE);
    if (file)
         gtk_file_chooser_set_current_name
             (GTK_FILE_CHOOSER(file_chooser), file);
    else  
        gtk_file_chooser_set_current_name
            (GTK_FILE_CHOOSER(file_chooser), "Untitle.gqda");
    int res = gtk_dialog_run(GTK_DIALOG(file_chooser));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        if (file) 
            free(file);
        file = astrcpy(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser)));
        xml_write_file(file);
    }
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

gboolean on_tag_add(GtkWidget *widget, gpointer data)
{
    GtkListStore *store;
    GtkTreeIter iter;
    GtkEntry *entry;
    const gchar *tag_text;
    entry = GTK_ENTRY(data);
    tag_text = gtk_entry_get_text(entry);
    if (strlen(tag_text) > 0) {
        store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tag_tree))); 
        gtk_list_store_append(store, &iter);
        // tag_active = tag_counter;
        gtk_list_store_set(store, &iter, 
                TAG_NAME, tag_text, 
                TAG_MEMO, "", 
                TAG_ID, tag_counter++, 
                -1);  
        gtk_entry_set_text(entry, "");
        
    }
    return FALSE;
}
