#include <stdlib.h>
#include <gtk/gtk.h>
#include <poppler/glib/poppler.h>

GtkDrawingArea *canvas = NULL;
guint max_pages = 1;
guint cur_page  = 0;
PopplerDocument *doc;
PopplerPage *page;

void slide_show()
{
    page = poppler_document_get_page(doc, cur_page);
    gtk_widget_queue_draw_area(GTK_WIDGET(canvas), 0, 0, 
        gtk_widget_get_allocated_width(GTK_WIDGET(canvas)),
        gtk_widget_get_allocated_height(GTK_WIDGET(canvas)));
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
    guint width;
    gdouble popwidth, popheight, scale_factor;
    width = gtk_widget_get_allocated_width(GTK_WIDGET(canvas));
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
    GtkWindow *window;
    gchar *uri = NULL;
    GError *error;
    gtk_init(&argc, &argv);
    window = (GtkWindow*) gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(window, 800, 600);
    gtk_window_set_title(window, "PDF Viewer");
    // g_signal_connect(window, "destroy", quit_callback, NULL);
    g_signal_connect(window, "destroy", gtk_main_quit, NULL);
    
    canvas = (GtkDrawingArea*) gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(canvas));
    g_signal_connect((GtkWidget*) window, "key-press-event",            
        G_CALLBACK(keypress_callback), NULL);
    g_signal_connect(GTK_WIDGET(canvas), "draw", G_CALLBACK(draw_callback), NULL);
    
    // uri = g_strconcat("file://", argv[1], NULL);
    
    doc = poppler_document_new_from_file(argv[1], NULL, &error);
    if (!doc) {
        fprintf(stderr, "Document could not be openned\n%s\n", error->message);
        exit(EXIT_FAILURE);
    }
    max_pages = poppler_document_get_n_pages(doc);
    page = poppler_document_get_page(doc, cur_page);
    
    gtk_widget_show_all(GTK_WIDGET(window));
    gtk_main();
    return 0;
}
