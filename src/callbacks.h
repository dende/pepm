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
 * callbacks.h
 */

#include <gtk/gtk.h>

void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_app_safe_destroy_event              (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_app_safe_destroy                    (GtkObject       *object,
                                        gpointer         user_data);

void
on_button_ok_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_cancel_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_new_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

/* void
on_clist_main_select_row               (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data);
*/


void
on_clist_main_select_row               (GtkTreeView *treeview,
					gpointer data);

void
on_button_edit_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_app_safe_delete_event               (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_clist_main_button_press_event       (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_clist_main_button_release_event     (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_save1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_exit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_button_save_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_dialog_edit_passitem_delete_event   (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_dialog_edit_passitem_destroy        (GtkObject       *object,
                                        gpointer         user_data);

void
on_dialog_edit_passitem_show           (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_togglebutton_showpass_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_button_jump_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_pass_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_user_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_password_ok_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_password_cancel_clicked      (GtkButton       *button,
                                        gpointer         user_data);


void
on_gpw_toggled                         (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_button_generate_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_gen_generate_now_clicked     (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_gen_ok_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_gen_cancel_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_entry_password_activate             (GtkEditable     *editable,
                                        gpointer         user_data);


void
on_button_cpw_ok_clicked               (GtkButton       *button,
                                        gpointer         user_data);
/*
void
on_button_cpw_cancel_clicked           (GtkButton       *button,
                                        gpointer         user_data);
*/

void
on_copy_pass_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_copy_user_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_item_password_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_button_prefs_ok_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_prefs_cancel_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_item_delete_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_button_change_cat_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_dialog_prefs_destroy                (GtkObject       *object,
                                        gpointer         user_data);

void
on_export1_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_import_passwords1_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

/*void
on_button_import_ok_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_import_cancel_clicked        (GtkButton       *button,
                                        gpointer         user_data);
*/
void
on_current_password_file1_activate     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);


void
on_aboutdialog1_close                  (GtkDialog       *dialog,
                                        gpointer         user_data);

void
on_entry_title_changed                 (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_entry_changed                       (GtkEditable     *editable,
                                        gpointer         user_data);

gboolean
on_dialog_edit_passitem_delete_event   (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save_as1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_preferences2_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_create_backup_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);
void
on_add_item1_activate 	              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_edit_item1_activate 	              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_dialog_password_delete_event        (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);


void
on_optionmenu_category_changed         (GtkComboBox     *combobox,
                                        gpointer         user_data);


void
on_find_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_find_close_clicked                  (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

gboolean
on_find_entry_key_press_event          (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);
void
on_find_entry_changed                  (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_find_match_case_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_find_entry_activate                 (GtkEntry        *entry,
                                        gpointer         user_data);


void
on_app_safe_destroy                    (GtkObject       *object,
                                        gpointer         user_data);

gboolean
on_app_safe_delete_event               (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save_as1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_import_passwords1_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_export1_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_exit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_copy_pass_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_copy_user_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_add_item1_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_edit_item1_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_find_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_item_delete_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_preferences1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_item_password_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_preferences2_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_current_password_file1_activate     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_optionmenu_category_changed         (GtkComboBox     *combobox,
                                        gpointer         user_data);

gboolean
on_clist_main_button_press_event       (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_clist_main_button_release_event     (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_clist_main_select_row               (GtkTreeView     *treeview,
                                        gpointer         user_data);

void
on_find_close_clicked                  (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

gboolean
on_find_entry_key_press_event          (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);

void
on_find_entry_changed                  (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_find_entry_activate                 (GtkEntry        *entry,
                                        gpointer         user_data);

void
on_find_match_case_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
tray_icon_on_click			(GtkStatusIcon *status_icon, 
					 gpointer user_data);

void
tray_icon_on_menu			(GtkStatusIcon *status_icon, guint button, 
					 guint activate_time, gpointer user_data);

void
on_enable_tray_icon_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_lock1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_tr_auto_hide_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_tr_auto_lock_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_hide_to_tray_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_app_safe_window_state_event         (GtkWidget       *widget,
                                        GdkEventWindowState	*event,
                                        gpointer         user_data);

void
on_show1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);


gboolean
on_app_safe_focus_in_event             (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

gboolean
on_app_safe_focus_out_event            (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

void
on_unlock1_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_toolbar1_popup_context_menu         (GtkToolbar      *toolbar,
                                        gint             arg1,
                                        gint             arg2,
                                        gint             arg3,
                                        gpointer         user_data);

void
on_icons_and_text1_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_icons_only1_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_text_only1_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_hide_toolbar1_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_startup_category_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data);
void
on_startup_category_combo_changed      (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_toolbar2_activate                   (GtkCheckMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_button_cpw_cancel_clicked           (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);
void
on_select_key_file_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_dialog_edit_passitem_destroy        (GtkObject       *object,
                                        gpointer         user_data);

/*void
on_optionmenu_launcher_changed         (GtkComboBox     *combobox,
                                        gpointer         user_data);
*/
void
on_copy_target_combo_changed        (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_new_file_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_combo_entry_launcher_changed        (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_btn_launcher_remove_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_btn_launcher_add_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_btn_launcher_apply_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
dialog_launcher_destroy                (GtkObject       *object,
                                        gpointer         user_data);

void
on_btn_launcher_close_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_copy_both1_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_btn_show_key_file_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_launcher_menu_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_clone_item1_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_clist_main_row_activated            (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);

void
on_clist_main_columns_changed          (GtkTreeView     *treeview,
                                        gpointer         user_data);

void
on_view_menu_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_dblclick_action_combo_changed       (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_startup_search_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data);
