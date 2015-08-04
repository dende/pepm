/* FIGARO'S PASSWORD MANAGER 2 (pepm)
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
 * fpm.c - general routines
 */

#include "fpm.h"
#include "fpm_gpw.h"
#include "fpm_file.h"
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "fpm_crypt.h"
#include <fcntl.h>
#include <sys/file.h>
#include <stdlib.h>
#include <unistd.h>

#include <glib/gstdio.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>


/* GLOBAL VARIABLES */
GtkWidget* glb_win_misc;  /* Misc window, used for dialogs         */

GtkWidget* glb_win_import; /* The import window                    */
fpm_data* glb_edit_data;  /* The password item being edited now    */
gint glb_num_row;         /* The total number of rows in the CList */
gint glb_cur_row;         /* The last item clicked on in the CList */
gchar* glb_filename;      /* The location of the pasword file.     */
GTimer* glb_timer_click;  /* Timer used to check for double clicks */
gint glb_click_count;	  /* Click count used for double clicks    */
gchar* new_salt;	  /* Salt to use on next save.             */
gchar* old_salt;	  /* Salt to used on last save.            */
gboolean glb_click_btn;	  /* Right button clicked -> context menu  */
gint glb_click_row;       /* Store row for first click on dblclick */

static int lock_fd = -1;

void* old_context;
void* new_context;

char* vstring;
char* file_version;

GList *glb_pass_list;	 /* A GList of fpm_data containing all items */
GList *glb_cat_string_list;	 /* A GList of strings, used to populate combos */
GList *glb_launcher_list;
GList *glb_launcher_string_list;

/* Set default cipher algorithm*/
fpm_cipher_algo cipher_algo = AES256;

/* Current versions of FPM encrypt all fields.  Fields other than the
 * password are decrypted when the program reads the password file,
 * and the password is decrypted as it is needed (to try to prevent it
 * from showing up in coredumps, etc.)  The global variable  glb_need_decrypt
 * is set by the passfile loading routines to specify that the passfile
 * was created with a version of FPM that requires this decryption.  
 * This allows backward compatibility with previous versions of FPM which
 * only encrypted passwords.
 */
gboolean glb_need_decrypt;

GList *columns_order = NULL;

gchar *columns_title [] =
{
   "",
   N_("Title") ,
   N_("URL") ,
   N_("User"),
   N_("Password"),
   N_("Category"),
   N_("Notes") ,
   N_("Launcher")
};

void
lock_fpm_file(gchar* glb_filename)
{
	gchar *dia_str;

	if ( ( lock_fd = open( glb_filename, O_RDONLY )) < 0 )
	{
		dia_str = g_strdup_printf(_("Could not open %s"), 
			glb_filename);
		fpm_message(GTK_WINDOW(gui->main_window), dia_str, GTK_MESSAGE_ERROR);
		perror(glb_filename);
		exit(1);
	}
	if ( flock(lock_fd, LOCK_EX | LOCK_NB) ) {
		dia_str = g_strdup_printf(_("Could not lock %s\nFile already locked."), 
			glb_filename);
		fpm_message(GTK_WINDOW(gui->main_window), dia_str, GTK_MESSAGE_ERROR);
		printf("%s\n", dia_str);
		exit(1);
	}

}

void
unlock_fpm_file(void)
{
	if ( lock_fd > -1 )
	{
		close(lock_fd);
	}
}

void
fpm_new_passitem(GtkWidget** win_edit_ptr, fpm_data** data_ptr)
{
    fpm_data* new_data;
    gchar plaintext[FPM_PASSWORD_LEN+1] = {0};

    new_data = g_malloc0(sizeof(fpm_data));
    state->new_entry = TRUE;

    strncpy(plaintext, "", FPM_PASSWORD_LEN);
    fpm_encrypt_field(	old_context, new_data->password,
			plaintext, FPM_PASSWORD_LEN);
    new_data->title=g_strdup("");
    new_data->arg=g_strdup("");
    new_data->user=g_strdup("");
    new_data->notes=g_strdup("");

    if (g_utf8_collate(state->category, FPM_ALL_CAT_MSG) == 0 || g_utf8_collate(state->category, FPM_NONE_CAT_MSG) == 0)
	new_data->category=g_strdup("");
    else
	new_data->category=g_strdup(state->category);

    new_data->launcher=g_strdup("");
    *data_ptr = new_data;

    fpm_edit_passitem(win_edit_ptr, new_data);
}

void
fpm_gtk_combo_set_popdown_strings(GtkComboBox *combo_box, GList *list, gchar *active_item) {
    gint i = 0, index = 0;

    while (list) {
	gtk_combo_box_append_text (GTK_COMBO_BOX(combo_box), list->data);
	if(!strcmp(active_item, list->data))
	    index = i;
	i++;
	list = g_list_next(list);
    }
  gtk_combo_box_set_active (GTK_COMBO_BOX(combo_box), index);
}

void
fpm_edit_passitem(GtkWidget** win_edit_ptr, fpm_data* data)
{

  GtkComboBoxEntry* combo_box_category;
  GtkComboBox *combo_box_launcher;
  gchar cleartext[FPM_PASSWORD_LEN+1] = {0};
  GtkTextView* notes;
  GtkTextBuffer*  buffer;
  int ix;

  /* Create new dialog box */
  *win_edit_ptr = create_dialog_edit_passitem();

  /* Populate drop down boxes on edit screen */
  fpm_create_category_list(1);
  combo_box_category = GTK_COMBO_BOX_ENTRY(lookup_widget(*win_edit_ptr, "combo_box_category"));
  g_assert(glb_cat_string_list != NULL);
  fpm_gtk_combo_set_popdown_strings(GTK_COMBO_BOX(combo_box_category), glb_cat_string_list, data->category);

  fpm_create_launcher_string_list();
  g_assert(glb_launcher_string_list != NULL);

  combo_box_launcher = GTK_COMBO_BOX(lookup_widget(*win_edit_ptr, "combo_box_launcher"));
  if(glb_launcher_string_list!=NULL)
    fpm_gtk_combo_set_popdown_strings(combo_box_launcher, glb_launcher_string_list, data->launcher);

  /* Load the data into the dialog box */
  fpm_set_entry(*win_edit_ptr, "entry_title", data->title);
  fpm_set_entry(*win_edit_ptr, "entry_arg", data->arg);
  fpm_set_entry(*win_edit_ptr, "entry_user", data->user);

  gtk_toggle_button_set_active(
	GTK_TOGGLE_BUTTON(lookup_widget(*win_edit_ptr, "checkbutton_default")),
	data->default_list);

  ix = strlen(data->password) / 2;
  fpm_decrypt_field(old_context, cleartext, data->password, ix);
  fpm_set_entry(*win_edit_ptr, "entry_password", cleartext);
  wipememory(cleartext, FPM_PASSWORD_LEN);

  notes = GTK_TEXT_VIEW(lookup_widget(*win_edit_ptr, "text_notes"));

  buffer = gtk_text_view_get_buffer(notes);
  gtk_text_buffer_insert_at_cursor(buffer, data->notes ,strlen(data->notes));

  gtk_window_set_transient_for (GTK_WINDOW(*win_edit_ptr), GTK_WINDOW(gui->main_window));
  gtk_widget_show(*win_edit_ptr);
  gtk_widget_set_sensitive(gui->main_window, FALSE);

}

