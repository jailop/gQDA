#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>

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

struct note_t {
    char *label;
    char *content;
    char *id;
    char *pos;
};

unsigned int notes_counter = 0;
char *file = NULL;

char *astrcpy(const char *s)
{
    char *ret;
    int len = strlen(s) + 1;
    ret = malloc(sizeof(char) * len);
    strcpy(ret, s);
    return ret;
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
    return 1;
}

int xml_open_file(const char *filename)
{
#ifdef DEBUG
    printf("Function: xml_read_file\n");
#endif
    char *tagname;
    int ret;
    GtkListStore *store;
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(note_tree))); 
    gtk_list_store_clear(store);
    xmlTextReaderPtr reader = xmlReaderForFile(filename, NULL, 0);
    ret = xmlTextReaderRead(reader);
    while (ret == 1) {
        tagname = (char *) xmlTextReaderConstName(reader);
        if (strcmp(tagname, "note") == 0 && xmlTextReaderNodeType(reader) == 1)
            xml_read_note(reader, store);
        else
            printf("Tag: %s\n", tagname);
        ret = xmlTextReaderRead(reader);
    }
    xmlFreeTextReader(reader);
    return 1;
}

gboolean xml_write_each_note(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
#ifdef DEBUG
    printf("Function: xml_write_each_note\n");
#endif
    xmlTextWriterPtr writer = (xmlTextWriterPtr) data;
    gchar *name;
    gchar *content;
    int id;
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
#ifdef DEBUG
    printf("Function: xml_write_notes\n");
#endif
    GtkTreeModel *store = gtk_tree_view_get_model(GTK_TREE_VIEW(note_tree)); 
    xmlTextWriterStartElement(writer, BAD_CAST "notes");
    gtk_tree_model_foreach(store, xml_write_each_note, writer); 
    xmlTextWriterEndElement(writer);
    return 1;
}

int xml_write_file(const char *filename)
{
#ifdef DEBUG
    printf("Function: write_file %s\n", filename);
#endif
    xmlTextWriterPtr writer;

    writer = xmlNewTextWriterFilename(filename, 0);
    if (writer == NULL) {
        printf("xmlWriter failure: error creating writer\n");
        return 0;
    }

    xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);
    xmlTextWriterStartElement(writer, BAD_CAST "data");
    xml_write_notes(writer);
    xmlTextWriterEndElement(writer);
    xmlTextWriterEndDocument(writer);
    xmlFreeTextWriter(writer);
    return 1;
}
int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);
    GtkBuilder *builder = gtk_builder_new_from_file("gqda.glade");
    
    // Loading global objects
    window = GTK_WIDGET(gtk_builder_get_object(builder, "Window"));
    note_tree = GTK_WIDGET(gtk_builder_get_object(builder, "NoteTree"));
    note_view = GTK_WIDGET(gtk_builder_get_object(builder, "NoteView"));
    tag_tree = GTK_WIDGET(gtk_builder_get_object(builder, "TagTree"));

    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(G_OBJECT(builder));
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}

gboolean on_open_note(GtkWidget *widget, gpointer userdata)
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
        gtk_list_store_set(note_store, &iter, 
                NOTE_NAME, filename, 
                NOTE_CONTENT, s->str, 
                NOTE_ID, notes_counter++, 
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

gboolean on_note_tree_row_activated(GtkTreeView *tree, GtkTreePath *path, 
        GtkTreeViewColumn *column, gpointer userdata)
{
    GtkTreeModel *note_store =
        gtk_tree_view_get_model(GTK_TREE_VIEW(note_tree)); 
    GtkTreeIter iter;
    gtk_tree_model_get_iter(note_store, &iter, path);
    
    gchar *name;
    gchar *content;
    guint id;
    gtk_tree_model_get(note_store, &iter,
            NOTE_NAME, &name,
            NOTE_CONTENT, &content,
            NOTE_ID, &id,
            -1);

    GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note_view));
    gtk_text_buffer_set_text(tb, content, strlen(content));
    free(name);
    free(content);
    return FALSE;
}

gboolean on_project_new(GtkWidget *widget, gpointer data)
{
    return FALSE;
}

gboolean on_project_open(GtkWidget *widget, gpointer data)
{
    xml_open_file("test.gqda");
    return FALSE;
}

gboolean on_project_save(GtkWidget *widget, gpointer data)
{
    xml_write_file("test.gqda");
    return FALSE;
}
