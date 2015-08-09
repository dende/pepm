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
 * callbacks.c
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdlib.h>

#include "fpm.h"
#include "fpm_gpw.h"
#include "fpm_file.h"
#include "fpm_crypt.h"
#include "callbacks.h"
#include "interface.h"
#include "support.h"


void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget *about_dialog;

	about_dialog = create_dialog_about();
	gtk_window_set_icon (GTK_WINDOW(about_dialog), gtk_widget_render_icon (about_dialog, GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU, NULL));
	gtk_window_set_transient_for (GTK_WINDOW(about_dialog), GTK_WINDOW(gui->main_window));

	gtk_widget_show (about_dialog);
}

void
on_app_safe_destroy                    (GtkObject       *object,
                                        gpointer         user_data)
{
    gtk_main_quit();
}

void
on_button_ok_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
  if (!fpm_save_passitem(gui->edit_window, glb_edit_data)) {
	state->new_entry = FALSE;
	gtk_widget_destroy(gui->edit_window);
/*	gtk_tree_view_set_cursor(gui->main_clist, gtk_tree_path_new_from_string(g_strdup_printf("%i", glb_cur_row)), NULL, FALSE);
	gtk_widget_set_sensitive(gui->main_window, TRUE);
	gtk_widget_grab_focus(GTK_WIDGET(gui->main_clist));*/
   }
}


void
on_button_cancel_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
  gtk_widget_destroy(gui->edit_window);
/*  gtk_tree_view_set_cursor(gui->main_clist, gtk_tree_path_new_from_string(g_strdup_printf("%i", glb_cur_row)), NULL, FALSE);
  gtk_widget_set_sensitive(gui->main_window, TRUE);
  gtk_widget_grab_focus(GTK_WIDGET(gui->main_clist));
*/
}


void
on_button_new_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
  fpm_new_passitem(&gui->edit_window, &glb_edit_data);
}



void
on_button_edit_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  if(fpm_valid_edit_data())
    fpm_edit_passitem(&gui->edit_window, glb_edit_data);
}


gboolean
on_app_safe_delete_event               (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    if (ini->enable_tray_icon)
	fpm_tr_toggle_main_window(FALSE);
    else
	fpm_quit();

  return TRUE;
}


gboolean
on_clist_main_button_press_event       (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
 if (event->button == 3) {
    glb_click_btn = TRUE;
 }

 if (event->button == 2) {
  if((g_timer_elapsed(glb_timer_click, NULL) < FPM_MAX_DOUBLE_CLICK_TIME) && (glb_click_row == glb_cur_row))
  {
    glb_click_count++;
  }
  else
  {
    g_timer_reset(glb_timer_click);
    glb_click_count = 1;
    glb_click_row = glb_cur_row;
  }
 }


  return FALSE;
}


gboolean
on_clist_main_button_release_event     (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
 if (event->button == 2) {
  if((g_timer_elapsed(glb_timer_click, NULL) < FPM_MAX_DOUBLE_CLICK_TIME) &&
     (glb_click_count > 1))
  {
    if(glb_edit_data != NULL)
	fpm_middle_dblclick(glb_edit_data);
  }
 }

  return FALSE;
}

void
on_clist_main_select_row               (GtkTreeView *treeview,
					gpointer data)
{
  GtkTreeSelection *selection;
  GtkTreeIter selectedIter;
  GtkTreeModel *model;
  gchar *row;

  selection = gtk_tree_view_get_selection(treeview);
  if (gtk_tree_selection_get_selected(selection, &model, &selectedIter)) {

	gtk_tree_model_get(model, &selectedIter,
	  FPM_DATA_POINTER, &glb_edit_data,
	  -1);

    row = gtk_tree_model_get_string_from_iter(model, &selectedIter);
    glb_cur_row = atoi(row);
	g_free(row);

    if (glb_click_btn) {

		if (gui->context_menu == NULL)
			gui->context_menu = GTK_MENU(create_context_menu());

        gtk_menu_popup (gui->context_menu, NULL, NULL, NULL, NULL,
			0, gtk_get_current_event_time ());

	glb_click_btn = FALSE;
    }
  }
  fpm_sensitive_menu(TRUE);
}


void
on_save1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  passfile_save(glb_filename);
}


void
on_exit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  fpm_quit();
}


void
on_button_save_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  passfile_save(glb_filename);
}


gboolean
on_dialog_edit_passitem_delete_event   (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  GtkTextIter start, end;
  GtkTextView* notes;
  GtkTextBuffer*  buffer;
  gchar *entry_notes;

  notes = GTK_TEXT_VIEW(lookup_widget(gui->edit_window, "text_notes"));
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (notes));
  gtk_text_buffer_get_bounds (buffer, &start, &end);
  entry_notes = gtk_text_iter_get_text (&start, &end);

  if (strcmp(glb_edit_data->title, fpm_get_entry(gui->edit_window, "entry_title"))
	|| strcmp(glb_edit_data->arg, fpm_get_entry(gui->edit_window, "entry_arg"))
	|| strcmp(glb_edit_data->user, fpm_get_entry(gui->edit_window, "entry_user"))
	|| strcmp(glb_edit_data->notes, entry_notes)
     ) {
	if (fpm_question(GTK_WINDOW(gui->edit_window), _("Are you sure you want discard changes?"))
	    == GTK_RESPONSE_YES) {
		gtk_widget_destroy(gui->edit_window);
/*		gtk_widget_set_sensitive(gui->main_window, TRUE); */
	}
	g_free(entry_notes);
	return TRUE;
  }
  gtk_widget_destroy (gui->edit_window);