int
fpm_save_passitem(GtkWidget* win_edit, fpm_data* data)
{
  gint row = -1;
  gchar* orig_plaintext;
  gchar plaintext[FPM_PASSWORD_LEN+1] = {0};
  GtkEntry* entry;
  GtkTextView* notes;
  gchar *dia_str;

  fpm_data* data1;

  GtkTextIter start, end;
  GtkTextBuffer *buffer;
  GtkTreeIter iter;
  GtkTreeModel *model;
  gboolean valid;

  /* Set dirty flag so we know we need to save later. */
//  state->dirty = TRUE;
  fpm_dirty(TRUE);

  /* First update data structures */
  data->title = fpm_get_entry(win_edit, "entry_title");
  data->arg = fpm_get_entry(win_edit, "entry_arg");
  data->user = fpm_get_entry(win_edit, "entry_user");
  data->category = fpm_get_combo_entry(win_edit, "combo_box_category");
  data->launcher = fpm_get_combo_entry(win_edit, "combo_box_launcher");

  /* Update default check box */
  data->default_list = (GTK_TOGGLE_BUTTON(lookup_widget(win_edit, "checkbutton_default"))->active);

  /* Update password */
  entry = GTK_ENTRY(lookup_widget(win_edit, "entry_password"));

  orig_plaintext = gtk_editable_get_chars (GTK_EDITABLE(entry), 0, -1);

  if ( strlen(orig_plaintext) > (FPM_PASSWORD_LEN -1 )) {
	dia_str = g_malloc0(256);
	sprintf(dia_str, _("Password exceeded limit of %d characters"
		"\nYour password of %d characters was truncated."),
		FPM_PASSWORD_LEN-1, (int) strlen(orig_plaintext));

	fpm_message(GTK_WINDOW(win_edit), dia_str, GTK_MESSAGE_INFO);
	g_free(dia_str);
  }

  wipememory(plaintext, FPM_PASSWORD_LEN);
  strncpy(plaintext, orig_plaintext, FPM_PASSWORD_LEN-1);

  fpm_encrypt_field(old_context, data->password, plaintext, FPM_PASSWORD_LEN);
  wipememory(orig_plaintext, strlen(orig_plaintext));
  wipememory(plaintext, FPM_PASSWORD_LEN);

  g_free(orig_plaintext);

  /* Update notes */
  notes = GTK_TEXT_VIEW(lookup_widget(win_edit, "text_notes"));
  gtk_text_view_set_editable (GTK_TEXT_VIEW (notes), TRUE);
  g_free(data->notes);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (notes));
  gtk_text_buffer_get_bounds (buffer, &start, &end);
  data->notes = gtk_text_iter_get_text (&start, &end);

  model = gtk_tree_view_get_model(gui->main_clist);
  valid = gtk_tree_model_get_iter_first(model, &iter);

  while (valid) {
    gtk_tree_model_get(model, &iter, FPM_DATA_POINTER, &data1, -1);
    if( data == data1 ) {
	row = atoi(gtk_tree_model_get_string_from_iter(model, &iter));
    }
    valid = gtk_tree_model_iter_next (model, &iter);
  }

  if (row<0)
  {
    /* Keep track of the number of rows in the CList */
    glb_num_row++;

    /* Update the current row */
    row = glb_num_row-1;

    /* Also, add this row to our main GList */
    glb_pass_list=g_list_append(glb_pass_list, data);

//    g_print("inserting...\n");

    if (ini->save_on_add)
	passfile_save(glb_filename);
  }
  else
  {
//    g_print("updating...\n");
    if (ini->save_on_change)
	passfile_save(glb_filename);
  }

  /* Update Category drop down list on main screen */
  fpm_clist_populate_cat_list();
 return 0; // All is OK
}

char*
fpm_get_combo_entry(GtkWidget *win, gchar *entry_name)
{
  gchar *tmp;
  GtkComboBox *combo_box;

  combo_box = GTK_COMBO_BOX(lookup_widget (win, entry_name));
  tmp = gtk_combo_box_get_active_text(combo_box);
  if(!strcmp(tmp, C_("key_file","<NONE>")))
    tmp="";
  return (tmp);
}

char*
fpm_get_entry(GtkWidget *win, gchar *entry_name)
{
  gchar *tmp;
  GtkEntry* entry_field;

  entry_field = GTK_ENTRY(lookup_widget (win, entry_name));
  tmp = gtk_editable_get_chars (GTK_EDITABLE(entry_field), 0, -1);
  return (tmp);
}

void
fpm_set_entry(GtkWidget* win, gchar* entry_name, char* text)
{

  GtkEntry* entry_field;

  entry_field = GTK_ENTRY(lookup_widget (win, entry_name));
  gtk_entry_set_text(entry_field, text);
}

void
fpm_init(char* opt_file_name, int tray_on_startup)
{
    static gboolean first_run = TRUE;
    GtkComboBox *combo_box;

//    glb_launcher_string_list = NULL;

    if (first_run == TRUE) {
	umask(S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

	gui = g_malloc0(sizeof(fpm_gui));
	state = g_malloc0(sizeof(fpm_state));
	cipher = g_malloc0(sizeof(fpm_cipher));

	fpm_ini_load();

	gui->main_window = create_app_safe();
	gtk_window_add_mnemonic(GTK_WINDOW(gui->main_window), gdk_keyval_from_name ("k"), lookup_widget(gui->main_window, "optionmenu_category"));

	fpm_toolbar_style();
	gtk_widget_hide(GTK_WIDGET(lookup_widget (gui->main_window ,"hide_to_tray")));

	gui->main_clist = GTK_TREE_VIEW(lookup_widget (gui->main_window, "clist_main"));

	fpm_clist_init();

	gint i;
        GtkWidget *view = gtk_menu_item_get_submenu(GTK_MENU_ITEM(lookup_widget (gui->main_window ,"view1")));
        GList *children = gtk_container_get_children(GTK_CONTAINER(view));
	for (i = 1; i < FPM_NUM_COLS; i++) {
	    g_object_set_data(G_OBJECT(children->data), "index", GINT_TO_POINTER(i));

	    if (g_list_find(columns_order, GINT_TO_POINTER(i)) != NULL) {
		g_signal_handlers_block_by_func(G_OBJECT(children->data), G_CALLBACK(on_view_menu_activate), NULL);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(children->data), TRUE);
		g_signal_handlers_unblock_by_func(G_OBJECT(children->data), G_CALLBACK(on_view_menu_activate), NULL);
	    }

	    children = g_list_next(children);
	}

	state->new_entry = FALSE;
	state->startup_tray = tray_on_startup;

	glb_timer_click = g_timer_new();
	g_timer_start(glb_timer_click);
	glb_click_count = -1;
	glb_need_decrypt = FALSE;

	fpm_gpw_set_options(8, TRUE, TRUE, TRUE, FALSE, TRUE);

	first_run = FALSE;

    } else {
	gtk_widget_hide(gui->main_window);
    }

  if (opt_file_name)
    glb_filename = opt_file_name;
  else
    glb_filename = g_build_filename (g_get_home_dir(), FPM_DIR, "fpm", NULL);

  if(!g_file_test(g_build_filename (g_get_home_dir(), FPM_DIR, NULL), G_FILE_TEST_IS_DIR))
	g_mkdir(g_build_filename (g_get_home_dir(), FPM_DIR, NULL), S_IRUSR | S_IWUSR | S_IXUSR);

  /* Lock the password file */
    if(g_file_test (glb_filename, G_FILE_TEST_EXISTS))
	lock_fpm_file(glb_filename);

  if(passfile_load(glb_filename)) {
    /* No passwords file so ask for master password */
    glb_win_misc = create_dialog_cpw();
//    gui->pass_window = create_dialog_cpw();
    combo_box = GTK_COMBO_BOX(lookup_widget(glb_win_misc, "key_file_combo"));
    gtk_combo_box_append_text (combo_box, C_("key_file","<NONE>"));
    gtk_combo_box_set_active(combo_box, 0);
    gtk_widget_show(glb_win_misc);
  } else {
//    glb_win_misc = create_dialog_password();
    gui->pass_window = create_dialog_password();

    combo_box = GTK_COMBO_BOX(lookup_widget(gui->pass_window, "key_file_combo"));
    gtk_combo_box_append_text (combo_box, C_("key_file","<NONE>"));
    gtk_combo_box_set_active(combo_box, 0);


  if(tray_on_startup) {
    ini->enable_tray_icon = TRUE;
    ini->tr_always_visible = TRUE;
    state->minimized = TRUE;
    fpm_tray_icon();
    fpm_lock();
 } else {
    gtk_widget_show(gui->pass_window);
 }
}
}

