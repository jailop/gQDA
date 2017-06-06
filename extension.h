#ifndef _EXTENSION_H
#define _EXTENSION_H 1

#include <gtk/gtk.h>

gboolean tree_activate_first_row(GtkTreeView *tree_view);
/* Moves cursor to and activates the first row in
 * GtkTreeView widget.
 *
 * @param tree_view: The GtkTreeView widget when the first row
 *                   will be activated.
 * @return         : TRUE in success
 *                   FALSE otherwise
 */

gboolean tree_activate_row(GtkTreeView *tree_view, GtkTreeIter *iter);
/* Activate in the GtkTreeView the row pointed by an iter
 *
 * @param tree_view : The pointer to GtkTreeView structure
 * @param iter      : Iterator of the row to activate
 * @return          : TRUE activation successful
 *                  : FALSE otherwise
 */

#endif /* _EXTENSION_H */