/*  gtk_widget_set_sensitive (gui->main_window, TRUE);
  gtk_widget_grab_focus (GTK_WIDGET(lookup_widget(gui->main_window, "clist_main")));
*/
  g_free(entry_notes);
  return FALSE;
}

  /* Make main window active again */

/* void
on_dialog_edit_passitem_destroy        (GtkObject       *object,
                                        gpointer         user_data)
{
  gtk_widget_set_sensitive(gui->main_window, TRUE);
}
*/

void
on_dialog_edit_passitem_show           (GtkWidget       *widget,
                                        gpointer         user_data)
{

  /* Make main window inactive to simulate a modal edit window. 
   * We don't want to make the edit window really modal because
   * then anoying things happen like confirmation dialog boxes'
   * buttons don't work.
   */
  gtk_widget_set_sensitive(gui->main_window, FALSE);

}


void
on_togglebutton_showpass_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    GtkEntry* entry_password;

    entry_password = GTK_ENTRY(lookup_widget (gui->edit_window, "entry_password"));
    gtk_entry_set_visibility(entry_password, togglebutton->active);
}

void
on_button_jump_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  if(fpm_valid_edit_data())
    fpm_jump(glb_edit_data);
}


void
on_button_pass_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  if(fpm_valid_edit_data())
    fpm_clipboard_init(glb_edit_data, ini->copy_target, TRUE, FALSE);
}


void
on_button_user_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  if(fpm_valid_edit_data())
    fpm_clipboard_init(glb_edit_data, ini->copy_target, FALSE, FALSE);
}


void
on_button_password_ok_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  fpm_check_password();
}


void
on_button_password_cancel_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
  if(state->startup_tray) {
    gtk_widget_hide(gui->pass_window);
    state->locked = TRUE;
  } else
    gtk_main_quit();
}


void
on_gpw_toggled                         (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  GtkWidget* widget;

  widget = lookup_widget(glb_win_misc, "entry_bits");
  if(strcmp("", gtk_entry_get_text(GTK_ENTRY(widget))))
  {
     fpm_gpw_set_from_dialog(glb_win_misc);
  }

}


void
on_button_generate_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{

  glb_win_misc = create_dialog_password_gen();
  fpm_gpw_start_screen(glb_win_misc);
  fpm_gpw_set_from_dialog(glb_win_misc);
  gtk_widget_show(glb_win_misc);

}

void
on_button_gen_generate_now_clicked     (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget* widget;

  widget = GTK_WIDGET(create_window_entropy());

  gtk_window_set_transient_for (GTK_WINDOW(widget), GTK_WINDOW(gui->main_window));
  gtk_widget_show(widget);
  while (gtk_events_pending()) gtk_main_iteration();

  fpm_gpw_fillin_password(glb_win_misc);

  gtk_widget_hide(widget);
}


void
on_button_gen_ok_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkEntry *entry1, *entry2;
  gchar* text;

  entry1 = GTK_ENTRY(lookup_widget(glb_win_misc, "entry_gen_password"));
  entry2 = GTK_ENTRY(lookup_widget(gui->edit_window, "entry_password"));

  text = gtk_editable_get_chars (GTK_EDITABLE(entry1), 0, -1);
  if (strcmp(text, "")) gtk_entry_set_text(entry2, text);

  gtk_widget_destroy(glb_win_misc);
}


void
on_button_gen_cancel_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  gtk_widget_destroy(glb_win_misc);
}


void
on_entry_password_activate             (GtkEditable     *editable,
                                        gpointer         user_data)
{
  fpm_check_password();
}


void
on_button_cpw_ok_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
  fpm_set_password();
}

gboolean
on_button_cpw_cancel_clicked           (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  if(old_context == NULL)
  {
    /* We must be running for the first time.  Cancel quits. */
    gtk_main_quit();
  }
  else
  {
    /* User is canceling from changing a password.  Just close the screen. */
    gtk_widget_destroy(glb_win_misc);
  }

  return TRUE;

}



void
on_copy_pass_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if(fpm_valid_edit_data())
    fpm_clipboard_init(glb_edit_data, ini->copy_target, TRUE, FALSE);
}


void
on_copy_user_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if(fpm_valid_edit_data())
    fpm_clipboard_init(glb_edit_data, ini->copy_target, FALSE, FALSE);
}


void
on_item_password_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GtkComboBox *combo_box;
    glb_win_misc = create_dialog_cpw();
    gtk_window_set_transient_for(GTK_WINDOW(glb_win_misc), GTK_WINDOW(gui->main_window));

/*
    combo_box = GTK_COMBO_BOX(lookup_widget(glb_win_misc, "key_file_combo"));
    gtk_combo_box_append_text (combo_box, C_("key_file","<NONE>"));
    gtk_combo_box_set_active(combo_box, 0);
    */

    gtk_widget_show(glb_win_misc);
}