void fpm_unlock() {
    GtkLabel *label;

    passfile_load(glb_filename);

    label = GTK_LABEL(lookup_widget(gui->pass_window, "label_password_message"));
    gtk_widget_hide(GTK_WIDGET(label));
    gtk_widget_grab_focus(lookup_widget(gui->pass_window, "entry_password"));
    gtk_widget_show(gui->pass_window);
}

void
fpm_quit(void)
{
  if (state->dirty) {
    if (ini->save_on_quit)
	passfile_save(glb_filename);
    else
    if(fpm_question(GTK_WINDOW(gui->main_window), _("Do you want to save changes?"))
	== GTK_RESPONSE_YES) {
	passfile_save(glb_filename);
    }
  }

  if (gui->tray_icon != NULL)
    gui->tray_icon = (g_object_unref (gui->tray_icon), NULL);

  fpm_ini_save();

  gtk_main_quit();

}

void
fpm_clipboard_get(GtkClipboard *clip, GtkSelectionData *selection_data, guint info, gpointer data) {

    fpm_clipboard *clipboard = data;

    if(clipboard->is_password) {
	gchar plaintext[FPM_PASSWORD_LEN+1] = {0};
	guint ix;

	ix = strlen(clipboard->password)/2;
	fpm_decrypt_field(old_context, plaintext, clipboard->password, ix);
	gtk_selection_data_set_text(selection_data, plaintext, -1);
	wipememory(plaintext, FPM_PASSWORD_LEN);
	if(ini->clear_target)
	    wipememory(clipboard->password, FPM_PASSWORD_LEN);
    } else {
	gtk_selection_data_set_text(selection_data, clipboard->user, -1);
	if(ini->clear_target)
	    wipememory(clipboard->user, strlen(clipboard->user));

	if(clipboard->multi_select) {
	    clipboard->no_clear = TRUE;
	    clipboard->is_password = TRUE;
	    fpm_clipboard_set(clipboard);
	}
    }
}

void
fpm_clipboard_clear(GtkClipboard *clip, gpointer data) {
    fpm_clipboard *clipboard = data;

    if(clipboard->no_clear) {
	clipboard->no_clear = FALSE;
    } else {
	g_free(clipboard);
    }

}

void
fpm_clipboard_set(fpm_clipboard *clipboard) {

    static const GtkTargetEntry targets[] = {{ "STRING", 0, 1 }};
    gchar *selection = _("primary selection");
    gchar *selection_type = _("user");
    gchar *dia_str;

    if(clipboard->selection == GDK_SELECTION_CLIPBOARD)
	selection = _("clipboard");

    if(clipboard->is_password)
	selection_type = _("password");

    dia_str = g_strdup_printf(_("Copy %s to %s."), selection_type, selection);

    fpm_statusbar_push(dia_str);

    g_free(dia_str);

    if(!gtk_clipboard_set_with_data(gtk_clipboard_get(clipboard->selection), targets, G_N_ELEMENTS(targets), &fpm_clipboard_get, &fpm_clipboard_clear, clipboard)) {
	printf("Error clip");
    }
}

void
fpm_clipboard_init(fpm_data *data, gint selection, gboolean is_password, gboolean multi_select) {

    GdkAtom SELECTION[] = { GDK_SELECTION_PRIMARY, GDK_SELECTION_CLIPBOARD };
    fpm_clipboard *clipboard;

    clipboard = g_malloc0(sizeof(fpm_clipboard));

    clipboard->selection = SELECTION[selection];
    clipboard->is_password = is_password;
    clipboard->multi_select = multi_select;

    if(!is_password)
	clipboard->user = g_strdup(data->user);

    if(multi_select || is_password)
	clipboard->password = g_strdup(data->password);

    fpm_clipboard_set(clipboard);
}

void
fpm_double_click(fpm_data* data)
{
 switch(ini->dblclick_action) {
    case ACTION_RUN_LAUNCHER:
	if (data != NULL)
	    fpm_jump(data);
	break;
    case ACTION_COPY_USERNAME:
	fpm_clipboard_init(data, ini->copy_target, FALSE, FALSE);
	break;
    case ACTION_COPY_PASSWORD:
	fpm_clipboard_init(data, ini->copy_target, TRUE, FALSE);
	break;
    case ACTION_COPY_BOTH:
	fpm_clipboard_init(data, ini->copy_target, FALSE, TRUE);
	break;
    case ACTION_EDIT_ENTRY:
	fpm_edit_passitem(&gui->edit_window, data);
	break;
 }
}

void
fpm_jump(fpm_data* data)
{
  gchar* cmd;
  fpm_launcher *launcher;
  fpm_launcher *tmp;
  GList *list;

  launcher=NULL;
  list=g_list_first(glb_launcher_list);
  while (list!=NULL)
  {
    tmp=list->data;
    if (!strcmp(tmp->title, data->launcher)) launcher=tmp;
    list=g_list_next(list);
  }

  if(launcher)
  {

    if((launcher->copy_password == 1) && (launcher->copy_user == 1)) {
	fpm_clipboard_init(data, SELECTION_PRIMARY, FALSE, TRUE);
	goto cont;
    }

    if((launcher->copy_password == 2) && (launcher->copy_user == 2)) {
	fpm_clipboard_init(data, SELECTION_CLIPBOARD, FALSE, TRUE);
	goto cont;
    }

  /* Check for items that should be on clipboard or primary selection */
    if(launcher->copy_password == 2)
    {
      fpm_clipboard_init(data, SELECTION_CLIPBOARD, TRUE, FALSE);
    }
    if(launcher->copy_user == 2)
    {
      fpm_clipboard_init(data, SELECTION_CLIPBOARD, FALSE, FALSE);
    }
    if(launcher->copy_user == 1)
    {
      fpm_clipboard_init(data, SELECTION_PRIMARY, FALSE, FALSE);
    }
    if(launcher->copy_password == 1)
    {
      fpm_clipboard_init(data, SELECTION_PRIMARY, TRUE, FALSE);
    }
cont:
    if ( strlen(launcher->cmdline) > 0 )
    {
        cmd = fpm_create_cmd_line(launcher->cmdline, data->arg,
		 data->user, data->password);
	fpm_execute_shell(cmd);
    }
    else
    {
    fpm_message(GTK_WINDOW(gui->main_window), _("This password's launcher command is undefined.\nPlease define it in the edit launcher preference screen."), GTK_MESSAGE_WARNING);
    }
  }
  else
    fpm_message(GTK_WINDOW(gui->main_window), _("This password's launcher is undefined.\nPlease define it in the edit password item screen."), GTK_MESSAGE_WARNING);
}

