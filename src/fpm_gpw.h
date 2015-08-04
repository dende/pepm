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
 * fpm_gpw.h
 */

void   
fpm_gpw_set_options(    gint            pw_len,
                        gboolean        use_lcase,
                        gboolean        use_ucase,
                        gboolean        use_num,
                        gboolean        use_sym,
                        gboolean        no_amb);

void
fpm_gpw_set_from_dialog(GtkWidget* win);

void
fpm_gpw_start_screen(GtkWidget* win);


void
fpm_gpw_fillin_password(GtkWidget* win);

