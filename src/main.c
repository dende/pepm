/* FIGARO'S PASSWORD MANAGER 2 (FPM2)
 * Copyright (C) 2000 John Conneely
 * Copyright (C) 2008,2009 Aleš Koval
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
 * main.c
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdlib.h>
#include <unistd.h>

#include "interface.h"
#include "support.h"

#include <libxml/parser.h>

#include "fpm.h"

int
main (int argc, char *argv[])
{

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  gtk_set_locale ();
  gtk_init (&argc, &argv);

  add_pixmap_directory (PACKAGE_PIXMAP_DIR);

  char *opt_file_name = NULL;
  int opt, tray_on_startup = 0;

  while ((opt = getopt(argc, argv, "f:th?")) != EOF) {
    switch (opt) {
	case 'f':
	    opt_file_name = optarg;
	    break;
	case 't':
	    tray_on_startup = 1;
	    break;
	default:
	    printf("Usage: fpm2 [-f filename] [-t]\n");
	    printf("	-f filename		Open filename passwords file\n");
	    printf("	-t			Start in tray icon\n");
	    exit(0);
    }
  }

  fpm_init (opt_file_name, tray_on_startup);

  gtk_main ();

  fpm_tr_cleanup();
  unlock_fpm_file();

  xmlCleanupParser();

  return 0;
}