void
fpm_check_password()
{
  GtkEntry* entry;
  GtkLabel* label;
  GtkComboBox *combo_box;

  char vstring_plain[cipher->blocksize + 1];
  char vstring_plain1[32 + 1] = {0};
  gchar *password;

//  entry = GTK_ENTRY(lookup_widget(glb_win_misc, "entry_password"));
  entry = GTK_ENTRY(lookup_widget(gui->pass_window, "entry_password"));
  password = gtk_editable_get_chars (GTK_EDITABLE(entry), 0, -1);

     gtk_entry_set_text(entry, "");

//  combo_box = GTK_COMBO_BOX(lookup_widget(glb_win_misc, "key_file_combo"));
  combo_box = GTK_COMBO_BOX(lookup_widget(gui->pass_window, "key_file_combo"));

  if(gtk_combo_box_get_active(combo_box)) {
    guchar kf_hash[SHA256_DIGEST_LENGTH + 1] = {0};

    if(fpm_sha256_file(gtk_combo_box_get_active_text(combo_box), kf_hash)) {
//	fpm_message(GTK_WINDOW(glb_win_misc), _("Cannot read key file!"), GTK_MESSAGE_ERROR);
	fpm_message(GTK_WINDOW(gui->pass_window), _("Cannot read key file!"), GTK_MESSAGE_ERROR);
	return;
    }
    password = g_strdup_printf("%s%s", password, kf_hash);
    wipememory(kf_hash, sizeof(kf_hash));
  }

  if(cipher_algo == BLOWFISH) {
    fpm_crypt_init(password);
    fpm_decrypt_field(old_context, vstring_plain, vstring, cipher->blocksize);
  } else if(cipher_algo == AES256) {
    if(!glb_need_decrypt) {
	fpm_launcher_remove_all();
	fpm_clear_list();
	passfile_load(glb_filename);
    }
    fpm_crypt_init(password);
    fpm_decrypt_launchers();
    fpm_decrypt_all();
    glb_need_decrypt = FALSE;
    gchar* vhash;
    vhash = g_malloc0(cipher->hash_len + 1);
    fpm_sha256_fpm_data(vhash);

    fpm_decrypt_field(old_context, vstring_plain1, vstring, cipher->hash_len);
    if(strcmp(vstring_plain1, vhash) == 0) {
	strcpy(vstring_plain,"FIGARO");
    }
    g_free(vhash);
  }
  if (strcmp(vstring_plain, "FIGARO"))
  {
  wipememory(password, strlen(password));
  g_free(password);
    gtk_editable_select_region(GTK_EDITABLE(entry), 0, -1);
//    label=GTK_LABEL(lookup_widget(glb_win_misc, "label_password_message"));
    label=GTK_LABEL(lookup_widget(gui->pass_window, "label_password_message"));
    gtk_widget_show(GTK_WIDGET(label));
    gtk_label_set_text(label, _("Password incorrect.  Please try again."));
    return;
  }

  if (glb_need_decrypt)
  {
    fpm_decrypt_all();
    glb_need_decrypt=FALSE;
  }

    if(cipher_algo == BLOWFISH) {
//      if (fpm_question(GTK_WINDOW(glb_win_misc), _("Old file format detected.\n\nDo you want change password file to new more secure cipher?\n\nPlease take note that it will not be possible to continue open this password file in the earlier version pepm."))
      if (fpm_question(GTK_WINDOW(gui->pass_window), _("Old file format detected.\n\nDo you want change password file to new more secure cipher?\n\nPlease take note that it will not be possible to continue open this password file in the earlier version pepm."))
	 == GTK_RESPONSE_YES) {

//	gtk_widget_destroy(glb_win_misc);
	gtk_widget_hide(gui->pass_window);
	glb_win_misc = GTK_WIDGET(create_window_entropy());
	gtk_widget_show(glb_win_misc);
	while (gtk_events_pending()) gtk_main_iteration();

	fpm_file_convert(glb_filename, password);

//	gtk_widget_hide(glb_win_misc);
	gtk_widget_destroy(glb_win_misc);

	passfile_load(glb_filename);
	fpm_crypt_init(password);
	fpm_decrypt_launchers();
	fpm_decrypt_all();
	fpm_message(NULL, _("Conversion of password file is done!"), GTK_MESSAGE_INFO);
    }
  }

  wipememory(password, strlen(password));
  g_free(password);

  if(ini->startup_category_active)
    state->category = ini->startup_category;
  else
    state->category = ini->last_category;

  fpm_clist_populate_cat_list();

  gtk_window_resize (GTK_WINDOW(gui->main_window), ini->main_width, ini->main_height);
  if((ini->main_x != -1) && (ini->main_y != -1) && !ini->dont_remember_position)
    gtk_window_move(GTK_WINDOW(gui->main_window), ini->main_x, ini->main_y);

  if(ini->enable_tray_icon)
    fpm_tray_icon();

  gtk_window_set_skip_taskbar_hint(GTK_WINDOW(gui->main_window), FALSE);
  gtk_window_set_skip_pager_hint(GTK_WINDOW(gui->main_window), FALSE);
  gtk_window_present(GTK_WINDOW(gui->main_window));
  gtk_window_deiconify(GTK_WINDOW(gui->main_window));

//  gtk_widget_destroy(glb_win_misc);
  gtk_widget_hide(gui->pass_window);

  state->locked = FALSE;
  state->minimized = FALSE;

  while (gtk_events_pending()) gtk_main_iteration();

  if(ini->startup_search)
    on_find_activate(NULL, NULL);

}


void fpm_set_password()
{
  GtkEntry *entry;
  GtkComboBox *combo_box;
  gchar *pw1, *pw2;

  entry = GTK_ENTRY(lookup_widget(glb_win_misc, "entry_cpw1"));
  pw1 = gtk_editable_get_chars (GTK_EDITABLE(entry), 0, -1);
  gtk_entry_set_text(entry, "");
  entry = GTK_ENTRY(lookup_widget(glb_win_misc, "entry_cpw2"));
  pw2 = gtk_editable_get_chars (GTK_EDITABLE(entry), 0, -1);
  gtk_entry_set_text(entry, "");

  if (strcmp(pw1, pw2))
  {
    fpm_message(GTK_WINDOW(glb_win_misc), _("Sorry, the passwords do not match."), GTK_MESSAGE_ERROR);
    return;
  }
  if (strlen(pw1)<4)
  {
    fpm_message(GTK_WINDOW(glb_win_misc), _("Sorry, your password is too short."), GTK_MESSAGE_ERROR);
    return;
  }

  combo_box = GTK_COMBO_BOX(lookup_widget(glb_win_misc, "key_file_combo"));
  if(gtk_combo_box_get_active(combo_box)) {
    guchar kf_hash[SHA256_DIGEST_LENGTH + 1] = {0};

    if(fpm_sha256_file(gtk_combo_box_get_active_text(combo_box), kf_hash)) {
	fpm_message(GTK_WINDOW(glb_win_misc), _("Cannot read key file!"), GTK_MESSAGE_ERROR);
	return;
    }
    pw1 = g_strdup_printf("%s%s", pw1, kf_hash);
    wipememory(kf_hash, sizeof(kf_hash));
  }

  if(old_context == NULL)
  {
    /* This is the first time we are running the app.*/
    state->category=g_strdup(FPM_ALL_CAT_MSG);

    fpm_crypt_init(pw1);
//    state->dirty = TRUE;
    fpm_dirty(TRUE);
    gtk_widget_show(gui->main_window);
    gtk_widget_set_sensitive(gui->main_window, TRUE);

    fpm_clist_populate_cat_list();
  }
  else
  {
    /* We are chaning the password.*/
    fpm_crypt_set_password(pw1);

//    gtk_widget_destroy(glb_win_misc);

    passfile_save(glb_filename);
  }

  gtk_widget_destroy(glb_win_misc);

  wipememory(pw1, strlen(pw1));
  wipememory(pw2, strlen(pw2));
  g_free(pw1);
  g_free(pw2);
}

