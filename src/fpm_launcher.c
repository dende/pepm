/* FIGARO'S PASSWORD MANAGER (FPM)
 * Copyright (C) 2000 John Conneely
 * Copyright (C) 2009 Aleš Koval
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
 * fpm_launcher.c formerly fpm_pref.c
 */

#include <gtk/gtk.h>

#include "fpm.h"
#include "fpm_gpw.h"
#include "fpm_crypt.h"
#include "callbacks.h"
#include "interface.h"
#include "support.h"

fpm_launcher* current_launcher;

void fpm_launcher_populate_combo_box(gint launcher_idx);
void fpm_launcher_show(fpm_launcher *launcher);
void fpm_launcher_delete_from_items(fpm_launcher *launcher);

void fpm_launcher_init() {

  gtk_widget_set_sensitive(gui->main_window, FALSE);
  glb_win_misc = create_dialog_launcher();
  gtk_window_set_transient_for(GTK_WINDOW(glb_win_misc), GTK_WINDOW(gui->main_window));

  fpm_launcher_populate_combo_box(0);

  gtk_widget_show(glb_win_misc);

}

void fpm_launcher_populate_combo_box(gint launcher_idx) {
  fpm_launcher *launcher;
  GtkComboBox *combo_box;
  GtkTreeModel *combo_box_model;
  GList* list;

  combo_box_model = GTK_TREE_MODEL(gtk_list_store_new (1, G_TYPE_STRING));

  combo_box = GTK_COMBO_BOX(lookup_widget(glb_win_misc, "combo_entry_launcher"));
  gtk_combo_box_set_model(combo_box, combo_box_model);

  list = g_list_first(glb_launcher_list);
  if(list == NULL) {
    gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "btn_launcher_remove"), FALSE);
    gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "btn_launcher_apply"), FALSE);
    gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "combo_entry_launcher"), FALSE);
    gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "entry_launcher_cmd"), FALSE); 
    gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "hbox_radio_user"), FALSE); 
    gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "hbox_radio_pass"), FALSE); 
  } else { 
    current_launcher = list->data;
  }
  while(list != NULL)
  {
    launcher = list->data;

    gtk_combo_box_append_text (combo_box, launcher->title);

    list=g_list_next(list);
  }

  if(launcher_idx < 0) 
    launcher_idx = 0;
  gtk_combo_box_set_active(combo_box, launcher_idx);

}



void fpm_launcher_add() {
  fpm_launcher *launcher;

  launcher = g_malloc0(sizeof(fpm_launcher));
  glb_launcher_list = g_list_append(glb_launcher_list, launcher);
//  fpm_prefs_load_for_edit();
//  fpm_prefs_get_launcher();
  launcher->title = g_strdup("");
  launcher->cmdline = g_strdup("");
//    fpm_create_launcher_string_list();
  fpm_launcher_populate_combo_box(g_list_index(glb_launcher_list, launcher));
  fpm_launcher_show(launcher);
//  gtk_editable_set_editable(GTK_EDITABLE((gtk_bin_get_child(GTK_BIN(lookup_widget(glb_win_misc, "combo_entry_launcher"))))), TRUE);
  gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "btn_launcher_add"), FALSE);
  gtk_widget_grab_focus(lookup_widget(glb_win_misc, "combo_entry_launcher"));
	gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "btn_launcher_remove"), TRUE);
	gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "btn_launcher_apply"), TRUE);
	gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "combo_entry_launcher"), TRUE);
	gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "entry_launcher_cmd"), TRUE); 
	gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "hbox_radio_user"), TRUE); 
	gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "hbox_radio_pass"), TRUE); 

//  state->dirty = TRUE;
  fpm_dirty(TRUE);
}

void fpm_launcher_remove(fpm_launcher *launcher) {
    gint launcher_idx;

    launcher_idx = g_list_index(glb_launcher_list, launcher);
    glb_launcher_list = g_list_remove(glb_launcher_list, launcher);
    fpm_launcher_delete_from_items(launcher);
    g_free(launcher);
    fpm_create_launcher_string_list();

    /* Launcher list is empty */
    if(!g_list_length(glb_launcher_list)) {
        gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(lookup_widget(glb_win_misc, "combo_entry_launcher")))), "");
	gtk_entry_set_text(GTK_ENTRY(lookup_widget(glb_win_misc, "entry_launcher_cmd")), "");
	gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "btn_launcher_remove"), FALSE);
	gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "btn_launcher_apply"), FALSE);
	gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "combo_entry_launcher"), FALSE);
	gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "entry_launcher_cmd"), FALSE); 
	gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "hbox_radio_user"), FALSE); 
	gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "hbox_radio_pass"), FALSE); 
	current_launcher = NULL;
    } else {
	fpm_launcher_populate_combo_box(launcher_idx - 1);
    }
//  state->dirty = TRUE;
  fpm_dirty(TRUE);
}

/* After remove launcher, delete it from all password items */
void fpm_launcher_delete_from_items(fpm_launcher *launcher) {
    GList *list;
    fpm_data *data;

    list = g_list_first(glb_pass_list);

    while (list) {
	data = list->data;
	if(!strcmp(data->launcher, launcher->title) && strcmp(launcher->title,""))
	    data->launcher ="";

	list = g_list_next(list);
    }

}

/* Show launcher details in dialog */
void fpm_launcher_show(fpm_launcher *launcher) {

  GtkWidget* widget;

    widget=lookup_widget(glb_win_misc, "entry_launcher_cmd");
    gtk_entry_set_text(GTK_ENTRY(widget), launcher->cmdline);

    if (launcher->copy_user==2) widget=lookup_widget(glb_win_misc, "radiobutton_user_clip");
    else
    {
      if (launcher->copy_user==1) widget=lookup_widget(glb_win_misc, "radiobutton_user_pri");
      else widget=lookup_widget(glb_win_misc, "radiobutton_user_none");
    }

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);

    if (launcher->copy_password==2) widget=lookup_widget(glb_win_misc, "radiobutton_pass_clip");
    else
    {
      if (launcher->copy_password==1) widget=lookup_widget(glb_win_misc, "radiobutton_pass_pri");
      else widget=lookup_widget(glb_win_misc, "radiobutton_pass_none");
    }

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);

    current_launcher = launcher;
}

gboolean fpm_launcher_search(gchar *title) {
    GList *list;
    fpm_launcher *data;

    list = g_list_first(glb_launcher_list);
    while (list) {
	data = list->data;
	if(!strcmp(title, data->title))
	    return(TRUE);
	list = g_list_next(list);
    }
    return(FALSE);
}

void fpm_launcher_remove_all() {
  GList *list;

  list = g_list_first(glb_launcher_list);
  while(list != NULL) {
    g_free(list->data);
    list = g_list_next(list);
  }
  g_list_free(glb_launcher_list);
  glb_launcher_list = NULL;
}