void
on_item_delete_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if(fpm_valid_edit_data())
  {
    if(fpm_question(GTK_WINDOW(gui->main_window), _("Are you sure you want to delete this item?"))
	== GTK_RESPONSE_YES) {
	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar *dia_str;

	glb_pass_list=g_list_remove(glb_pass_list, glb_edit_data);
	model = gtk_tree_view_get_model(gui->main_clist);
	gtk_tree_model_get_iter_from_string(model, &iter, g_strdup_printf("%i",glb_cur_row));
	gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	glb_num_row--;

	glb_edit_data = NULL;
	fpm_sensitive_menu(FALSE);
//	state->dirty = TRUE;
	fpm_dirty(TRUE);
	if (ini->save_on_delete)
	    passfile_save(glb_filename);

	dia_str = g_strdup_printf(_("Passwords in category: %d"), glb_num_row);
	fpm_statusbar_push(dia_str);
	g_free(dia_str);
    }
  }
}

void
on_dialog_prefs_destroy                (GtkObject       *object,
                                        gpointer         user_data)
{
  gtk_widget_set_sensitive(gui->main_window, TRUE);
}


void
on_export1_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget     *dialog;
	GtkWidget     *combo_box;
	GtkWidget     *combo_box_category;
	const gchar *fpm_directory;

	dialog = gtk_file_chooser_dialog_new (
			_("Export passwords"),
			GTK_WINDOW (gui->main_window),
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_SAVE, GTK_RESPONSE_OK,
			NULL);

    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

    fpm_directory = g_get_home_dir();
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), fpm_directory);

    GtkWidget *table;
    table = gtk_table_new (3, 2, FALSE);
    gtk_table_attach (GTK_TABLE(table),
	gtk_label_new (_("Export type:")), 0,1,0,1,
	0,0,15,0
	);
/*
	combo_box = gtk_combo_box_new_text ();
	gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), _("Plain XML"));
	gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box), 0);
    gtk_table_attach_defaults (GTK_TABLE(table),
	GTK_WIDGET(combo_box), 1,2,0,1);

    gtk_table_attach (GTK_TABLE(table),
	gtk_label_new (_("Launchers:")), 0,1,1,2,
	0,0,15,0
	);
	combo_box = gtk_combo_box_new_text ();
	gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), _("Do not export"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), _("Export only the launchers, which are used in the selected category"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), _("Export all"));
	gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box), 1);
    gtk_table_attach_defaults (GTK_TABLE(table),
	GTK_WIDGET(combo_box), 1,2,1,2);

    gtk_table_attach (GTK_TABLE(table),
	gtk_label_new (_("Category:")), 0,1,2,3,
	0,0,15,0
	);

	combo_box_category = gtk_combo_box_new_text ();
	fpm_populate_combo_box(GTK_COMBO_BOX(combo_box_category), state->category);
	gtk_table_attach_defaults (GTK_TABLE(table), GTK_WIDGET(combo_box_category), 1,2,2,3);
  */

	gtk_widget_show_all (table);

	gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (dialog), GTK_WIDGET(table));

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK) {

        gchar *filename;
        gchar *export_category;
/*
        gint export_launchers;
        */

        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
 //       export_launchers = gtk_combo_box_get_active (GTK_COMBO_BOX(combo_box));
 //       export_category = gtk_combo_box_get_active_text (GTK_COMBO_BOX(combo_box_category));
          export_category = g_strdup(FPM_NONE_CAT_MSG);
          printf("%s", export_category);
        fpm_file_export(filename, 0, export_category);

 //       g_free(export_category);
        g_free(filename);
    }
  gtk_widget_destroy (dialog);
}


void
on_import_passwords1_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GtkWidget *dialog;
    const gchar *fpm_directory;
    GtkWidget *combo_box;
    GtkWidget *combo_box_category;
    GtkWidget *table;

    dialog = gtk_file_chooser_dialog_new (_("Import Password File"),
    				      GTK_WINDOW(gui->main_window),
    				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);
    fpm_directory = g_get_home_dir();
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), fpm_directory);

    table = gtk_table_new (2, 2, FALSE);
    gtk_table_attach (GTK_TABLE(table), gtk_label_new (_("Import type:")), 0,1,0,1, 0,0,15,0);

    combo_box = gtk_combo_box_new_text ();
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), _("Plain XML"));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box), 0);
    gtk_table_attach_defaults (GTK_TABLE(table),GTK_WIDGET(combo_box), 1,2,0,1);

    gtk_table_attach (GTK_TABLE(table), gtk_label_new (_("Launchers:")), 0,1,1,2, 0,0,15,0);

    combo_box = gtk_combo_box_new_text ();
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), _("Do not import"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), _("Import only new launchers"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), _("Import and replace all current launchers"));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box), 1);
    gtk_table_attach_defaults (GTK_TABLE(table),GTK_WIDGET(combo_box), 1,2,1,2);

    gtk_table_attach (GTK_TABLE(table), gtk_label_new (_("Category:")), 0,1,2,3, 0,0,15,0);

    combo_box_category = gtk_combo_box_new_text ();
    fpm_populate_combo_box(GTK_COMBO_BOX(combo_box_category), state->category);
    gtk_combo_box_remove_text (GTK_COMBO_BOX(combo_box_category), 0);
    gtk_combo_box_remove_text (GTK_COMBO_BOX(combo_box_category), 0);
    gtk_combo_box_insert_text (GTK_COMBO_BOX(combo_box_category), 0, _("Do not change the category of items and import as it is."));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box_category), 0);
    gtk_table_attach_defaults (GTK_TABLE(table), GTK_WIDGET(combo_box_category), 1,2,2,3);

    gtk_widget_show_all (table);
    gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (dialog), GTK_WIDGET(table));

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
	gchar *filename;
	gchar *message;
	gchar *import_category;
	gint import_launchers;
	gint import_status;

        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        import_launchers = gtk_combo_box_get_active (GTK_COMBO_BOX(combo_box));
	if(gtk_combo_box_get_active (GTK_COMBO_BOX(combo_box_category)))
	    import_category = gtk_combo_box_get_active_text (GTK_COMBO_BOX(combo_box_category));
	else
	    import_category = "";

	gtk_widget_destroy (dialog);

	import_status = fpm_file_import(filename, import_launchers, import_category, &message);

	if(import_status == 0) {
	    /* Switch view to all passwords */
	    fpm_clist_populate_cat_list();
	    gtk_combo_box_set_active(GTK_COMBO_BOX(lookup_widget(gui->main_window, "optionmenu_category")), 0);
	    fpm_statusbar_push(message);
	} else {
	    fpm_message(GTK_WINDOW(gui->main_window), message, GTK_MESSAGE_WARNING);
	}

	g_free (message);
        g_free (filename);
    } else {
	gtk_widget_destroy (dialog);
    }
}