void fpm_clear_list()
{
  GList *list;
  GtkTreeModel *list_store;

  list = g_list_first(glb_pass_list);
  while (list != NULL)
  {
    g_free(list->data);
    list = g_list_next(list);
  }
  g_list_free(glb_pass_list);
  glb_pass_list = NULL;

  list_store = gtk_tree_view_get_model(gui->main_clist);
  gtk_list_store_clear(GTK_LIST_STORE(list_store));
}

void fpm_init_launchers()
{
  GList * list;
  fpm_launcher* launcher;

  list = NULL;

  launcher = g_malloc0(sizeof(fpm_launcher));
  launcher->title=g_strdup("Web");
  launcher->cmdline=g_strdup("xdg-open \"$a\"");
  launcher->copy_user = 2;
  launcher->copy_password = 1;
  list = g_list_append(list, launcher);

  launcher = g_malloc0(sizeof(fpm_launcher));
  launcher->title=g_strdup("ssh");
  launcher->cmdline=g_strdup("gnome-terminal -e 'ssh $a -l $u'");
  launcher->copy_user = 0;
  launcher->copy_password = 1;
  list = g_list_append(list, launcher);

  launcher = g_malloc0(sizeof(fpm_launcher));
  launcher->title=g_strdup("Generic Command");
  launcher->cmdline=g_strdup("$a");
  launcher->copy_user = 0;
  launcher->copy_password = 0;
  list = g_list_append(list, launcher);

  glb_launcher_list=list;

  fpm_create_launcher_string_list();
}

void fpm_create_launcher_string_list()
{
  GList * list;
  fpm_launcher* launcher;

  glb_launcher_string_list=NULL;
  list=g_list_first(glb_launcher_list);
  glb_launcher_string_list = g_list_append(glb_launcher_string_list, g_strdup(C_("key_file","<NONE>")));
  while (list!= NULL)
  {
    launcher=list->data;
    glb_launcher_string_list=g_list_append(glb_launcher_string_list, g_strdup(launcher->title));
    list=g_list_next(list);
  }
}

gchar* fpm_create_cmd_line(gchar* cmd, gchar* arg, gchar* user, gchar* pass)
{
  gchar* result;
  gchar* tmp;
  gchar** cmd_arr;
  gint num, i;
  gchar* cleartext;

  cmd_arr = g_strsplit(cmd, "$", 0);
  num=sizeof(cmd_arr)/sizeof(char);
  result=g_strdup(cmd_arr[0]);
  i=1;
  while (cmd_arr[i]!=NULL)
  {
    tmp=result;
    if(cmd_arr[i][0]=='a')
      result=g_strconcat(tmp, arg, cmd_arr[i]+1, NULL);
    else if(cmd_arr[i][0]=='u')
      result=g_strconcat(tmp, user, cmd_arr[i]+1, NULL);
    else if(cmd_arr[i][0]=='p')
    {
      cleartext=fpm_decrypt_field_var(old_context, pass);
      result=g_strconcat(tmp, cleartext, cmd_arr[i]+1, NULL);
      wipememory(cleartext, strlen(cleartext));
      g_free(cleartext);
    }
    else
      result=g_strconcat(tmp, "$", cmd_arr[i], NULL);
    wipememory(tmp, strlen(tmp));
    g_free(tmp);
    i++;
  }
  g_strfreev(cmd_arr);

  return(result);
}

void fpm_message(GtkWindow* win, gchar* message, GtkMessageType message_type)
{
    GtkWidget *dw;
    dw = gtk_message_dialog_new (GTK_WINDOW(win),
                              GTK_DIALOG_DESTROY_WITH_PARENT,
                              message_type,
                              GTK_BUTTONS_CLOSE,
                              "%s",
                              message);
    gtk_window_set_title (GTK_WINDOW(dw), _("Information"));
    gtk_dialog_run (GTK_DIALOG (dw));
    gtk_widget_destroy (dw);
}

