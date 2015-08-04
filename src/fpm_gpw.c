/* FIGARO'S PASSWORD MANAGER (FPM)
 * Copyright (C) 2000 John Conneely
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
 *
 * fpm-gpw.c -- Password generation routines
 */

#include <gtk/gtk.h>
#include <math.h>
#include <string.h>

#include "fpm_gpw.h"
#include "fpm.h"
#include "support.h"

#define GPW_CHAR_BUFFER 256

static gint gpw_len;
static gboolean gpw_use_lcase;
static gboolean gpw_use_ucase;
static gboolean gpw_use_num;
static gboolean gpw_use_sym;
static gboolean gpw_no_amb;

static gchar gpw_chars[GPW_CHAR_BUFFER];
static gint gpw_num_chars;




void
fpm_gpw_set_options(	gint 		pw_len,
			gboolean 	use_lcase,
			gboolean 	use_ucase,
			gboolean	use_num,
			gboolean	use_sym,
			gboolean	no_amb)
/* This routine sets what types of characters can be used in
 * generated passwords */
{
  const gchar chars_lcase1[] = "abcdefghijkmnpqrstuvwxyz";
  const gchar chars_lcase2[] = "lo";
  const gchar chars_ucase1[] = "ABCDEFHJKLMNPQRSTUVWXYZ";
  const gchar chars_ucase2[] = "IOG";
  const gchar chars_num1[] = "2345789";
  const gchar chars_num2[] = "016";
  const gchar chars_sym1[] = "!@#$%&*()+=/{}[]:;<>";
  const gchar chars_sym2[] = "_-|,.`'~^";


  gpw_len=pw_len;
  gpw_use_lcase=use_lcase;
  gpw_use_ucase=use_ucase;
  gpw_use_num=use_num;
  gpw_use_sym=use_sym;
  gpw_no_amb=no_amb;

  strncpy(gpw_chars, "", GPW_CHAR_BUFFER);
  if (gpw_use_lcase) strncat(gpw_chars, chars_lcase1, GPW_CHAR_BUFFER-1);
  if (gpw_use_ucase) strncat(gpw_chars, chars_ucase1, GPW_CHAR_BUFFER-1);
  if (gpw_use_num) strncat(gpw_chars, chars_num1, GPW_CHAR_BUFFER-1);
  if (gpw_use_sym) strncat(gpw_chars, chars_sym1, GPW_CHAR_BUFFER-1);
  if (!no_amb)
  {
    if (gpw_use_lcase) strncat(gpw_chars, chars_lcase2, GPW_CHAR_BUFFER-1);
    if (gpw_use_ucase) strncat(gpw_chars, chars_ucase2, GPW_CHAR_BUFFER-1);
    if (gpw_use_num) strncat(gpw_chars, chars_num2, GPW_CHAR_BUFFER-1);
    if (gpw_use_sym) strncat(gpw_chars, chars_sym2, GPW_CHAR_BUFFER-1);
  }

  gpw_num_chars = strlen(gpw_chars);
}

static gboolean
fpm_gpw_get_check_button_value(GtkWidget* win, gchar* check_name)
{
  GtkWidget* widget;
  widget = lookup_widget(win, check_name);
  return(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
}

static void
fpm_gpw_set_check_button_value(	GtkWidget* win,	
				gchar* check_name,
				gboolean value)
{
  GtkWidget* widget;
  widget = lookup_widget(win, check_name);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), value);
}

static void
fpm_put_stats(GtkWidget* win)
/* Update the statistics on the password generator screen.  This consists
 * of the number of possible passwords that could be generated given the
 * specified criteria of length and character set.  The numbers are given
 * in log base 10, and log base 2.  Log base 10 is good because it allows
 * people to have a grasp of how big the number is (a 1 and so many zeros).
 * Log base 2 is good because it shows about how good the key is in terms
 * of bits.
 */
{
  GtkWidget* widget;
  gdouble base10;
  gdouble bits;
  gchar* tmp;

  bits = gpw_len * log(gpw_num_chars) / log(2);
  base10 = bits * log(2) / log(10);
  widget = lookup_widget(win, "entry_base_10");
  tmp = g_strdup_printf("%f", base10);
  gtk_entry_set_text(GTK_ENTRY(widget), tmp);
  g_free(tmp);
  widget = lookup_widget(win, "entry_bits");
  tmp = g_strdup_printf("%f", bits);
  gtk_entry_set_text(GTK_ENTRY(widget), tmp);
  g_free(tmp);
}

void
fpm_gpw_set_from_dialog(GtkWidget* win)
{
  GtkWidget* widget;
  
  widget = lookup_widget(win, "spinbutton_num_char");
  
  
  fpm_gpw_set_options(
	gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget)), 
	fpm_gpw_get_check_button_value(win, "checkbutton_lcase"),
	fpm_gpw_get_check_button_value(win, "checkbutton_ucase"),
	fpm_gpw_get_check_button_value(win, "checkbutton_num"),
	fpm_gpw_get_check_button_value(win, "checkbutton_sym"),
	fpm_gpw_get_check_button_value(win, "checkbutton_amb"));
  fpm_put_stats(win);
}




void
fpm_gpw_start_screen(GtkWidget* win)
/* Set the screen to match current rules. */
{
  fpm_gpw_set_check_button_value(win, "checkbutton_lcase", gpw_use_lcase);
  fpm_gpw_set_check_button_value(win, "checkbutton_ucase", gpw_use_ucase);
  fpm_gpw_set_check_button_value(win, "checkbutton_num", gpw_use_num);
  fpm_gpw_set_check_button_value(win, "checkbutton_sym", gpw_use_sym);
  fpm_gpw_set_check_button_value(win, "checkbutton_amb", gpw_no_amb);
  fpm_put_stats(win);
}


static gchar* 
fpm_gpw_gen_password()
/* Actually generate the passwords.  Notice fpm_gpw_set_options must be called
 * before running this routine, because gpw_chars and gpw_num_chars need to
 * be populated.
 * This routine assumes the existance of /dev/random.  Password generation is
 * needs good entropy, which is why I use /dev/random here.  This makes the
 * code less portable, however.  Perhaps we should roll back to another
 * random number generator if /dev/random fails.
 */
{
  gchar* password;
  FILE* rnd;
  guint16 rnd_num;
  gint i, idx;
  size_t result;
  
  password = g_malloc0(gpw_len+1);

  rnd = fopen("/dev/random", "r");

  for (i=0;i<gpw_len;i++)
  {
    result = fread(&rnd_num, sizeof(guint16), 1, rnd);
    idx = floor(gpw_num_chars * rnd_num / pow(2, 16));
    password[i] = gpw_chars[idx];
  }
  fclose(rnd);

  return(password);
}

void
fpm_gpw_fillin_password(GtkWidget* win)
{
  gchar* password;
  GtkWidget* widget;

  password = fpm_gpw_gen_password();
  widget = lookup_widget(win, "entry_gen_password");
  gtk_entry_set_text(GTK_ENTRY(widget), password);
  memset(password, 0, strlen(password));
  g_free(password);
  
}  
