/*
 * Copyright © 2004 Noah Levitt
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02110-1301  USA
 */

#include <config.h>
#include <string.h>
#include <gtk/gtk.h>

#include "mcharmap.h"
#include "mcharmap-private.h"

#include "unicode-blocks.h"

struct _McharmapChaptersViewPrivate {
  GtkTreeViewColumn *column;
  McharmapChaptersModel *model;
};

static void
select_iter (McharmapChaptersView *view,
             GtkTreeIter *iter)
{
  GtkTreeView *tree_view = GTK_TREE_VIEW (view);
  GtkTreeSelection *selection;
  GtkTreePath *path;

  selection = gtk_tree_view_get_selection (tree_view);
  gtk_tree_selection_select_iter (selection, iter);

  path = gtk_tree_model_get_path (gtk_tree_view_get_model (tree_view), iter);
  gtk_tree_view_set_cursor (tree_view, path, NULL, FALSE);
  gtk_tree_view_scroll_to_cell (tree_view, path, NULL, FALSE, 0.5, 0);
  gtk_tree_path_free (path);
}

static void
mcharmap_chapters_view_init (McharmapChaptersView *view)
{
  McharmapChaptersViewPrivate *priv;
  GtkTreeView *tree_view = GTK_TREE_VIEW (view);
  GtkCellRenderer *cell;
  GtkTreeViewColumn *column;
  GtkTreeSelection *selection;

  priv = view->priv = G_TYPE_INSTANCE_GET_PRIVATE (view, MCHARMAP_TYPE_CHAPTERS_VIEW, McharmapChaptersViewPrivate);

  cell = gtk_cell_renderer_text_new ();
  column = priv->column = gtk_tree_view_column_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_add_attribute (column, cell, "text", MCHARMAP_CHAPTERS_MODEL_COLUMN_LABEL);
  gtk_tree_view_column_set_sort_column_id (column, MCHARMAP_CHAPTERS_MODEL_COLUMN_LABEL);
  gtk_tree_view_append_column (tree_view, column);

  selection = gtk_tree_view_get_selection (tree_view);
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);

  gtk_tree_view_set_enable_search (tree_view, TRUE);
}

static void
mcharmap_chapters_view_class_init (McharmapChaptersViewClass *klass)
{
  g_type_class_add_private (klass, sizeof (McharmapChaptersViewPrivate));
}

G_DEFINE_TYPE (McharmapChaptersView, mcharmap_chapters_view, GTK_TYPE_TREE_VIEW)

GtkWidget * 
mcharmap_chapters_view_new (void)
{
  return g_object_new (mcharmap_chapters_view_get_type (), NULL);
}

McharmapChaptersModel *
mcharmap_chapters_view_get_model (McharmapChaptersView *view)
{
  McharmapChaptersViewPrivate *priv = view->priv;

  return priv->model;
}

void
mcharmap_chapters_view_set_model (McharmapChaptersView *view,
                                   McharmapChaptersModel *model)
{
  McharmapChaptersViewPrivate *priv = view->priv;
  GtkTreeView *tree_view = GTK_TREE_VIEW (view);

  priv->model = model;
  gtk_tree_view_set_model (tree_view, GTK_TREE_MODEL (model));
  if (!model)
    return;

  gtk_tree_view_column_set_title (priv->column, mcharmap_chapters_model_get_title (model));
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model),
                                        MCHARMAP_CHAPTERS_MODEL_COLUMN_LABEL,
                                        GTK_SORT_ASCENDING);

  /* Need to re-set this here since it's set to -1 when the tree view model changes! */
  gtk_tree_view_set_search_column (tree_view, MCHARMAP_CHAPTERS_MODEL_COLUMN_LABEL);
}

/**
 * mcharmap_view_view_next:
 * @view: a #McharmapChapters
 *
 * Moves to the next chapter if applicable.
 **/
void
mcharmap_chapters_view_next (McharmapChaptersView *view)
{
  GtkTreeView *tree_view = GTK_TREE_VIEW (view);
  GtkTreeSelection *selection = gtk_tree_view_get_selection (tree_view);
  GtkTreeModel *model;
  GtkTreePath *path;
  GtkTreeIter iter;

  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    return;
  if (!gtk_tree_model_iter_next (model, &iter))
    return;
   
  path = gtk_tree_model_get_path (model, &iter);
  gtk_tree_view_set_cursor (tree_view, path, NULL, FALSE);
  gtk_tree_path_free (path);
}