void
on_current_password_file1_activate     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gchar *dia_str;

	dia_str = g_strdup_printf(_("Current location of password file:\n"
		"%s\n\nCipher: %s"), glb_filename, cipher->name);
	fpm_message(GTK_WINDOW(gui->main_window), dia_str, GTK_MESSAGE_INFO);
	g_free(dia_str);

}

void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GtkWidget *dialog;
    gchar *fpm_directory;

    if (state->dirty) {
	if(fpm_question(GTK_WINDOW(gui->main_window), _("Do you want to save changes?"))
	    == GTK_RESPONSE_YES) {
	    passfile_save(glb_filename);
	}
    }

    dialog = gtk_file_chooser_dialog_new (_("Open Password File"),
				      GTK_WINDOW(gui->main_window),
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);

    fpm_directory = g_build_filename (g_get_home_dir(), FPM_DIR, NULL);

    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), fpm_directory);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
        glb_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
//	unlock_fpm_file1();
	fpm_init(glb_filename, 0);
    }
    gtk_widget_destroy (dialog);
}


void
on_save_as1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GtkWidget *dialog;
    gchar *fpm_directory;

    dialog = gtk_file_chooser_dialog_new (_("Save Password File"),
    				      GTK_WINDOW(gui->main_window),
    				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

    fpm_directory = g_build_filename (g_get_home_dir(), FPM_DIR, NULL);

    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), fpm_directory);
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), _("Untitled password file"));

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
	char *filename;

        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	passfile_save(filename);
        g_free (filename);
    }
    gtk_widget_destroy (dialog);
}

void
on_preferences2_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    glb_win_misc = create_dialog_preferences();
    gtk_window_set_transient_for (GTK_WINDOW(glb_win_misc), GTK_WINDOW(gui->main_window));
//    gtk_window_set_destroy_with_parent (GTK_WINDOW(glb_win_misc), TRUE);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "save_on_add")), ini->save_on_add);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "save_on_change")), ini->save_on_change);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "save_on_delete")), ini->save_on_delete);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "save_on_quit")), ini->save_on_quit);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "create_backup")), ini->create_backup);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(lookup_widget (glb_win_misc, "number_backup_files")), ini->number_backup_files);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "search_in_title")), ini->search_in_title);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "search_in_url")), ini->search_in_url);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "search_in_user")), ini->search_in_user);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "search_in_notes")), ini->search_in_notes);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "search_match_case")), ini->search_match_case);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "search_limit_category")), ini->search_limit_category);

    if (ini->create_backup)
	gtk_widget_set_sensitive (GTK_WIDGET(lookup_widget (glb_win_misc, "backup_files_hbox")), TRUE);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "enable_tray_icon")), ini->enable_tray_icon);
    if (ini->enable_tray_icon) {
	gtk_widget_set_sensitive (GTK_WIDGET(lookup_widget (glb_win_misc, "tr_always_visible")), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET(lookup_widget (glb_win_misc, "tr_auto_hide")), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET(lookup_widget (glb_win_misc, "tr_auto_lock")), TRUE);
    }

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "tr_always_visible")), ini->tr_always_visible);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "tr_auto_hide")), ini->tr_auto_hide);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(lookup_widget (glb_win_misc, "tr_auto_hide_minutes")), ini->tr_auto_hide_minutes);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "tr_auto_lock")), ini->tr_auto_lock);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(lookup_widget (glb_win_misc, "tr_auto_lock_minutes")), ini->tr_auto_lock_minutes);
    if (!ini->enable_tray_icon) {
	gtk_widget_set_sensitive (GTK_WIDGET(lookup_widget (glb_win_misc , "tr_auto_hide_align")), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET(lookup_widget (glb_win_misc , "tr_auto_lock_align")), FALSE);
    }

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "startup_category")), ini->startup_category_active);
	fpm_populate_combo_box(GTK_COMBO_BOX(lookup_widget (glb_win_misc, "startup_category_combo")), ini->startup_category);

    fpm_copy_target_combo(GTK_COMBO_BOX(lookup_widget(glb_win_misc, "copy_target_combo")), ini->copy_target);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "clear_target")), ini->clear_target);
    gtk_combo_box_set_active (GTK_COMBO_BOX(lookup_widget(glb_win_misc, "dblclick_action_combo")), ini->dblclick_action);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "startup_search")), ini->startup_search);

    if (gtk_dialog_run (GTK_DIALOG (glb_win_misc)) == GTK_RESPONSE_OK)
    {
	ini->save_on_add = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "save_on_add")));
	ini->save_on_change = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "save_on_change")));
	ini->save_on_delete = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "save_on_delete")));
	ini->save_on_quit = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "save_on_quit")));
	ini->create_backup = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "create_backup")));
	ini->number_backup_files = gtk_spin_button_get_value (GTK_SPIN_BUTTON(lookup_widget (glb_win_misc, "number_backup_files")));

	ini->search_in_title = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "search_in_title")));
	ini->search_in_url = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "search_in_url")));
	ini->search_in_user = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "search_in_user")));
	ini->search_in_notes = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "search_in_notes")));
	ini->search_match_case = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "search_match_case")));
	ini->search_limit_category = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "search_limit_category")));

	ini->enable_tray_icon = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "enable_tray_icon")));
	ini->tr_always_visible = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "tr_always_visible")));
	ini->tr_auto_hide = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "tr_auto_hide")));
	ini->tr_auto_hide_minutes = gtk_spin_button_get_value (GTK_SPIN_BUTTON(lookup_widget (glb_win_misc, "tr_auto_hide_minutes")));
	ini->tr_auto_lock = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "tr_auto_lock")));
	ini->tr_auto_lock_minutes = gtk_spin_button_get_value (GTK_SPIN_BUTTON(lookup_widget (glb_win_misc, "tr_auto_lock_minutes")));

	ini->startup_category_active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "startup_category")));

	ini->clear_target = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(lookup_widget (glb_win_misc, "clear_target")));

	if (ini->enable_tray_icon)
	    fpm_tray_icon();
	else {
	    fpm_tr_cleanup();
	    gtk_widget_hide (GTK_WIDGET(lookup_widget (gui->main_window , "hide_to_tray")));
//	    g_signal_handlers_disconnect_by_func (gui->main_window, G_CALLBACK (on_app_safe_event), NULL);
	    g_source_remove_by_user_data (gui->main_window);
	}

	fpm_ini_save();
    }
    gtk_widget_destroy (glb_win_misc);
}

