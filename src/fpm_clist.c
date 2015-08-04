/* FIGARO'S PASSWORD MANAGER 2 (FPM2)
 * Copyright (C) 2000 John Conneely
 * Copyright (C) 2008-2010 Aleš Koval
 *
 * FPM is open source / free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * FPM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * fpm_clist.c
 */

#include "fpm.h"
#include "fpm_crypt.h"
#include "support.h"
#include <string.h>

static gint
fpm_cmp_title(fpm_data *a, fpm_data *b)
{
  return(g_utf8_collate(a->title, b->title));
}

void
fpm_create_category_list(gint edit_flag)
{
  GList *list;
  fpm_data *data;
//  GCompareFunc cmp_func;


  /* Clear cat list */
  list = g_list_first(glb_cat_string_list);
  while(list!=NULL)
  {
    g_assert(list->data!=NULL);
    g_free(list->data);
    list=g_list_next(list);
  }
  g_list_free(glb_cat_string_list);
  glb_cat_string_list = NULL;

//  cmp_func = (GCompareFunc) g_strcasecmp;
//  cmp_func = (GCompareFunc) g_utf8_collate;

  /* Step through pass list adding to cat list */
  list = g_list_first(glb_pass_list);
  while(list!=NULL)
  {
    data = list->data;
    if (
	strcmp(data->category, "") &&
	    g_list_find_custom(glb_cat_string_list, data->category, (GCompareFunc) g_utf8_collate) == NULL)
    {
      g_assert(data->category != NULL);
      glb_cat_string_list = g_list_insert_sorted(glb_cat_string_list,
		g_strdup(data->category), (GCompareFunc) g_utf8_collate);
    }
    list=g_list_next(list);
  }



  if(edit_flag)
  {
    /* Add first item entry to beginning of list */
    glb_cat_string_list = g_list_prepend(glb_cat_string_list, g_strdup(""));
  }
  else
  {
    /* Add all category and default category */
    glb_cat_string_list = g_list_prepend(glb_cat_string_list, g_strdup(FPM_NONE_CAT_MSG));
    glb_cat_string_list = g_list_prepend(glb_cat_string_list, g_strdup(FPM_DEFAULT_CAT_MSG));
    glb_cat_string_list = g_list_prepend(glb_cat_string_list, g_strdup(FPM_ALL_CAT_MSG));
  }
}

void
fpm_populate_combo_box(GtkComboBox *combo_box, gchar *active_item) {
  GList* list;
  gint i, index;
  GtkTreeModel *combo_box_model;

  fpm_create_category_list(0);
  g_assert(glb_cat_string_list!=NULL);

  combo_box_model = GTK_TREE_MODEL(gtk_list_store_new (1, G_TYPE_STRING));

  gtk_combo_box_set_model(combo_box, combo_box_model);

  list=g_list_first(glb_cat_string_list);
  i=0;
  index=0;
  while(list!=NULL)
  {
//    gtk_combo_box_append_text(combo_box, g_strdup(list->data));
    gtk_combo_box_append_text(combo_box, list->data);

    if(!strcmp(active_item, list->data))
	index=i;
    i++;
    list=g_list_next(list);
  }

  gtk_combo_box_set_active(combo_box, index);
}

void
fpm_clist_populate_cat_list()
{
	fpm_populate_combo_box(GTK_COMBO_BOX(lookup_widget(gui->main_window, "optionmenu_category")), state->category);
}

