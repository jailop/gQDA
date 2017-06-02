#include <stdlib.h>
#include <gtk/gtk.h>
#include <poppler/glib/poppler.h>

GtkWidget *canvas = NULL;
guint max_pages = 1;
guint cur_page  = 0;
PopplerDocument *doc;
PopplerPage *page;

void slide_show()
{
    page = poppler_document_get_page(doc, cur_page);
    gtk_widget_queue_draw_area(canvas, 0, 0, 
                               gtk_widget_get_allocated_width(canvas),
                               gtk_widget_get_allocated_height(canvas));
    printf("Page %d/%d\n", cur_page, max_pages);
    fflush(stdout);
}

gboolean keypress_callback(GtkWidget *widget, GdkEventKey *ev, 
                           gpointer data)
{
    switch (ev->keyval) {
        case GDK_KEY_Escape:
            gtk_main_quit();
        case GDK_KEY_Right:
            cur_page++;
            cur_page %= max_pages;
            slide_show();
            break;
        case GDK_KEY_Left:
            cur_page--;
            cur_page %= max_pages;
            slide_show();
            break;
    }
    return FALSE;
}

gboolean back_callback(GtkWidget *widget, gpointer data)
{
    cur_page--;
    cur_page %= max_pages;
    slide_show();
    return FALSE;
}

gboolean forward_callback(GtkWidget *widget, gpointer data)
{
    cur_page++;
    cur_page %= max_pages;
    slide_show();
    return FALSE;
}

gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    guint width, height;
    gdouble popwidth, popheight, scale_factor;
    
    width = gtk_widget_get_allocated_width(canvas);
    height = gtk_widget_get_allocated_height(canvas);
    
    poppler_page_get_size(page, &popwidth, &popheight);
    
    scale_factor = width / popwidth;

    cairo_scale(cr, scale_factor, scale_factor);
    poppler_page_render(page, cr);
    return FALSE;
}

gboolean quit_callback(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
    return FALSE;
}

int main(int argc, char **argv)
{
    GtkWidget *window = NULL;
    gchar *uri = NULL;

    gtk_init(&argc, &argv);
    GtkBuilder *builder = gtk_builder_new_from_file("pdfview.glade");
    window = GTK_WIDGET(gtk_builder_get_object(builder, "Window"));
    canvas = GTK_WIDGET(gtk_builder_get_object(builder, "DrawingArea"));
    
    uri = g_strconcat("file://", argv[1], NULL);
    
    doc = poppler_document_new_from_file(uri, NULL, NULL);
    max_pages = poppler_document_get_n_pages(doc);
    page = poppler_document_get_page(doc, cur_page);

    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(G_OBJECT(builder));
    
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