void
on_create_backup_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    gtk_widget_set_sensitive (GTK_WIDGET(lookup_widget (glb_win_misc , "backup_files_hbox")), togglebutton->active);
}

void
on_add_item1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    fpm_new_passitem(&gui->edit_window, &glb_edit_data);
}

void
on_edit_item1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if(fpm_valid_edit_data())
    fpm_edit_passitem(&gui->edit_window, glb_edit_data);
}

gboolean
on_dialog_password_delete_event        (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  if(state->startup_tray) {
    gtk_widget_hide(gui->pass_window);
    state->locked = TRUE;
  } else
    gtk_main_quit();

  return TRUE;
}

void
on_optionmenu_category_changed         (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
    gchar *dia_str;

    state->category = gtk_combo_box_get_active_text(combobox);
    fpm_clist_create_view(state->category);

    dia_str = g_strdup_printf(_("Passwords in category: %d"), glb_num_row);
//	dia_str = g_strdup_printf(ngettext ("One password in category", "Passwords in category: %d", glb_num_row), glb_num_row);
    fpm_statusbar_push(dia_str);
    g_free(dia_str);
    glb_edit_data = NULL;
}

void
on_find_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget (gui->main_window, "find_match_case")), ini->search_match_case);

    gtk_widget_show (GTK_WIDGET(lookup_widget (gui->main_window, "findbar")));
    gtk_widget_grab_focus (GTK_WIDGET(lookup_widget (gui->main_window, "find_entry")));
}


void
on_find_close_clicked                  (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
    gtk_widget_hide (GTK_WIDGET(lookup_widget (gui->main_window, "findbar")));
    fpm_sensitive_menu(FALSE);
    fpm_clist_create_view(state->category);
}

gboolean
on_find_entry_key_press_event          (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
    // Escape
    if (event->keyval == 0xff1b) {
	gtk_widget_hide (GTK_WIDGET(lookup_widget (gui->main_window, "findbar")));
	fpm_sensitive_menu(FALSE);
	fpm_clist_create_view(state->category);
    	return TRUE;
    }

  return FALSE;
}

void
on_find_entry_changed                  (GtkEditable     *editable,
                                        gpointer         user_data)
{
	gchar *search_text;

	fpm_sensitive_menu(FALSE);
	search_text = gtk_editable_get_chars(editable, 0, -1);
	fpm_search(search_text, FALSE);
	g_free(search_text);
}


void
on_find_match_case_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    ini->search_match_case = togglebutton->active;
    gtk_widget_activate(GTK_WIDGET(lookup_widget (gui->main_window, "find_entry")));
}


void
on_find_entry_activate                 (GtkEntry        *entry,
                                        gpointer         user_data)
{
    fpm_search(gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1), TRUE);
}

void
tray_icon_on_click			(GtkStatusIcon *status_icon, 
					 gpointer user_data)
{
    if (state->locked) {
	if (GTK_WIDGET_VISIBLE(gui->pass_window))
	    gtk_widget_hide(gui->pass_window);
	else
	    fpm_unlock();
    } else {
	if(state->minimized && ini->after_unhide_set_startup_category)
	    fpm_switch_view_category(ini->startup_category);
    fpm_tr_toggle_main_window(FALSE);
    }
}

