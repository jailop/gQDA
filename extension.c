#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

char *astrcpy(const char *s)
{
    int len;
    char *ret;
    if (s == NULL) {
        fprintf(stderr, "astrcpy: A NULL string was given to copy\n");
        return NULL;
    }
    len = strlen(s) + 1; /* memory lenght to be allocated */
    ret = malloc(sizeof(char) * len);
    if (ret == NULL) {
        fprintf(stderr, "astrcpy: Memory allocation failed\n");
        return NULL;
    }
    strcpy(ret, s); /* finally, string is copied */
    return ret;
}

gboolean tree_activate_first_row(GtkTreeView *tree_view)
{
    GtkTreePath *path;
    GtkTreeViewColumn *column;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean has_items;
    if (tree_view == NULL) {
        fprintf(stderr, "tree_select_first: parameter is NULL\n");
        return FALSE;
    }
    /* Checking if the tree model has any element */
    model = gtk_tree_view_get_model(tree_view);
    has_items = gtk_tree_model_get_iter_first(model, &iter);
    if (has_items == FALSE)
        return FALSE;  /* So, it is empty */
    /* Getting path for the first row */
    path = gtk_tree_path_new_from_string("0");
    if (path == NULL) {
        fprintf(stderr, "tree_select_first: path new first getting failed\n");
        return FALSE;
    }
    /* Getting the first column */
    column = gtk_tree_view_get_column(tree_view, 0);
    if (column == NULL) {
        fprintf(stderr, "tree_select_first: getting first coloumn failed\n");
        return FALSE;
    }
    /* Moving cursor to the first row and activating that */
    gtk_tree_view_set_cursor(tree_view, path, column, TRUE);
    gtk_tree_view_row_activated(tree_view, path, column);
    /* Cleaning en return */
    gtk_tree_path_free(path);
    return TRUE;
}

gboolean tree_activate_row(GtkTreeView *tree_view, GtkTreeIter *iter)
{
    GtkTreePath *path;
    GtkTreeModel *model;
    GtkTreeViewColumn *column;


    if (tree_view == NULL) {
        fprintf(stderr, "tree_activate_row: tree_view is NULL");
        return FALSE;
    }
    if (iter == NULL) {
        fprintf(stderr, "tree_activate_row: iter is NULL");
        return FALSE;
    }

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view));
    column = gtk_tree_view_get_column(tree_view, 0);
    if (column == NULL) {
        fprintf(stderr, "tree_select_first: getting first coloumn failed\n");
        return FALSE;
    }

    path = gtk_tree_model_get_path(model, iter);
    gtk_tree_view_set_cursor(tree_view, path, column, TRUE);
    gtk_tree_view_row_activated(tree_view, path, column);

    return TRUE;
}