void fpm_ini_load()
{
    GKeyFile *keyfile;
    GError *error = NULL;
    gsize count;
    gint i, *columns;

    ini = g_malloc0(sizeof(fpm_ini));

    gchar *fpm_ini_file = g_build_filename (g_get_home_dir(), FPM_DIR, "fpm.ini", NULL);

    keyfile = g_key_file_new ();
    if (!g_key_file_load_from_file (keyfile, fpm_ini_file, G_KEY_FILE_NONE, NULL))
    {
	/* No ini file found -> set defaults and save */
	ini->save_on_add = TRUE;
	ini->save_on_change = TRUE;
	ini->save_on_delete = FALSE;
	ini->save_on_quit = FALSE;
	ini->create_backup = TRUE;
	ini->number_backup_files = 5;
	ini->main_x = -1;
	ini->main_y = -1;
	ini->main_width = 500;
	ini->main_height = 350;
	ini->toolbar_visible = TRUE;
	ini->toolbar_icon_style = GTK_TOOLBAR_BOTH;
	ini->last_category = FPM_ALL_CAT_MSG;
	ini->search_in_title = TRUE;
	ini->search_in_url = FALSE;
	ini->search_in_user = FALSE;
	ini->search_in_notes = FALSE;
	ini->search_match_case = FALSE;
	ini->search_limit_category = TRUE;
	ini->enable_tray_icon = FALSE;
	ini->tr_always_visible = FALSE;
	ini->tr_auto_hide = FALSE;
	ini->tr_auto_hide_minutes = 15;
	ini->tr_auto_lock = FALSE;
	ini->tr_auto_lock_minutes = 15;

	ini->startup_category_active = FALSE;
	ini->startup_category = FPM_DEFAULT_CAT_MSG;
	ini->after_unhide_set_startup_category = FALSE;
	state->category = FPM_ALL_CAT_MSG;

	ini->copy_target = SELECTION_PRIMARY;
	ini->clear_target = TRUE;

	ini->dont_remember_position = FALSE;
	ini->dblclick_action = ACTION_RUN_LAUNCHER;
	ini->startup_search = FALSE;

	fpm_ini_save();
    } else {
	ini->save_on_add = g_key_file_get_boolean (keyfile, "general", "save_on_add", NULL);
        ini->save_on_delete = g_key_file_get_boolean (keyfile, "general", "save_on_delete", NULL);
        ini->save_on_quit = g_key_file_get_boolean (keyfile, "general", "save_on_quit", NULL);
        ini->save_on_change = g_key_file_get_boolean (keyfile, "general", "save_on_change", NULL);
        ini->create_backup = g_key_file_get_boolean (keyfile, "general", "create_backup", NULL);
        ini->number_backup_files = g_key_file_get_integer (keyfile, "general", "number_backup_files", NULL);

        ini->last_category = g_key_file_get_string (keyfile, "general", "last_category", &error);
	if (error != NULL)
	    ini->last_category = FPM_ALL_CAT_MSG;

	error = NULL;
	ini->main_x = g_key_file_get_integer (keyfile, "general", "main_x", &error);
	if (error != NULL)
	    ini->main_x = -1;
	error = NULL;
	ini->main_y = g_key_file_get_integer (keyfile, "general", "main_y", &error);
	if (error != NULL)
	    ini->main_y = -1;

	error = NULL;
	ini->main_width = g_key_file_get_integer (keyfile, "general", "main_width", &error);
	if (error != NULL)
	    ini->main_width = 500;

	error = NULL;
	ini->main_height = g_key_file_get_integer (keyfile, "general", "main_height", &error);
	if (error != NULL)
	    ini->main_height = 350;

	error = NULL;
	columns = g_key_file_get_integer_list (keyfile, "general", "columns_width", &count, &error);
	if (error != NULL || count != FPM_NUM_COLS || columns == NULL) {
	    for (i = 0; i < FPM_NUM_COLS; i++)
		ini->columns_width[i] = 150;
	} else {
	    memcpy(ini->columns_width, columns, sizeof(ini->columns_width));
	    g_free(columns);
	}

	error = NULL;
	columns = g_key_file_get_integer_list (keyfile, "general", "columns_order", &count, &error);
	if (error != NULL || columns == NULL) {
	    columns_order = g_list_append (columns_order, GINT_TO_POINTER (FPM_TITLE));
	    columns_order = g_list_append (columns_order, GINT_TO_POINTER (FPM_URL));
	    columns_order = g_list_append (columns_order, GINT_TO_POINTER (FPM_USER));
	} else {
	    for (i = 0; i < count; i++)
		columns_order = g_list_append (columns_order, GINT_TO_POINTER (columns[i]));
	    g_free(columns);
	}

        ini->dont_remember_position = g_key_file_get_boolean (keyfile, "general", "dont_remember_position", NULL);

	error = NULL;
	ini->search_in_title = g_key_file_get_boolean (keyfile, "search", "search_in_title", &error);
	if (error != NULL)
	    ini->search_in_title = TRUE;

	ini->search_in_url = g_key_file_get_boolean (keyfile, "search", "search_in_url", NULL);
	ini->search_in_user = g_key_file_get_boolean (keyfile, "search", "search_in_user", NULL);
	ini->search_in_notes = g_key_file_get_boolean (keyfile, "search", "search_in_notes", NULL);
	ini->search_match_case = g_key_file_get_boolean (keyfile, "search", "search_match_case", NULL);

	error = NULL;
	ini->search_limit_category = g_key_file_get_boolean (keyfile, "search", "search_limit_category", &error);
	if (error != NULL)
	    ini->search_limit_category = TRUE;

	error = NULL;
	ini->enable_tray_icon = g_key_file_get_boolean (keyfile, "tray_icon", "enable_tray_icon", &error);
	if (error != NULL)
	    ini->enable_tray_icon = FALSE;

	ini->tr_always_visible = g_key_file_get_boolean (keyfile, "tray_icon", "tr_always_visible", NULL);
	ini->tr_auto_hide = g_key_file_get_boolean (keyfile, "tray_icon", "tr_auto_hide", NULL);

	error = NULL;
	ini->tr_auto_hide_minutes = g_key_file_get_integer (keyfile, "tray_icon", "tr_auto_hide_minutes", &error);
	if (error != NULL)
	    ini->tr_auto_hide_minutes = 15;

	ini->tr_auto_lock = g_key_file_get_boolean (keyfile, "tray_icon", "tr_auto_lock", NULL);

	error = NULL;
	ini->tr_auto_lock_minutes = g_key_file_get_integer (keyfile, "tray_icon", "tr_auto_lock_minutes", &error);
	if (error != NULL)
	    ini->tr_auto_lock_minutes = 15;

	error = NULL;
	ini->startup_category_active = g_key_file_get_boolean (keyfile, "general", "startup_category_active", &error);
	if (error != NULL)
		ini->startup_category_active = FALSE;

	error = NULL;
	ini->startup_category = g_key_file_get_string (keyfile, "general", "startup_category", &error);
	if (error != NULL)
		ini->startup_category = FPM_DEFAULT_CAT_MSG;

	error = NULL;
	ini->toolbar_visible = g_key_file_get_integer(keyfile, "general", "toolbar_visible", &error);
	if (error != NULL)
		ini->toolbar_visible = TRUE;

	error = NULL;
	ini->toolbar_icon_style = g_key_file_get_integer(keyfile, "general", "toolbar_icon_style", &error);
	if (error != NULL)
		ini->toolbar_icon_style = GTK_TOOLBAR_BOTH;

	error = NULL;
	ini->copy_target = g_key_file_get_integer(keyfile, "general", "copy_target", &error);
	if (error != NULL)
		ini->copy_target = SELECTION_PRIMARY;

	error = NULL;
	ini->clear_target = g_key_file_get_boolean(keyfile, "general", "clear_target", &error);
	if (error != NULL)
		ini->clear_target = TRUE;

	error = NULL;
	ini->after_unhide_set_startup_category = g_key_file_get_boolean(keyfile, "general", "after_unhide_set_startup_category", &error);
	if (error != NULL)
		ini->after_unhide_set_startup_category = FALSE;

	error = NULL;
	ini->dblclick_action = g_key_file_get_integer(keyfile, "general", "dblclick_action", &error);
	if (error != NULL)
		ini->dblclick_action = ACTION_RUN_LAUNCHER;

        ini->startup_search = g_key_file_get_boolean (keyfile, "general", "startup_search", NULL);

	g_free(fpm_ini_file);

    }

    g_key_file_free (keyfile);
}

void fpm_ini_save()
{
    GKeyFile *keyfile;
    gint x, y, width, height;
    gint i = 0;
    GList *columns;
    gint columns_order_tmp[FPM_NUM_COLS];

    if (gui->main_window == NULL) {
	x = -1;
	y = -1;
	width = 500;
	height = 350;
	for (i = 0; i < FPM_NUM_COLS; i++)
	    ini->columns_width[i] = 150;

        columns_order = g_list_append (columns_order, GINT_TO_POINTER (FPM_TITLE));
        columns_order = g_list_append (columns_order, GINT_TO_POINTER (FPM_URL));
        columns_order = g_list_append (columns_order, GINT_TO_POINTER (FPM_USER));

    } else {
	gtk_window_get_size (GTK_WINDOW(gui->main_window), &width, &height);
	gtk_window_get_position(GTK_WINDOW(gui->main_window), &x, &y);

	columns = gtk_tree_view_get_columns(gui->main_clist);
	while (columns != NULL) {
	    ini->columns_width[i] = gtk_tree_view_column_get_width (columns->data);
	    columns = g_list_next(columns);
	    i++;
	}
	g_list_free(columns);

	i = 0;
	columns = g_list_first(columns_order);
	while(columns != NULL) {
	    columns_order_tmp[i] = GPOINTER_TO_INT(columns->data);
	    columns = g_list_next(columns);
	    i++;
	}

    }

    gchar *fpm_ini_file = g_build_filename (g_get_home_dir(), FPM_DIR, "fpm.ini", NULL);

    keyfile = g_key_file_new ();

    g_key_file_set_boolean (keyfile, "general", "save_on_add", ini->save_on_add);
    g_key_file_set_boolean (keyfile, "general", "save_on_change", ini->save_on_change);
    g_key_file_set_boolean (keyfile, "general", "save_on_delete", ini->save_on_delete);
    g_key_file_set_boolean (keyfile, "general", "save_on_quit", ini->save_on_quit);
    g_key_file_set_boolean (keyfile, "general", "create_backup", ini->create_backup);
    g_key_file_set_integer (keyfile, "general", "number_backup_files", ini->number_backup_files);
    g_key_file_set_string (keyfile, "general", "last_category", state->category);
    g_key_file_set_integer (keyfile, "general", "main_x", x);
    g_key_file_set_integer (keyfile, "general", "main_y", y);
    g_key_file_set_integer (keyfile, "general", "main_width", width);
    g_key_file_set_integer (keyfile, "general", "main_height", height);
    g_key_file_set_integer_list (keyfile, "general", "columns_width", ini->columns_width, FPM_NUM_COLS);
    g_key_file_set_integer_list (keyfile, "general", "columns_order", columns_order_tmp, i);
    g_key_file_set_boolean (keyfile, "general", "dont_remember_position", ini->dont_remember_position);
    g_key_file_set_integer (keyfile, "general", "dblclick_action", ini->dblclick_action);

    g_key_file_set_boolean (keyfile, "search", "search_in_title", ini->search_in_title);
    g_key_file_set_boolean (keyfile, "search", "search_in_url", ini->search_in_url);
    g_key_file_set_boolean (keyfile, "search", "search_in_user", ini->search_in_user);
    g_key_file_set_boolean (keyfile, "search", "search_in_notes", ini->search_in_notes);
    g_key_file_set_boolean (keyfile, "search", "search_match_case", ini->search_match_case);
    g_key_file_set_boolean (keyfile, "search", "search_limit_category", ini->search_limit_category);

    g_key_file_set_boolean (keyfile, "tray_icon", "enable_tray_icon", ini->enable_tray_icon);
    g_key_file_set_boolean (keyfile, "tray_icon", "tr_always_visible", ini->tr_always_visible);
    g_key_file_set_boolean (keyfile, "tray_icon", "tr_auto_hide", ini->tr_auto_hide);
    g_key_file_set_integer (keyfile, "tray_icon", "tr_auto_hide_minutes", ini->tr_auto_hide_minutes);
    g_key_file_set_boolean (keyfile, "tray_icon", "tr_auto_lock", ini->tr_auto_lock);
    g_key_file_set_integer (keyfile, "tray_icon", "tr_auto_lock_minutes", ini->tr_auto_lock_minutes);

    g_key_file_set_boolean (keyfile, "general", "startup_category_active", ini->startup_category_active);
    g_key_file_set_string (keyfile, "general", "startup_category", ini->startup_category);
    g_key_file_set_boolean (keyfile, "general", "after_unhide_set_startup_category", ini->after_unhide_set_startup_category);

    g_key_file_set_integer (keyfile, "general", "toolbar_visible", ini->toolbar_visible);
    g_key_file_set_integer (keyfile, "general", "toolbar_icon_style", ini->toolbar_icon_style);

    g_key_file_set_integer (keyfile, "general", "copy_target", ini->copy_target);
    g_key_file_set_boolean (keyfile, "general", "clear_target", ini->clear_target);

    g_key_file_set_boolean (keyfile, "general", "startup_search", ini->startup_search);

    gchar *save_data = g_key_file_to_data (keyfile, NULL, NULL);

    g_file_set_contents (fpm_ini_file, save_data, -1, NULL);

    g_free(fpm_ini_file);
    g_free(save_data);
    g_key_file_free (keyfile);
}

