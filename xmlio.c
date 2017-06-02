#include <string.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>
#include <libxml/encoding.h>
#include "base.h"
#include "extension.h"

/*
#define XML_ENCODING "ISO-8859-1"
*/
#define XML_ENCODING "UTF-8"

static int xml_read_note(xmlTextReaderPtr reader, GtkListStore *store, 
                         struct gqda_app *app)
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
    app->note_counter++;
    return 1;
}

static int xml_read_tag(xmlTextReaderPtr reader, GtkTreeStore *store, 
                        GPtrArray **iters, struct gqda_app *app)
{
    int i;
    int id;
    int parent;
    gchar *name;
    gchar *memo;
    GtkTreeIter iter; 
    GtkTreeIter *iter_parent;
    GPtrArray *iterarray = *iters;
    xmlChar *tmp;
    
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
    name = (char *) xmlTextReaderReadString(reader);
    memo = "";

    if (parent < 0) 
        gtk_tree_store_append(store, &iter, NULL);
    else { 
        iter_parent = g_ptr_array_index(iterarray, parent); 
        if (iter_parent != NULL)
            gtk_tree_store_append(store, &iter, iter_parent);
        else
            gtk_tree_store_append(store, &iter, NULL);
    }

    if (iterarray == NULL)
        iterarray = *iters = g_ptr_array_new();
    
    if (iterarray->len <= id)
        for (i = iterarray->len; i <= id; i++)
            g_ptr_array_insert(iterarray, i, NULL);
    
    g_ptr_array_insert(iterarray, id, gtk_tree_iter_copy(&iter));
    
    gtk_tree_store_set(store, &iter,
        TAG_NAME, name,
        TAG_MEMO, memo,
        TAG_ID, id,
        -1);
    
    app->tag_counter++;
    
    free(name);
    return 1;
}



int xml_open(struct gqda_app *app)
{
    int i;
    char *tagname;
    int ret;
    GtkListStore *note_store;
    GtkTreeStore *tag_store;
    GPtrArray *iterarray = NULL;
    note_store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(app->note_tree)));
    gtk_list_store_clear(note_store);
    app->note_counter = 0;
    tag_store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(app->tag_tree)));
    gtk_tree_store_clear(tag_store);
    app->tag_counter = 0;
    xmlTextReaderPtr reader = xmlReaderForFile(app->file, NULL, 0);
    ret = xmlTextReaderRead(reader);
    while (ret == 1) {
        tagname = (char *) xmlTextReaderConstName(reader);
        if (strcmp(tagname, "note") == 0 &&
            xmlTextReaderNodeType(reader) == 1) {
            xml_read_note(reader, note_store, app);
        }
        else if (strcmp(tagname, "tag") == 0 &&
                 xmlTextReaderNodeType(reader) == 1) {
            xml_read_tag(reader, tag_store, &iterarray, app);
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
            selection_add(&(app->selections), note_id, tag_id, start, end);
        }
        ret = xmlTextReaderRead(reader);
    }
    xmlFreeTextReader(reader);

    tree_activate_first_row(GTK_TREE_VIEW(app->note_tree));
    tree_activate_first_row(GTK_TREE_VIEW(app->tag_tree));
    tree_activate_first_row(GTK_TREE_VIEW(app->main_tree));

    if (iterarray) {
        for (i = 0; i < iterarray->len; i++) {
            GtkTreeIter *iter_parent;
            iter_parent = g_ptr_array_index(iterarray, i);
            if (iter_parent)
                gtk_tree_iter_free(iter_parent);
        }
        g_ptr_array_free(iterarray, FALSE);
    }
    return 1;
}

static gboolean xml_write_each_note(GtkTreeModel *model, GtkTreePath *path,
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
    xmlTextWriterWriteAttribute(writer, BAD_CAST "content", BAD_CAST "text");
    xmlTextWriterWriteElement(writer, BAD_CAST "name", BAD_CAST name);
    xmlTextWriterWriteElement(writer, BAD_CAST "content", BAD_CAST content);
    xmlTextWriterEndElement(writer);
#ifdef DEBUG
    printf("xml_write_each_node: it is finishing to write\n");
#endif
    return FALSE;
}

static int xml_write_notes(xmlTextWriterPtr writer, struct gqda_app *app)
{
#ifdef DEBUG
    printf("xml_write_notes: it is ready to start writing\n");
#endif
    xmlTextWriterStartElement(writer, BAD_CAST "notes");
    gtk_tree_model_foreach(app->note_model, xml_write_each_note, writer);
    xmlTextWriterEndElement(writer);
#ifdef DEBUG
    printf("xml_write_notes: it is finishing to write\n");
#endif
    return 1;
}

static gboolean xml_write_each_tag(GtkTreeModel *model, GtkTreePath *path, 
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

static int xml_write_tags(xmlTextWriterPtr writer, struct gqda_app *app)
{
    GtkTreeModel *store;
    store = gtk_tree_view_get_model(GTK_TREE_VIEW(app->tag_tree));
    xmlTextWriterStartElement(writer, BAD_CAST "tags");
    gtk_tree_model_foreach(store, xml_write_each_tag, writer);
    xmlTextWriterEndElement(writer);
#ifdef DEBUG
    printf("xml_write_tags: it is finishing to write\n");
#endif
    return 1;
}

static int xml_write_selections(xmlTextWriterPtr writer, struct gqda_app *app)
{
    int i, j, k;
    GPtrArray *tag, *sel;
    struct selection *s;
    if (app->selections == NULL)
        return 0;
    xmlTextWriterStartElement(writer, BAD_CAST "selections");
    for (i = 0; i < app->selections->len; i++) {
        tag = g_ptr_array_index(app->selections, i);
        if (tag == NULL)
            continue;
        for (j = 0; j < tag->len; j++) {
            sel = g_ptr_array_index(tag, j);
            if (sel == NULL)
                continue;
            for (k = 0; k < sel->len; k++) {
                s = g_ptr_array_index(sel, k);
                if (!s) continue;
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

int xml_write(struct gqda_app *app)
{
    xmlTextWriterPtr writer;

    if (app->file == NULL) {
        fprintf(stderr, "xml_write_file: filename is NULL\n");
        return 0;
    }
    writer = xmlNewTextWriterFilename(app->file, 0);
    if (writer == NULL) {
        fprintf(stderr, "xml_write_file: error creating writer\n");
        return 0;
    }
    
#ifdef DEBUG
    printf("xml_write_file: it is ready to start writing\n");
#endif
    xmlTextWriterStartDocument(writer, NULL, XML_ENCODING, NULL);
    xmlTextWriterStartElement(writer, BAD_CAST "data");
    xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "version", "%f", 1.0);
    
    if (app->note_counter > 0) 
        xml_write_notes(writer, app);
    if (app->tag_counter > 0) 
        xml_write_tags(writer, app);
    xml_write_selections(writer, app);

#ifdef DEBUG
    printf("xml_write_file: it is finishing to write\n");
#endif
    xmlTextWriterEndElement(writer);
    xmlTextWriterEndDocument(writer);
    
    xmlFreeTextWriter(writer);
    return 1;
}