void
tray_icon_on_menu			(GtkStatusIcon *status_icon, guint button, 
                    			 guint activate_time, gpointer user_data)
{
	if (gui->tray_menu == NULL)
		gui->tray_menu = GTK_MENU(create_tray_menu());

	if (!state->locked) {
		gtk_widget_hide (GTK_WIDGET(lookup_widget (GTK_WIDGET(gui->tray_menu) , "unlock1")));
		gtk_widget_show (GTK_WIDGET(lookup_widget (GTK_WIDGET(gui->tray_menu) , "lock1")));
		gtk_widget_show (GTK_WIDGET(lookup_widget (GTK_WIDGET(gui->tray_menu) , "show1")));
		gtk_widget_show (GTK_WIDGET(lookup_widget (GTK_WIDGET(gui->tray_menu) , "tr_about1")));
	} else {
		gtk_widget_show (GTK_WIDGET(lookup_widget (GTK_WIDGET(gui->tray_menu) , "unlock1")));
		gtk_widget_hide (GTK_WIDGET(lookup_widget (GTK_WIDGET(gui->tray_menu) , "show1")));
		gtk_widget_hide (GTK_WIDGET(lookup_widget (GTK_WIDGET(gui->tray_menu) , "lock1")));
		gtk_widget_hide (GTK_WIDGET(lookup_widget (GTK_WIDGET(gui->tray_menu) , "tr_about1")));
	}

	gtk_menu_popup (gui->tray_menu, NULL, NULL, gtk_status_icon_position_menu, status_icon,
					0, gtk_get_current_event_time());
}

void
on_enable_tray_icon_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    gtk_widget_set_sensitive (GTK_WIDGET(lookup_widget (glb_win_misc , "tr_always_visible")), togglebutton->active);
    gtk_widget_set_sensitive (GTK_WIDGET(lookup_widget (glb_win_misc , "tr_auto_hide")), togglebutton->active);
    gtk_widget_set_sensitive (GTK_WIDGET(lookup_widget (glb_win_misc , "tr_auto_lock")), togglebutton->active);

    gtk_widget_set_sensitive (GTK_WIDGET(lookup_widget (glb_win_misc , "tr_auto_hide_align")), togglebutton->active & ini->tr_auto_hide);
    gtk_widget_set_sensitive (GTK_WIDGET(lookup_widget (glb_win_misc , "tr_auto_lock_align")), togglebutton->active & ini->tr_auto_lock);
}

void
on_lock1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    fpm_lock();
}


void
on_tr_auto_hide_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    gtk_widget_set_sensitive (GTK_WIDGET(lookup_widget (glb_win_misc , "tr_auto_hide_align")), togglebutton->active);
}


void
on_tr_auto_lock_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    gtk_widget_set_sensitive (GTK_WIDGET(lookup_widget (glb_win_misc , "tr_auto_lock_align")), togglebutton->active);
}

void
on_hide_to_tray_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    fpm_tr_toggle_main_window(FALSE);
}


gboolean
on_app_safe_window_state_event         (GtkWidget       *widget,
                                        GdkEventWindowState	 *event,
                                        gpointer         user_data)
{
    if (event->changed_mask & GDK_WINDOW_STATE_ICONIFIED)
	state->minimized = (event->new_window_state & GDK_WINDOW_STATE_ICONIFIED) ? 1 : 0;

    return FALSE;
}

void
on_show1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	fpm_tr_toggle_main_window(FALSE);
}

gboolean
on_app_safe_focus_in_event             (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data)
{
    if (ini->tr_auto_hide)
	g_source_remove_by_user_data (gui->main_window);

    return FALSE;
}


gboolean
on_app_safe_focus_out_event            (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data)
{
    if (!fpm_window_check() && ini->tr_auto_hide && !state->minimized) {
	g_timeout_add_seconds(ini->tr_auto_hide_minutes*60, (GSourceFunc) fpm_auto_hide, gui->main_window);
}

  return FALSE;
}


void
on_unlock1_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
//    gtk_status_icon_set_visible(gui->tray_icon, FALSE);
//    state->locked = FALSE;
//    unlock_fpm_file1();
    fpm_unlock();
//    fpm_init(glb_filename, 0);
}

gboolean
on_toolbar1_popup_context_menu         (GtkToolbar      *toolbar,
                                        gint             arg1,
                                        gint             arg2,
                                        gint             arg3,
                                        gpointer         user_data)
{
    gtk_menu_popup (gui->toolbar_menu, NULL, NULL, NULL, NULL,
	    	        0, gtk_get_current_event_time ());
    return FALSE;
}

void
on_icons_and_text1_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ini->toolbar_icon_style = GTK_TOOLBAR_BOTH;
	gtk_toolbar_set_style(user_data, GTK_TOOLBAR_BOTH);
	gtk_widget_show(lookup_widget(user_data, "label_category"));
}


void
on_icons_only1_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ini->toolbar_icon_style = GTK_TOOLBAR_ICONS;
	gtk_toolbar_set_style(user_data, GTK_TOOLBAR_ICONS);
	gtk_widget_hide(lookup_widget(user_data, "label_category"));
}


void
on_text_only1_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ini->toolbar_icon_style = GTK_TOOLBAR_TEXT;
	gtk_toolbar_set_style(user_data, GTK_TOOLBAR_TEXT);
	gtk_widget_hide(lookup_widget(user_data, "label_category"));
}


void
on_hide_toolbar1_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gtk_widget_hide(lookup_widget(gui->main_window, "toolbar1"));
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(gui->main_window, "toolbar2")), FALSE);
}

void
on_startup_category_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    gtk_widget_set_sensitive (GTK_WIDGET(lookup_widget (glb_win_misc , "startup_category_combo")), togglebutton->active);
}


