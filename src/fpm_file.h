/* FIGARO'S PASSWORD MANAGER 2 (FPM2)
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
 * fpm_file.h formerly passfile.h
 */

void passfile_save(gchar* file_name);
gint passfile_load(gchar* file_name);
void fpm_file_save(gchar* file_name, gboolean convert);
void fpm_file_convert(gchar* file_name, gchar *password);
gint fpm_file_import(gchar *file_name, gint import_launchers, gchar *import_category, gchar **message);
void fpm_file_export(gchar *file_name, gint export_launchers, gchar *export_category);