/**
 * mcharmap_chapters_view_previous:
 * @view: a #McharmapChapters
 *
 * Moves to the previous chapter if applicable.
 **/
void
mcharmap_chapters_view_previous (McharmapChaptersView *view)
{
  GtkTreeView *tree_view = GTK_TREE_VIEW (view);
  GtkTreeSelection *selection = gtk_tree_view_get_selection (tree_view);
  GtkTreeModel *model;
  GtkTreePath *path;
  GtkTreeIter iter;

  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    return;

  path = gtk_tree_model_get_path (model, &iter);
  if (gtk_tree_path_prev (path))
    gtk_tree_view_set_cursor (tree_view, path, NULL, FALSE);
  gtk_tree_path_free (path);
}


/**
 * mcharmap_chapter_view_get_selected:
 * @view: a #McharmapChapters
 *
 * Returns a newly allocated string containing
 * the name of the currently selected chapter
 **/
gchar*
mcharmap_chapters_view_get_selected (McharmapChaptersView *view)
{
  GtkTreeView *tree_view = GTK_TREE_VIEW (view);
  GtkTreeSelection *selection = gtk_tree_view_get_selection (tree_view);
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *name = NULL;

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    gtk_tree_model_get(model, &iter, MCHARMAP_CHAPTERS_MODEL_COLUMN_ID, &name, -1);

  return name;
}

/**
 * mcharmap_chapter_view_set_selected:
 * @view: a #McharmapChapters
 * @name: 
 *
 * Sets the selection to the row specified by @name
 * Return value: %TRUE on success, %FALSE on failure
 **/
gboolean
mcharmap_chapters_view_set_selected (McharmapChaptersView *view,
                                      const char        *name)
{
  McharmapChaptersViewPrivate *priv = view->priv;
  GtkTreeIter iter;

  if (!mcharmap_chapters_model_id_to_iter (priv->model, name, &iter))
    return FALSE;

  select_iter (view, &iter);
  return TRUE;
}

/**
 * mcharmap_chapters_view_select_character:
 * @view: a #McharmapChaptersView
 * @wc: a character
 *
 * Return value: %TRUE on success, %FALSE on failure.
 **/
gboolean
mcharmap_chapters_view_select_character (McharmapChaptersView *view,
                                          gunichar           wc)
{
  McharmapChaptersViewPrivate *priv = view->priv;
  GtkTreeIter iter;

  g_return_val_if_fail (MCHARMAP_IS_CHAPTERS_VIEW (view), FALSE);

  if (wc > UNICHAR_MAX)
    return FALSE;

  if (!mcharmap_chapters_model_character_to_iter (priv->model, wc, &iter))
    return FALSE;

  select_iter (view, &iter);
  return TRUE;
}

/**
 * mcharmap_chapters_view_get_codepoint_list:
 * @view: a #McharmapChaptersView
 *
 * Creates a new #McharmapCodepointList representing the characters in the
 * current chapter.
 *
 * Return value: the newly-created #McharmapCodepointList, or NULL if
 * there is no chapter selected. The caller should release the result with
 * g_object_unref() when finished.
 **/
McharmapCodepointList * 
mcharmap_chapters_view_get_codepoint_list (McharmapChaptersView *view)
{
  McharmapChaptersViewPrivate *priv = view->priv;
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  
  g_return_val_if_fail (MCHARMAP_IS_CHAPTERS_VIEW (view), NULL);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
  if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
    return NULL;

  return mcharmap_chapters_model_get_codepoint_list (priv->model, &iter);
}

/**
 * mcharmap_chapters_view_get_codepoint_list:
 * @view: a #McharmapChaptersView
 *
 * Return value: a reference to a #McharmapCodepointList representing all the characters
 * in all the chapters. It should not be modified, but must be g_object_unref()'d after use.
 **/
McharmapCodepointList *
mcharmap_chapters_view_get_book_codepoint_list (McharmapChaptersView *view)
{
  McharmapChaptersViewPrivate *priv = view->priv;

  g_return_val_if_fail (MCHARMAP_IS_CHAPTERS_VIEW (view), NULL);

  return mcharmap_chapters_model_get_book_codepoint_list (priv->model);
}

gboolean
mcharmap_chapters_view_select_locale (McharmapChaptersView *view)
{
  return mcharmap_chapters_view_select_character (view,
                                                   mcharmap_unicode_get_locale_character ());
}