void fpm_search(gchar *search_text, gboolean select_first)
{
    fpm_data *data;
    GList *list;
    GtkTreeIter iter;
    GtkTreeModel *list_store;
    gchar *title, *url, *user, *notes;

    if (!ini->search_match_case)
	search_text = g_utf8_casefold(search_text, -1);

    list_store = gtk_tree_view_get_model (gui->main_clist);
    gtk_list_store_clear(GTK_LIST_STORE(list_store));

    list = g_list_first(glb_pass_list);
    while (list != NULL)
    {
	data = list->data;
	title = data->title;
	url = data->arg;
	user = data->user;
	notes = data->notes;

        if ((!strcmp(state->category, data->category) && ini->search_limit_category)
	    || ((!strcmp(state->category, FPM_ALL_CAT_MSG)
	    || !ini->search_limit_category)) 
	    || (!strcmp(state->category, FPM_NONE_CAT_MSG) && !strlen(data->category))
	    || (!strcmp(state->category, FPM_DEFAULT_CAT_MSG) && data->default_list)
    	    )
	{

	    if (!ini->search_match_case) {
		title = g_utf8_casefold(title, -1);
		url = g_utf8_casefold(url, -1);
		user = g_utf8_casefold(user, -1);
		notes = g_utf8_casefold(notes, -1);
	    }

	    if ((strstr(title, search_text) && ini->search_in_title)
		|| (strstr(url, search_text) && ini->search_in_url)
		|| (strstr(user, search_text) && ini->search_in_user)
		|| (strstr(notes, search_text) && ini->search_in_notes))
	    {

        	gtk_list_store_append (GTK_LIST_STORE(list_store), &iter);

		fpm_clist_set_data (data, list_store, iter);
	    }
	}
/*	if (!ini->search_match_case) {
		g_free(title);
		g_free(url);
		g_free(user);
		g_free(notes);
	} */
	list = g_list_next(list);
    }

//	if (!ini->search_match_case)
//		g_free(search_text);

    if (select_first)
		gtk_widget_grab_focus(GTK_WIDGET(gui->main_clist));

}

void fpm_execute_shell(gchar *cmd)
{
    gchar *execute;
    int ret;

    execute = g_strjoin(NULL, cmd, " &", NULL);
    ret = system(execute);
}

void fpm_statusbar_push(gchar *message)
{
    GtkStatusbar *statusbar;

    statusbar = GTK_STATUSBAR(lookup_widget (gui->main_window, "statusbar"));
    gtk_statusbar_pop(statusbar, 1);
    gtk_statusbar_push(statusbar, 1, message);
}

gboolean fpm_auto_hide() {

    fpm_tr_toggle_main_window(TRUE);

    return FALSE;
}

void fpm_tray_icon() {

    if (gui->tray_icon == NULL) {
	gui->tray_icon = gtk_status_icon_new();

	gtk_status_icon_set_visible(gui->tray_icon, FALSE);
	g_signal_connect(G_OBJECT(gui->tray_icon), "activate", 
			G_CALLBACK(tray_icon_on_click), NULL);
	g_signal_connect(G_OBJECT(gui->tray_icon), "popup-menu",
			G_CALLBACK(tray_icon_on_menu), NULL);
	gtk_status_icon_set_from_file (gui->tray_icon, PACKAGE_PIXMAP_DIR "/logo.png");
    }

    gtk_status_icon_set_tooltip_text(gui->tray_icon, _("Privacy Enhanced Password Manager"));
    gtk_status_icon_set_visible(gui->tray_icon, ini->tr_always_visible);
    gtk_widget_show (GTK_WIDGET(lookup_widget (gui->main_window , "hide_to_tray")));
}

gboolean fpm_hide_main_window() {
    gtk_widget_hide(gui->main_window);
    return FALSE;
}

void fpm_tr_toggle_main_window(gboolean force_hide) {

    static int x, y;
    static GdkScreen *screen;

    GdkRectangle  bounds;
    gulong        data[4];
    Display      *dpy;
    GdkWindow    *gdk_window;


    if (state->minimized && !force_hide) {
	gtk_window_set_screen(GTK_WINDOW(gui->main_window), screen);
	gtk_window_move(GTK_WINDOW(gui->main_window), x, y);

	gtk_widget_show(gui->main_window);
	gtk_window_deiconify(GTK_WINDOW(gui->main_window));
	gtk_window_present(GTK_WINDOW(gui->main_window));

	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(gui->main_window), FALSE);
	gtk_window_set_skip_pager_hint(GTK_WINDOW(gui->main_window), FALSE);

	if (!ini->tr_always_visible)
	    gtk_status_icon_set_visible(gui->tray_icon, FALSE);

	state->minimized = FALSE;

    } else {

	gtk_status_icon_set_visible(gui->tray_icon, TRUE);
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(gui->main_window), TRUE);
	gtk_window_set_skip_pager_hint(GTK_WINDOW(gui->main_window), TRUE);

	if(!ini->tr_always_visible)
	    usleep(50000);

	g_timeout_add(50, fpm_hide_main_window, NULL);

	gtk_window_get_position(GTK_WINDOW(gui->main_window), &x, &y);
	screen = gtk_window_get_screen(GTK_WINDOW(gui->main_window));

	gtk_status_icon_get_geometry(gui->tray_icon, NULL, &bounds, NULL );
	gdk_window = gui->main_window->window;
	dpy = gdk_x11_drawable_get_xdisplay (gdk_window);

	data[0] = bounds.x;
	data[1] = bounds.y;
	data[2] = bounds.width;
	data[3] = bounds.height;

	XChangeProperty (dpy,
			GDK_WINDOW_XID (gdk_window),
			gdk_x11_get_xatom_by_name_for_display (gdk_drawable_get_display (gdk_window),
			"_NET_WM_ICON_GEOMETRY"),
			XA_CARDINAL, 32, PropModeReplace,
			(guchar*)&data, 4);

	gtk_window_iconify(GTK_WINDOW(gui->main_window));

	if (ini->tr_auto_lock && !state->locked) {
	    g_timeout_add_seconds(ini->tr_auto_lock_minutes*60, (GSourceFunc) fpm_lock, NULL);
	}
	state->minimized = TRUE;
    }
}