void
fpm_clist_init()
{
  GtkListStore *main_list_store;
  GtkCellRenderer *cellRenderer;
  GtkTreeViewColumn *treeViewColumn = NULL;
//  GtkTreeModel *list_store;
  GType types[FPM_NUM_COLS];
  gint i = 1, j;

  g_signal_handlers_block_by_func (gui->main_clist, G_CALLBACK (on_clist_main_columns_changed), NULL);

  GList *columns;
  columns = gtk_tree_view_get_columns(gui->main_clist);
  while (columns != NULL) {
       gtk_tree_view_remove_column(gui->main_clist, GTK_TREE_VIEW_COLUMN(columns->data));
       columns = g_list_next(columns);
   }
  g_list_free(columns);

  cellRenderer = gtk_cell_renderer_text_new();

  columns = g_list_first(columns_order);
  while(columns != NULL) {

	j = GPOINTER_TO_INT(columns->data);

	types[i] = G_TYPE_STRING;
	treeViewColumn = gtk_tree_view_column_new_with_attributes (gettext(columns_title[j]), cellRenderer, "text", i, NULL);
	g_object_set_data(G_OBJECT(treeViewColumn), "index", GINT_TO_POINTER(j));
	gtk_tree_view_append_column (GTK_TREE_VIEW(gui->main_clist), treeViewColumn);
	gtk_tree_view_column_set_resizable (treeViewColumn, TRUE);
	gtk_tree_view_column_set_fixed_width (treeViewColumn, ini->columns_width[i-1]);
	gtk_tree_view_column_set_sizing (treeViewColumn, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_reorderable (treeViewColumn, TRUE);
	gtk_tree_view_column_set_sort_column_id (treeViewColumn, i);

	i++;
	columns = g_list_next(columns);
   }

    if(treeViewColumn != NULL)
	gtk_tree_view_column_set_sizing (treeViewColumn, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    g_list_free(columns);

    types[0] = G_TYPE_POINTER;

    main_list_store = gtk_list_store_newv (i, types);
    gtk_tree_view_set_model (gui->main_clist, GTK_TREE_MODEL (main_list_store));

    g_signal_handlers_unblock_by_func (gui->main_clist, G_CALLBACK (on_clist_main_columns_changed), NULL);

}


void
fpm_clist_create_view(gchar *category)
/* Create visible CList from abstract GList */
{
//  gint row,i,j;
  fpm_data *data;
  GList *list;

  GtkTreeIter iter;
  GtkTreeModel *list_store;
//  GCompareFunc cmp_list;

  glb_num_row = 0;

  /* Sort the list */
  //glb_cmp = (GCompareFunc) fpm_cmp_title;
  glb_pass_list = g_list_sort(glb_pass_list, (GCompareFunc) fpm_cmp_title);

  list_store = gtk_tree_view_get_model (gui->main_clist);

  gtk_list_store_clear(GTK_LIST_STORE(list_store));

  /* Iterate over the list */
  list = g_list_first(glb_pass_list);
  while (list!=NULL)
  {

    /* Get data for this element */
    data = list->data;

    if(category == NULL ||
       !strcmp(category, FPM_ALL_CAT_MSG) ||
       !strcmp(category, data->category) ||
       (!strcmp(category, FPM_NONE_CAT_MSG) && !strlen(data->category)) ||
       (!strcmp(category, FPM_DEFAULT_CAT_MSG) && data->default_list) )
    {

    gtk_list_store_append (GTK_LIST_STORE(list_store), &iter);

    fpm_clist_set_data (data, list_store, iter);

    glb_num_row++;
//      row = glb_num_row - 1;

    }

    list = g_list_next(list);
  }
}

void
fpm_clist_set_data(fpm_data *data, GtkTreeModel *list_store, GtkTreeIter iter) {

  GList *columns;
  GValue value = {0};
  gchar cleartext[FPM_PASSWORD_LEN+1] = {0};
  gint i, j, ix;


  i = 1;
  columns = g_list_first(columns_order);
  while (columns != NULL) {
	g_value_init(&value, G_TYPE_STRING);

	j = GPOINTER_TO_INT(columns->data);
	switch(j) {
	    case FPM_TITLE:
		g_value_set_static_string(&value, data->title);
		break;
	    case FPM_URL:
		g_value_set_static_string(&value, data->arg);
		break;
	    case FPM_USER:
		g_value_set_static_string(&value, data->user);
		break;
	    case FPM_PASSWORD:
		ix = strlen(data->password) / 2;
		fpm_decrypt_field(old_context, cleartext, data->password, ix);
		g_value_set_string(&value, cleartext);
		wipememory(cleartext, FPM_PASSWORD_LEN);
		break;
	    case FPM_CATEGORY:
		g_value_set_static_string(&value, data->category);
		break;
	    case FPM_NOTES:
		g_value_set_static_string(&value, data->notes);
		break;
	    case FPM_LAUNCHER:
		g_value_set_static_string(&value, data->launcher);
		break;
	}

        gtk_list_store_set_value(GTK_LIST_STORE (list_store), &iter, i, &value);
	g_value_unset(&value);
	i++;
	columns = g_list_next(columns);
  }

  g_value_init(&value, G_TYPE_POINTER);
  g_value_set_pointer(&value, data);
  gtk_list_store_set_value(GTK_LIST_STORE (list_store), &iter, FPM_DATA_POINTER, &value);
  g_value_unset(&value);

  g_list_free(columns);
}