void
on_startup_category_combo_changed      (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
    ini->startup_category = gtk_combo_box_get_active_text(combobox);
}

void
on_toolbar2_activate                   (GtkCheckMenuItem *menuitem,
                                        gpointer         user_data)
{
    ini->toolbar_visible = gtk_check_menu_item_get_active (menuitem);
    ini->toolbar_visible ? gtk_widget_show(lookup_widget(gui->main_window, "toolbar1")) : gtk_widget_hide(lookup_widget(gui->main_window, "toolbar1"));
}


void
on_select_key_file_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkWidget *dialog;
    const gchar *fpm_directory;

    dialog = gtk_file_chooser_dialog_new (_("Select Key File"),
    				      GTK_WINDOW(glb_win_misc),
    				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);

    fpm_directory = g_get_home_dir();
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), fpm_directory);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
	char *filename;

        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	GtkComboBox *combo_box;

        combo_box = GTK_COMBO_BOX(lookup_widget(gtk_widget_get_toplevel(GTK_WIDGET(button)), "key_file_combo"));
	gtk_combo_box_insert_text(combo_box, 1, filename);
        gtk_combo_box_set_active(combo_box, 1);

	g_free (filename);
    }
    gtk_widget_destroy (dialog);
}

void
on_dialog_edit_passitem_destroy        (GtkObject       *object,
                                        gpointer         user_data)
{
  if(state->new_entry) {
    g_free(glb_edit_data);
    state->new_entry = FALSE;
  }

  if(glb_num_row)
    gtk_tree_view_set_cursor(gui->main_clist, gtk_tree_path_new_from_string(g_strdup_printf("%i", glb_cur_row)), NULL, FALSE);

  gtk_widget_set_sensitive(gui->main_window, TRUE);
  gtk_widget_grab_focus(GTK_WIDGET(gui->main_clist));

  gui->edit_window = NULL;

}

void
on_copy_target_combo_changed        (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
    ini->copy_target = gtk_combo_box_get_active(combobox);

}


void
on_new_file_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GtkWidget *dialog;
    gchar *fpm_directory;

    dialog = gtk_file_chooser_dialog_new (_("New Password File"),
    				      GTK_WINDOW(gui->main_window),
    				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

    fpm_directory = g_build_filename (g_get_home_dir(), FPM_DIR, NULL);

    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), fpm_directory);
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), _("Untitled password file"));

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        glb_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

    gtk_widget_set_sensitive(gui->main_window, FALSE);
    fpm_launcher_remove_all();
    fpm_clear_list();
    fpm_clist_populate_cat_list();
    gtk_combo_box_set_active(GTK_COMBO_BOX(lookup_widget(gui->main_window, "optionmenu_category")), 0);

    fpm_crypt_init("");
    old_context = NULL;

    fpm_init_launchers();

    gtk_widget_destroy (dialog);

    GtkComboBox *combo_box;
    glb_win_misc = create_dialog_cpw();
    gtk_window_set_transient_for(GTK_WINDOW(glb_win_misc), GTK_WINDOW(gui->main_window));

    combo_box = GTK_COMBO_BOX(lookup_widget(glb_win_misc, "key_file_combo"));
    gtk_combo_box_append_text (combo_box, C_("key_file","<NONE>"));
    gtk_combo_box_set_active(combo_box, 0);

    gtk_widget_show(glb_win_misc);
    } else {
	gtk_widget_destroy (dialog);
    }
}

void
on_combo_entry_launcher_changed        (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
  gint launcher_idx;

  launcher_idx = gtk_combo_box_get_active(combobox);

  if(launcher_idx != -1)
    fpm_launcher_show(g_list_nth_data(glb_launcher_list, launcher_idx));
}


void
on_btn_launcher_remove_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkComboBox *combo_box;
  gint launcher_idx;

  combo_box = GTK_COMBO_BOX(lookup_widget(glb_win_misc, "combo_entry_launcher"));
  launcher_idx = gtk_combo_box_get_active(combo_box);
  fpm_launcher_remove(g_list_nth_data(glb_launcher_list, launcher_idx));
  gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "btn_launcher_add"), TRUE);
}


void
on_btn_launcher_add_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
    fpm_launcher_add();
}

void
on_btn_launcher_apply_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkComboBox *combo_box;
  gint launcher_idx;
  gchar *name;

  combo_box = GTK_COMBO_BOX(lookup_widget(glb_win_misc, "combo_entry_launcher"));
  launcher_idx = gtk_combo_box_get_active(combo_box);
  name = gtk_combo_box_get_active_text(combo_box);

  if(strlen(name)) {	/* Launcher name is not empty, so we need update it */
    g_free(current_launcher->title);
    current_launcher->title = g_strdup(name);
    g_free(current_launcher->cmdline);
    current_launcher->cmdline = g_strdup(gtk_entry_get_text(GTK_ENTRY(lookup_widget(glb_win_misc, "entry_launcher_cmd"))));

    current_launcher->copy_user=0;
    if (GTK_TOGGLE_BUTTON(lookup_widget(glb_win_misc, "radiobutton_user_clip"))->active)
	current_launcher->copy_user=2;
    if (GTK_TOGGLE_BUTTON(lookup_widget(glb_win_misc, "radiobutton_user_pri"))->active)
	current_launcher->copy_user=1;

    current_launcher->copy_password=0;
    if (GTK_TOGGLE_BUTTON(lookup_widget(glb_win_misc, "radiobutton_pass_clip"))->active)
	current_launcher->copy_password=2;
    if (GTK_TOGGLE_BUTTON(lookup_widget(glb_win_misc, "radiobutton_pass_pri"))->active)
	current_launcher->copy_password=1;

    fpm_launcher_populate_combo_box(g_list_index(glb_launcher_list, current_launcher));
    gtk_widget_set_sensitive(lookup_widget(glb_win_misc, "btn_launcher_add"), TRUE);

    fpm_dirty(TRUE);

  } else {
    fpm_message(GTK_WINDOW(glb_win_misc), _("Please enter launcher name."), GTK_MESSAGE_WARNING);
  }

  g_free(name);

}