void fpm_tr_cleanup() {
    if (gui->tray_icon) {
	gtk_status_icon_set_visible (gui->tray_icon, FALSE);
	gui->tray_icon = (g_object_unref (gui->tray_icon), NULL);
    }
}

gboolean fpm_lock() {
    if (!state->minimized)
	fpm_tr_toggle_main_window(TRUE);

    while (gtk_events_pending()) gtk_main_iteration();

    if(GTK_IS_WIDGET(glb_win_misc))
	gtk_widget_destroy(glb_win_misc);
    if(gui->edit_window != NULL)
	gtk_widget_destroy(gui->edit_window);
//    g_signal_handlers_disconnect_by_func(gui->tray_icon, G_CALLBACK (tray_icon_on_menu), NULL);

    gtk_status_icon_set_tooltip_text(gui->tray_icon, _("Figaro's Password Manager 2 - locked"));

    fpm_crypt_init("");
    fpm_clear_list();
//    fpm_ini_save();
//    unlock_fpm_file1();

    state->locked = TRUE;

    return FALSE;
}

/* This function is called when focus out from main window and we
   need know if focus get another pepm window (TRUE) or no (FALSE). */
gboolean fpm_window_check() {
    GList* list;
    list = gtk_window_list_toplevels();
    while (list) {
	if ((gtk_window_get_title(list->data) != NULL) && (list->data != gui->main_window) && (list->data != gui->pass_window))
	    return TRUE;
	list = g_list_next(list); 
    }
    return FALSE;
}

void fpm_toolbar_style() {

    GtkWidget *menuitem;
    GtkToolbar *toolbar;

	toolbar = GTK_TOOLBAR(lookup_widget(gui->main_window, "toolbar1"));

	if(gui->toolbar_menu == NULL) {
		gui->toolbar_menu = GTK_MENU(create_toolbar_menu());

    	g_signal_connect ((gpointer) lookup_widget(GTK_WIDGET(gui->toolbar_menu) ,"icons_only1"), "activate",
                    	G_CALLBACK (on_icons_only1_activate),
                    	toolbar);
    	g_signal_connect ((gpointer) lookup_widget(GTK_WIDGET(gui->toolbar_menu) ,"text_only1"), "activate",
                    	G_CALLBACK (on_text_only1_activate),
                    	toolbar);
    	g_signal_connect ((gpointer) lookup_widget(GTK_WIDGET(gui->toolbar_menu) ,"icons_and_text1"), "activate",
                    	G_CALLBACK (on_icons_and_text1_activate),
                    	toolbar);
	}

	switch (ini->toolbar_icon_style) {
		case 0:
				menuitem = lookup_widget(GTK_WIDGET(gui->toolbar_menu), "icons_only1");
				break;
		case 1:
				menuitem = lookup_widget(GTK_WIDGET(gui->toolbar_menu), "text_only1");
				break;
		default:
				menuitem = lookup_widget(GTK_WIDGET(gui->toolbar_menu), "icons_and_text1");
	}

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem), TRUE);

    if(!ini->toolbar_visible) {
	gtk_widget_hide(lookup_widget(gui->main_window, "toolbar1"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(gui->main_window, "toolbar2")), FALSE);
    }
}

gint fpm_question(GtkWindow *win, gchar *message)
{
    GtkWidget *dw;
    dw = gtk_message_dialog_new (GTK_WINDOW(win),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
			"%s",
			message);
    gtk_window_set_title (GTK_WINDOW(dw), _("Question"));

    gint result = gtk_dialog_run (GTK_DIALOG (dw));
    if(result != GTK_RESPONSE_NONE) gtk_widget_destroy (dw);

    return(result);
}

void fpm_middle_dblclick(fpm_data *data) {
    fpm_clipboard_init(data, ini->copy_target, FALSE, TRUE);
}

void fpm_copy_target_combo(GtkComboBox *combo_box, gint selection) {
    gtk_combo_box_append_text (combo_box, _("Primary selection"));
    gtk_combo_box_append_text (combo_box, _("Clipboard"));
    gtk_combo_box_set_active (combo_box, selection);
}

gboolean fpm_valid_edit_data() {
  if (glb_edit_data == NULL) {
    fpm_message(GTK_WINDOW(gui->main_window), _("\nPlease select a row."), GTK_MESSAGE_INFO);
    return(FALSE);
  }
  return(TRUE);
}

gboolean fpm_is_launcher_in_category(fpm_launcher *launcher, gchar *category) {
    GList *list;
    fpm_data *data;

    list = g_list_first(glb_pass_list);
    while (list) {
	data = list->data;
	if(!strcmp(launcher->title, data->launcher) && (!strcmp(data->category, category) || !strcmp(category, FPM_ALL_CAT_MSG)))
	    return(TRUE);

	list = g_list_next(list);
    }
    return(FALSE);
}

void fpm_switch_view_category(gchar *category) {
    GList* list;
    gint i = 0;
    gchar *dia_str;

    list = g_list_first(glb_cat_string_list);
    while(list != NULL) {
	if(!g_utf8_collate(category, list->data)) {
	    gtk_combo_box_set_active(GTK_COMBO_BOX(lookup_widget(gui->main_window, "optionmenu_category")), i);
	    break;
	}
	i++;
	list = g_list_next(list);
    }
    fpm_clist_create_view(ini->startup_category);
    dia_str = g_strdup_printf(_("Passwords in category: %d"), glb_num_row);
    fpm_statusbar_push(dia_str);
    g_free(dia_str);
}

void fpm_view_modify_column(gint type, gboolean active) {

    if(active)
	columns_order = g_list_append (columns_order, GINT_TO_POINTER(type));
    else
	columns_order = g_list_remove (columns_order, GINT_TO_POINTER(type));
}

void fpm_dirty(gboolean active) {
    state->dirty = active;
    gtk_widget_set_sensitive (lookup_widget(gui->main_window, "button_save"), active);
    gtk_widget_set_sensitive (lookup_widget(gui->main_window, "save1"), active);
}

void fpm_sensitive_menu(gboolean active) {
    gtk_widget_set_sensitive (lookup_widget(gui->main_window, "button_jump"), active);
    gtk_widget_set_sensitive (lookup_widget(gui->main_window, "button_edit"), active);
    gtk_widget_set_sensitive (lookup_widget(gui->main_window, "button_user"), active);
    gtk_widget_set_sensitive (lookup_widget(gui->main_window, "button_pass"), active);
    gtk_widget_set_sensitive (lookup_widget(gui->main_window, "copy_pass"), active);
    gtk_widget_set_sensitive (lookup_widget(gui->main_window, "copy_user"), active);
    gtk_widget_set_sensitive (lookup_widget(gui->main_window, "copy_both1"), active);
    gtk_widget_set_sensitive (lookup_widget(gui->main_window, "edit_item1"), active);
    gtk_widget_set_sensitive (lookup_widget(gui->main_window, "clone_item1"), active);
    gtk_widget_set_sensitive (lookup_widget(gui->main_window, "item_delete"), active);
}
