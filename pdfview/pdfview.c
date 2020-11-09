#include <stdlib.h>
#include <gtk/gtk.h>
#include <poppler/glib/poppler.h>

GtkDrawingArea *canvas = NULL;
GtkWidget *scroll = NULL;
GtkAdjustment *vadjust = NULL;
PopplerDocument *doc = NULL;
PopplerPage *page = NULL;
PopplerRectangle selection;
guint max_pages = 1;
guint cur_page  = 0;
gdouble scale_factor = 0.0;
guint width, height;
gdouble popwidth, popheight;
cairo_t *drawer;

void slide_show()
{
    page = poppler_document_get_page(doc, cur_page);
    // Setting the scroll on the top
    vadjust = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scroll));
    gtk_adjustment_set_value(vadjust, 0.0);
    gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(scroll), vadjust);
    // Request to update the page drawing
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

gboolean on_button_press(GtkWidget* widget, GdkEventButton *event, 
    GdkWindowEdge edge)
{
    selection.x1 = event->x / scale_factor;
    selection.y1 = event->y / scale_factor;
    return FALSE;
}

PopplerQuadrilateral rect2quad(PopplerRectangle r)
{
    PopplerQuadrilateral quad = {
        {r.x1, popheight - r.y1},
        {r.x2, popheight - r.y1},
        {r.x1, popheight - r.y2},
        {r.x2, popheight - r.y2}
    };
    return quad;
}

gboolean on_button_release(GtkWidget* widget, GdkEventButton *event, 
    GdkWindowEdge edge)
{
    PopplerColor color = {1.0, 1.0, 0.0};
    selection.x2 = event->x / scale_factor;
    selection.y2 = event->y / scale_factor;
    // char *text = poppler_page_get_selected_text(page, 
    //    POPPLER_SELECTION_WORD, &selection);
    char *text = poppler_page_get_text_for_area(page, &selection);
    // poppler_page_render_selection(page, drawer, &selection,
    //    &selection, POPPLER_SELECTION_WORD, &fg, &hg);    
    PopplerQuadrilateral quad = rect2quad(selection);
    GArray *array = g_array_new(FALSE, FALSE, sizeof(quad));
    g_array_append_val(array, quad);
    PopplerAnnot *annot = poppler_annot_text_markup_new_highlight(doc,
        &selection, array);
    poppler_annot_set_color(annot, &color);
    poppler_annot_markup_set_opacity((PopplerAnnotMarkup *) annot, 0.3);
    
    poppler_page_add_annot(page, annot);
    printf("%s\n", text);
    /*
    PopplerRectangle **rect = NULL;
    guint n_rect;
    poppler_page_get_text_layout_for_area(page, &selection, rect, &n_rect);
    printf("%d\n", n_rect);
    for (int i = 0; i < n_rect - 1; i++) {
        PopplerRectangle *aux = *rect + i;
        printf("%f %f %f %f\n", aux->x1, aux->x2, aux->y1, aux->y2);
    }
    */
    slide_show();
    return FALSE;
}



gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    drawer = cr;
    width = gtk_widget_get_allocated_width(GTK_WIDGET(canvas));
    poppler_page_get_size(page, &popwidth, &popheight);
    scale_factor = width / popwidth;
    height = popheight * scale_factor;
    gtk_widget_set_size_request(GTK_WIDGET(canvas), width, height);
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
    GError *error;
    gtk_init(&argc, &argv);
    if (argc > 2) {
        cur_page = atoi(argv[2]);
    }   
    GtkWindow * window = (GtkWindow*) gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(window, 800, 600);
    gtk_window_set_title(window, "PDF Viewer");
    // g_signal_connect(window, "destroy", quit_callback, NULL);
    g_signal_connect(window, "destroy", gtk_main_quit, NULL);
    
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(window), scroll);
    
    canvas = (GtkDrawingArea*) gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(canvas));
    g_signal_connect((GtkWidget*) window, "key-press-event",            
        G_CALLBACK(keypress_callback), NULL);
    g_signal_connect(GTK_WIDGET(canvas), "draw", 
        G_CALLBACK(draw_callback), NULL);
    
    gtk_widget_add_events(GTK_WIDGET(canvas), 
        GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
    g_signal_connect(GTK_WIDGET(canvas), "button-press-event", 
        G_CALLBACK(on_button_press), NULL);
    g_signal_connect(GTK_WIDGET(canvas), "button-release-event",
        G_CALLBACK(on_button_release), NULL);
    
    gchar *path = g_canonicalize_filename(argv[1], NULL);
    gchar *uri = g_filename_to_uri(path, NULL, NULL);
    doc = poppler_document_new_from_file(uri, NULL, &error);
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