void
dialog_launcher_destroy                (GtkObject       *object,
                                        gpointer         user_data)
{
  /* If user interupt launcher edit, we remove it */
  if((current_launcher != NULL) && !strcmp(current_launcher->title,"")) {
    glb_launcher_list = g_list_remove(glb_launcher_list, current_launcher);
    g_free(current_launcher);
    current_launcher = NULL;
  }
}


void
on_btn_launcher_close_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  gtk_widget_destroy(glb_win_misc);
}


void
on_copy_both1_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if(fpm_valid_edit_data())
    fpm_clipboard_init(glb_edit_data, ini->copy_target, FALSE, TRUE);
}

void
on_btn_show_key_file_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  if(!GTK_WIDGET_VISIBLE(lookup_widget(gui->pass_window, "key_file_label"))) {
      gtk_widget_show(lookup_widget(gui->pass_window, "key_file_label"));
      gtk_widget_show(lookup_widget(gui->pass_window, "key_file_combo"));
      gtk_widget_show(lookup_widget(gui->pass_window, "key_file_select_btn"));
      gtk_widget_set_tooltip_text (lookup_widget(gui->pass_window, "btn_show_key_file"), _("Hide Key File Dialog"));
  } else {
      gtk_widget_hide(lookup_widget(gui->pass_window, "key_file_label"));
      gtk_widget_hide(lookup_widget(gui->pass_window, "key_file_combo"));
      gtk_widget_hide(lookup_widget(gui->pass_window, "key_file_select_btn"));
      gtk_widget_set_tooltip_text (lookup_widget(gui->pass_window, "btn_show_key_file"), _("Show Key File Dialog"));
  }
}


void
on_launcher_menu_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  fpm_launcher_init();
}

void
on_clone_item1_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if(fpm_valid_edit_data()) {

    fpm_data* new_data;

    new_data = g_malloc0(sizeof(fpm_data));
    state->new_entry = TRUE;

    new_data->title=g_strdup(glb_edit_data->title);
    new_data->arg=g_strdup(glb_edit_data->arg);
    new_data->user=g_strdup(glb_edit_data->user);
    new_data->notes=g_strdup(glb_edit_data->notes);
    new_data->category=g_strdup(glb_edit_data->category);
    new_data->launcher=g_strdup(glb_edit_data->launcher);

    strncpy(new_data->password, glb_edit_data->password, FPM_PASSWORD_LEN*2+1);

    glb_edit_data = new_data;

    fpm_edit_passitem(&gui->edit_window, new_data);

  }

}


void
on_clist_main_row_activated            (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{
  fpm_double_click(glb_edit_data);
}

void
on_clist_main_columns_changed          (GtkTreeView     *treeview,
                                        gpointer         user_data)
{

  GtkTreeViewColumn *treeViewColumn = NULL;
  GList *columns;
  gint id;

  g_list_free (columns_order);
  columns_order = NULL;

  columns = gtk_tree_view_get_columns(gui->main_clist);
  while (columns != NULL) {
	id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(columns->data), "index"));
	columns_order = g_list_append(columns_order, GINT_TO_POINTER(id));

	treeViewColumn = GTK_TREE_VIEW_COLUMN(columns->data);

	gtk_tree_view_column_set_resizable (treeViewColumn, TRUE);
	gtk_tree_view_column_set_sizing (treeViewColumn, GTK_TREE_VIEW_COLUMN_FIXED);

	columns = g_list_next(columns);
   }

  if(treeViewColumn != NULL)
    gtk_tree_view_column_set_sizing (treeViewColumn, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

  g_list_free(columns);

}


void
on_view_menu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

  gint id, i = 0;

  id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(menuitem), "index"));

  if ((id == FPM_PASSWORD) && GTK_CHECK_MENU_ITEM(menuitem)->active ) {
    if (!(fpm_question(GTK_WINDOW(gui->main_window), _("Are you sure you want decrypt and display ALL passwords?"))
	== GTK_RESPONSE_YES)) {
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem), FALSE);
	return;
    }
  }

  fpm_view_modify_column(id, GTK_CHECK_MENU_ITEM (menuitem)->active);

  GList *columns = gtk_tree_view_get_columns(gui->main_clist);
  while (columns != NULL) {
    ini->columns_width[i] = gtk_tree_view_column_get_width(columns->data);
    columns = g_list_next(columns);
    i++;
  }
  g_list_free(columns);
  ini->columns_width[i-1] = 150;

  fpm_clist_init();
  fpm_clist_create_view(state->category);

}


void
on_dblclick_action_combo_changed       (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
    ini->dblclick_action = gtk_combo_box_get_active(combobox);
}


void
on_startup_search_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    ini->startup_search = togglebutton->active;

}
