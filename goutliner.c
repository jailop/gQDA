#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <webkit/webkit.h>

enum {
    LABEL = 0,
    CONTENT,
    COUNT
};

GtkWidget *tree;

void on_window_destroy(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

void on_file_new(GtkWidget *widget, gpointer data)
{
    g_print("On File New\n");
}

void on_file_open(GtkWidget *widget, gpointer data)
{
    g_print("On File Open\n");
}

void on_file_save(GtkWidget *widget, gpointer data)
{
    g_print("On File Save\n");
}

void on_note_add_sibling(GtkWidget *widget, gpointer data)
{
    gboolean flag;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter, parent;
    GtkTreePath *path;
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    if (!selection) {
        gtk_tree_store_append(GTK_TREE_STORE(model), &iter, NULL);
        return;
    }
    flag = gtk_tree_selection_get_selected(selection, &model, &iter);
    if (!flag) return;
    path = gtk_tree_model_get_path(model, &iter); 
    if (!path) return;
    flag = gtk_tree_path_up(path);
    if (flag) {
        gtk_tree_model_get_iter(model, &parent, path); 
        gtk_tree_store_append(GTK_TREE_STORE(model), &iter, &parent);
    }
    else
        gtk_tree_store_append(GTK_TREE_STORE(model), &iter, NULL);
}

void on_note_add_child(GtkWidget *widget, gpointer data)
{
}

void on_note_remove(GtkWidget *widget, gpointer data)
{
    g_print("On File Save\n");
}

GtkWidget *prepare_menubar()
{
    GtkWidget *menu_bar;
    GtkWidget *menu_set;
    GtkWidget *menu_item;

    menu_bar = gtk_menu_bar_new();
    menu_item = gtk_menu_item_new_with_mnemonic("_File");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
    menu_set = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu_set);

    menu_item = gtk_menu_item_new_with_mnemonic("_New");
    g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(on_file_new), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_set), menu_item);
    
    return menu_bar;
}

GtkWidget *prepare_toolbar()
{
    GtkWidget *toolbar;
    GtkToolItem *toolbutton;
    toolbar = gtk_toolbar_new();

    toolbutton = gtk_tool_button_new(
        gtk_image_new_from_icon_name(GTK_STOCK_NEW, GTK_ICON_SIZE_SMALL_TOOLBAR), 
        "New");
    g_signal_connect(G_OBJECT(toolbutton), "clicked", G_CALLBACK(on_file_new), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolbutton, -1);

    toolbutton = gtk_tool_button_new(
        gtk_image_new_from_icon_name(GTK_STOCK_OPEN, GTK_ICON_SIZE_SMALL_TOOLBAR), 
        "Open");
    g_signal_connect(G_OBJECT(toolbutton), "clicked", G_CALLBACK(on_file_open), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolbutton, -1);
    
    toolbutton = gtk_tool_button_new(
        gtk_image_new_from_icon_name(GTK_STOCK_SAVE, GTK_ICON_SIZE_SMALL_TOOLBAR), 
        "Save");
    g_signal_connect(G_OBJECT(toolbutton), "clicked", G_CALLBACK(on_file_save), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolbutton, -1);

    toolbutton = gtk_separator_tool_item_new();
    gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolbutton), TRUE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolbutton, -1);
    
    toolbutton = gtk_tool_button_new(
        gtk_image_new_from_icon_name(GTK_STOCK_ADD, GTK_ICON_SIZE_SMALL_TOOLBAR), 
        "Sibling");
    g_signal_connect(G_OBJECT(toolbutton), "clicked", G_CALLBACK(on_note_add_sibling), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolbutton, -1);

    toolbutton = gtk_tool_button_new(
        gtk_image_new_from_icon_name(GTK_STOCK_INDENT, GTK_ICON_SIZE_SMALL_TOOLBAR), 
        "Child");
    g_signal_connect(G_OBJECT(toolbutton), "clicked", G_CALLBACK(on_note_add_child), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolbutton, -1);

    toolbutton = gtk_tool_button_new(
        gtk_image_new_from_icon_name(GTK_STOCK_REMOVE, GTK_ICON_SIZE_SMALL_TOOLBAR), 
        "Remove");
    g_signal_connect(G_OBJECT(toolbutton), "clicked", G_CALLBACK(on_note_remove), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolbutton, -1);

    return toolbar;
}

GtkWidget *prepare_tree()
{
    GtkWidget *tree;
    GtkTreeStore *store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
    renderer = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer), "editable", TRUE, NULL);
    column = gtk_tree_view_column_new_with_attributes("Notes", renderer, "text", LABEL, NULL);
    tree = gtk_tree_view_new();
    gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(store));
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
    gtk_tree_view_set_enable_tree_lines(GTK_TREE_VIEW(tree), TRUE);
    return tree;
}

int main(int argc, char **argv)
{
    GtkWidget *window;
    GtkWidget *windowbox, *bodybox;
    GtkWidget *menubar;
    GtkWidget *toolbar;
    GtkWidget *notebook;
    GtkWidget *label;
    GtkWidget *editor;
    GtkWidget *viewer;
    GtkWidget *statusbar;
    GtkWidget *scrolledwindow;
    gtk_init(&argc, &argv);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_window_set_title(GTK_WINDOW(window), "gOutliner");
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(on_window_destroy), NULL);

    menubar = prepare_menubar();
    toolbar = prepare_toolbar();

    tree = prepare_tree();
    notebook = gtk_notebook_new();
    
    editor = gtk_source_view_new();
    label = gtk_label_new("Edit");
    scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolledwindow), editor);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrolledwindow, label);

    viewer = webkit_web_view_new();
    label = gtk_label_new("View");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), viewer, label);

    bodybox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolledwindow), tree);
    gtk_box_pack_start(GTK_BOX(bodybox), scrolledwindow, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(bodybox), notebook, TRUE, TRUE, 0);

    statusbar = gtk_statusbar_new();

    windowbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(windowbox), menubar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(windowbox), toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(windowbox), bodybox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(windowbox), statusbar, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), windowbox);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
