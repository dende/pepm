/* FIGARO'S PASSWORD MANAGER 2 (FPM2)
 * Copyright (C) 2000 John Conneely
 * Copyright (C) 2009,2010 Aleš Koval
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
 * fpm_file.c formerly passfile.c -- Routines to save and load XML files with FPM data.
 */

#include <gtk/gtk.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>

#include "fpm.h"
#include "fpm_file.h"
#include "fpm_crypt.h"
#include "support.h"

static void new_leaf_encrypt_password(xmlDocPtr doc, xmlNodePtr item, const xmlChar *name, char *text);

static void
passfile_update_key()
/* This routine goes through the entire password list and will decrypt the
 * passwords with an old key and encrypt it again with a new key.  This is
 * needed in two situations: First, if the user changes a password.  Secondly,
 * (and much more common) if the salt changes.  In practice, this routine gets
 * called whenever we save a file.
 */
{
  fpm_data* data;
  gchar plaintext[FPM_PASSWORD_LEN+1] = {0};
  GList *list;
  guint ix;

  if (old_context!=new_context)
  {
    list=g_list_first(glb_pass_list);
    while(list!=NULL)
    {
      data = list->data;
      ix = strlen(data->password) / 2;
      fpm_decrypt_field(old_context, plaintext,
	 data->password, ix);
      fpm_encrypt_field(new_context, data->password,
	 plaintext, FPM_PASSWORD_LEN);
      list = g_list_next(list);
    }

    wipememory(plaintext, FPM_PASSWORD_LEN);
    memcpy(old_context, new_context , cipher->contextsize);
    old_salt = new_salt;
  }
}

static void
move_backup(gchar* file_name)
{
  gchar file1[512];
  gchar file2[512];
  gint i;


  for(i=ini->number_backup_files;i>0;i--)
  {
    g_snprintf(file2, 512, "%s.%02d", file_name, i);
    if (i>1)
      g_snprintf(file1, 512, "%s.%02d", file_name, i-1);
    else
      g_snprintf(file1, 512, "%s", file_name);

    rename(file1, file2);

  }
}

static void
passfile_upd_attr(xmlNodePtr node, char *cond, char** data_ptr)
{
  if (!strcmp((char *)node->name, cond))
  {
    g_free(*data_ptr);
    if(node->xmlChildrenNode != NULL)
      *data_ptr=g_strdup((char *)node->xmlChildrenNode->content);
    else
      *data_ptr=g_strdup("");
  }
}

static void new_leaf(xmlDocPtr doc, xmlNodePtr item, const xmlChar *name, xmlChar *text)
{
  xmlChar *tmp;

  tmp = xmlEncodeEntitiesReentrant(doc, text);
  xmlNewChild(item, NULL, name, tmp);
  free(tmp);
}

static void new_leaf_encrypt
	(xmlDocPtr doc, xmlNodePtr item, const xmlChar *name, char *text)
{
  char *cipher_text;

  cipher_text=fpm_encrypt_field_var(new_context, text);
  new_leaf(doc, item, name, (xmlChar *)cipher_text);
  g_free(cipher_text);
}


void
fpm_file_save(gchar* file_name, gboolean convert)
{
  xmlDocPtr doc;
  xmlNodePtr nodelist, item;
  fpm_data* data;
  fpm_launcher* launcher;
  FILE * file;
//  gchar* cmd;
  gchar* tmp;
  GList * list;
  gint i;
  gchar *dia_str;
  gchar *vstring_cipher = "";
  gchar *vstring_plain = "";

  if (ini->create_backup) 
    move_backup(file_name);

  if(!convert)
  passfile_update_key();

  doc = xmlNewDoc((xmlChar *)"1.0");
  doc->xmlRootNode = xmlNewDocNode(doc, NULL, (xmlChar *)"FPM", NULL);
  xmlSetProp(doc->xmlRootNode, (xmlChar *)"full_version", (xmlChar *)FULL_VERSION);
  if(cipher_algo == BLOWFISH)
    xmlSetProp(doc->xmlRootNode, (xmlChar *)"min_version", (xmlChar *)MIN_VERSION_BLOWFISH);
  else
    xmlSetProp(doc->xmlRootNode, (xmlChar *)"min_version", (xmlChar *)MIN_VERSION);
  xmlSetProp(doc->xmlRootNode, (xmlChar *)"display_version", (xmlChar *)DISPLAY_VERSION);
  item = xmlNewChild(doc->xmlRootNode, NULL, (xmlChar *)"KeyInfo", NULL);
  xmlSetProp(item, (xmlChar *)"cipher", (xmlChar *)cipher->name);
  xmlSetProp(item, (xmlChar *)"salt", (xmlChar *)new_salt);

  if(cipher_algo == BLOWFISH) {
    vstring_cipher = g_malloc0(cipher->blocksize * 2 + 1);
    vstring_plain = g_malloc0(cipher->blocksize + 1);
    strncpy(vstring_plain, "FIGARO", cipher->blocksize);
    fpm_encrypt_field(new_context, vstring_cipher, vstring_plain, cipher->blocksize);
  } else if(cipher_algo == AES256) {
    vstring_cipher = g_malloc0(cipher->hash_len * 2 + 1);
    vstring_plain = g_malloc0(cipher->hash_len + 1);
    fpm_sha256_fpm_data(vstring_plain);
    fpm_encrypt_field(new_context, vstring_cipher, vstring_plain, cipher->hash_len);
  }
    xmlSetProp(item, (xmlChar *)"vstring", (xmlChar *)vstring_cipher);
    g_free(vstring_plain);
    g_free(vstring_cipher);

  nodelist = xmlNewChild(doc->xmlRootNode, NULL, (xmlChar *)"LauncherList", NULL);
  list = g_list_first(glb_launcher_list);

  while (list!=NULL)
  {
    launcher=list->data;
    item=xmlNewChild(nodelist, NULL, (xmlChar *)"LauncherItem", NULL);
    if(cipher_algo == BLOWFISH) {
	new_leaf(doc, item, (xmlChar *)"title", (xmlChar *)launcher->title);
	new_leaf(doc, item, (xmlChar *)"cmdline", (xmlChar *)launcher->cmdline);
	tmp=g_strdup_printf("%d", launcher->copy_user);
	new_leaf(doc, item, (xmlChar *)"copy_user", (xmlChar *)tmp);
	g_free(tmp);
	tmp=g_strdup_printf("%d", launcher->copy_password);
	new_leaf(doc, item, (xmlChar *)"copy_password", (xmlChar *)tmp);
    } else {
	new_leaf_encrypt(doc, item, (xmlChar *)"title", launcher->title);
	new_leaf_encrypt(doc, item, (xmlChar *)"cmdline", launcher->cmdline);
	tmp=g_strdup_printf("%d", launcher->copy_user);
	new_leaf_encrypt(doc, item, (xmlChar *)"copy_user", tmp);
	g_free(tmp);
	tmp=g_strdup_printf("%d", launcher->copy_password);
	new_leaf_encrypt(doc, item, (xmlChar *)"copy_password", tmp);
    }
    g_free(tmp);
    list = g_list_next(list);
  }
  nodelist = xmlNewChild(doc->xmlRootNode, NULL, (xmlChar *)"PasswordList", NULL);
  list = g_list_first(glb_pass_list);
  i=0;
  while (list!=NULL)
  {
    data = list->data;
    item = xmlNewChild(nodelist, NULL, (xmlChar *)"PasswordItem", NULL);
    new_leaf_encrypt(doc, item, (xmlChar *)"title", data->title);
    new_leaf_encrypt(doc, item, (xmlChar *)"user", data->user);
    new_leaf_encrypt(doc, item, (xmlChar *)"url", data->arg);

    if(convert)
	new_leaf_encrypt_password(doc, item, (xmlChar *)"password", data->password);
    else
	new_leaf(doc, item, (xmlChar *)"password", (xmlChar *)data->password);

    new_leaf_encrypt(doc, item, (xmlChar *)"notes", data->notes);
    new_leaf_encrypt(doc, item, (xmlChar *)"category", data->category);
    new_leaf_encrypt(doc, item, (xmlChar *)"launcher", data->launcher);

    if (data->default_list) xmlNewChild(item, NULL, (xmlChar *)"default", NULL);

    list=g_list_next(list);
    i++;
  }

  file=fopen(file_name, "w");
  xmlDocDump(file, doc);
  fclose(file);
//  chmod(file_name, S_IRUSR | S_IWUSR);
/*  cmd = g_strdup_printf("chmod 0600 %s", file_name);
  fpm_execute_shell(cmd);
  g_free(cmd);
*/
  dia_str = g_strdup_printf(_("Saved %d password(s)."), i);
  fpm_statusbar_push(dia_str);

//  printf("Saved %d password(s).\n", i);
//  state->dirty = FALSE;
  xmlFreeDoc(doc);
  fpm_dirty(FALSE);
}

void
passfile_save(gchar* file_name) {
    fpm_file_save(file_name, FALSE);
}

void
fpm_file_export(gchar *file_name, gint export_launchers, gchar *export_category)
{
  xmlDocPtr doc;
  xmlNodePtr nodelist, item;
  fpm_data* data;
  fpm_launcher* launcher;
  FILE * file;
//  gchar* cmd;
  gchar* tmp;
  GList * list;
  gint i;
  gchar plaintext[FPM_PASSWORD_LEN+1] = {0};
  gchar *dia_str;

  doc = xmlNewDoc((xmlChar *)"1.0");
  doc->xmlRootNode = xmlNewDocNode(doc, NULL, (xmlChar *)"FPM", NULL);
  xmlSetProp(doc->xmlRootNode, (xmlChar *)"full_version", (xmlChar *)FULL_VERSION);
  xmlSetProp(doc->xmlRootNode, (xmlChar *)"min_version", (xmlChar *)MIN_VERSION);
  xmlSetProp(doc->xmlRootNode, (xmlChar *)"display_version", (xmlChar *)DISPLAY_VERSION);
  item = xmlNewChild(doc->xmlRootNode, NULL, (xmlChar *)"KeyInfo", NULL);
  xmlSetProp(item, (xmlChar *)"cipher", (xmlChar *)"none");

  if(export_launchers) {

  nodelist = xmlNewChild(doc->xmlRootNode, NULL, (xmlChar *)"LauncherList", NULL);
  list = g_list_first(glb_launcher_list);

  while (list != NULL)
  {
    launcher = list->data;
    if((export_launchers == 2) || ((export_launchers == 1) && fpm_is_launcher_in_category(launcher, export_category))) {
    item = xmlNewChild(nodelist, NULL, (xmlChar *)"LauncherItem", NULL);
    new_leaf(doc, item, (xmlChar *)"title", (xmlChar *)launcher->title);
    new_leaf(doc, item, (xmlChar *)"cmdline", (xmlChar *)launcher->cmdline);
    tmp = g_strdup_printf("%d", launcher->copy_user);
    new_leaf(doc, item, (xmlChar *)"copy_user", (xmlChar *)tmp);
    g_free(tmp);
    tmp = g_strdup_printf("%d", launcher->copy_password);
    new_leaf(doc, item, (xmlChar *)"copy_password", (xmlChar *)tmp);
    g_free(tmp);
    }

    list = g_list_next(list);
  }

  }
  nodelist = xmlNewChild(doc->xmlRootNode, NULL, (xmlChar *)"PasswordList", NULL);
  list = g_list_first(glb_pass_list);
  i = 0;

  while (list != NULL)
  {
    data = list->data;

    if(!strcmp(export_category,data->category) || !strcmp(export_category, FPM_ALL_CAT_MSG)) {

    item = xmlNewChild(nodelist, NULL, (xmlChar *)"PasswordItem", NULL);
    new_leaf(doc, item, (xmlChar *)"title", (xmlChar *)data->title);
    new_leaf(doc, item, (xmlChar *)"user", (xmlChar *)data->user);
    new_leaf(doc, item, (xmlChar *)"url", (xmlChar *)data->arg);

    fpm_decrypt_field(old_context, plaintext, data->password, FPM_PASSWORD_LEN);

    new_leaf(doc, item, (xmlChar *)"password", (xmlChar *)plaintext);
    new_leaf(doc, item, (xmlChar *)"notes", (xmlChar *)data->notes);
    new_leaf(doc, item, (xmlChar *)"category", (xmlChar *)data->category);
    new_leaf(doc, item, (xmlChar *)"launcher", (xmlChar *)data->launcher);

    if (data->default_list) xmlNewChild(item, NULL, (xmlChar *)"default", NULL);
    i++;
    }
    list = g_list_next(list);
  }

  /* Zero out plain text */
  wipememory(plaintext, FPM_PASSWORD_LEN);

  file=fopen(file_name, "w");
  xmlDocDump(file, doc);
  fclose(file);
/*  cmd = g_strdup_printf("chmod 0600 %s", file_name);
  fpm_execute_shell(cmd);
  g_free(cmd);
*/
  dia_str = g_strdup_printf(_("Exported clear XML passwords to %s."), file_name);
  fpm_statusbar_push(dia_str);
  g_free(dia_str);
}

gint
passfile_load(gchar* file_name)
{
  xmlDocPtr doc;
  xmlNodePtr list, item, attr;
  fpm_data* data;
  fpm_launcher* launcher;
  gint i;
  gchar *cipher_name;

  /* Start from scratch */
  if(glb_pass_list != NULL) fpm_clear_list();

  LIBXML_TEST_VERSION
  xmlKeepBlanksDefault(0);
  doc=xmlParseFile(file_name);
  if (doc == NULL)
  {
    /* If we can't read the doc, then assume we are running for first time.*/
    fpm_cipher_init("AES-256");
    old_salt=get_new_salt(cipher->salt_len);
    new_salt=old_salt;
    glb_pass_list=NULL;
//    state->dirty=TRUE;
    fpm_dirty(TRUE);
    fpm_init_launchers();
    return(-1);
  }

  /* Check if document is one of ours */
  g_return_val_if_fail(!xmlStrcmp(doc->xmlRootNode->name, (xmlChar *)"FPM"), -2);

  file_version = (char *)xmlGetProp(doc->xmlRootNode, (xmlChar *)"min_version");
  if (strcmp(file_version, FULL_VERSION) > 0)
  {
    printf(_("Sorry, the password file cannot be read because it uses a future file format. Please download the latest version of FPM2 and try again.\n"));
    exit(-1);
  }

  free(file_version);
  file_version = (char *)xmlGetProp(doc->xmlRootNode, (xmlChar *)"full_version");

/* Current versions of FPM encrypt all fields.  Fields other than the 
 * password are decrypted when the program reads the password file, 
 * and the password is decrypted as it is needed (to try to prevent it
 * from showing up in coredumps, etc.)  The global variable  glb_need_decrypt
 * is set by the passfile loading routines to specify that the passfile
 * was created with a version of FPM that requires this decryption.  
 * This allows backward compatibility with previous versions of FPM which
 * only encrypted passwords.
 */

  glb_need_decrypt = (strcmp(file_version, ENCRYPT_VERSION) >= 0);

  list=doc->xmlRootNode->xmlChildrenNode;
  old_salt = (char *)xmlGetProp(list, (xmlChar *)"salt");
  vstring = (char *)xmlGetProp(list, (xmlChar *)"vstring");
  cipher_name = (char *)xmlGetProp(list, (xmlChar *)"cipher");

  /* If not find cipher attribute in password file we assume BLOWFISH */
  if ( cipher_name == NULL) {
    cipher_name = g_strdup("BLOWFISH");
  }

  fpm_cipher_init(cipher_name);

  g_free(cipher_name);

  new_salt = get_new_salt(cipher->salt_len);

  list=list->next;

  if (list==NULL || ( strcmp((char *)list->name, "PasswordList") && strcmp((char *)list->name, "LauncherList") ))
  {
    g_error("Invalid password file.");
    gtk_main_quit();
  }

  if(!strcmp((char *)list->name, "LauncherList"))
  {
//    printf("Loading launchers...\n");
    glb_launcher_list=NULL;
    item=list->xmlChildrenNode;
    while(item!=NULL)
    {
      launcher=g_malloc0(sizeof(fpm_launcher));
      launcher->title=g_strdup("");
      launcher->cmdline=g_strdup("");
      launcher->c_user=g_strdup("");
      launcher->c_pass=g_strdup("");
      launcher->copy_user=0;
      launcher->copy_password=0;
      attr=item->xmlChildrenNode;
      while(attr!=NULL)
      {
	if(!strcmp((char *)attr->name, "title") && attr->xmlChildrenNode && attr->xmlChildrenNode->content)
	{
	  g_free(launcher->title);
	  launcher->title=g_strdup((char *)attr->xmlChildrenNode->content);
	}
	if(!strcmp((char *)attr->name, "cmdline") && attr->xmlChildrenNode && attr->xmlChildrenNode->content)
	{
	  g_free(launcher->cmdline);
	  launcher->cmdline=g_strdup((char *)attr->xmlChildrenNode->content);
	}
    if( cipher_algo == BLOWFISH) {
	if(!strcmp((char *)attr->name, "copy_user"))
	{
	  if(!strcmp((char *)attr->xmlChildrenNode->content, "1")) launcher->copy_user=1;
	  if(!strcmp((char *)attr->xmlChildrenNode->content, "2")) launcher->copy_user=2;
	}
	if(!strcmp((char *)attr->name, "copy_password"))
	{
	  if(!strcmp((char *)attr->xmlChildrenNode->content, "1")) launcher->copy_password=1;
	  if(!strcmp((char *)attr->xmlChildrenNode->content, "2")) launcher->copy_password=2;
	}
    } else {
	if(!strcmp((char *)attr->name, "copy_user") && attr->xmlChildrenNode && attr->xmlChildrenNode->content)
	{
	  g_free(launcher->c_user);
	  launcher->c_user=g_strdup((char *)attr->xmlChildrenNode->content);
	}
	if(!strcmp((char *)attr->name, "copy_password") && attr->xmlChildrenNode && attr->xmlChildrenNode->content)
	{
	  g_free(launcher->c_pass);
	  launcher->c_pass=g_strdup((char *)attr->xmlChildrenNode->content);
	}
    }
      attr=attr->next;
      }
      glb_launcher_list=g_list_append(glb_launcher_list, launcher);

      item=item->next;
    }
    fpm_create_launcher_string_list();

    /* Incurement top-level list from launcher list to password list. */
    list=list->next; 
  }
  else
  {
    fpm_init_launchers();
  }

  if (list==NULL || strcmp((char *)list->name, "PasswordList"))
  {
    g_error("Invalid password file.");
    gtk_main_quit();
  }


  i=0;

  item=list->xmlChildrenNode;
  while(item!=NULL)
  {

    /* Start with blank data record */
    data = g_malloc0(sizeof(fpm_data));
    data->title=g_strdup("");
    data->arg=g_strdup("");
    data->user=g_strdup("");
    data->notes=g_strdup("");
    data->category=g_strdup("");
    data->launcher=g_strdup("");
    data->default_list=0;

    /* Update data record with each type of attribute */
    attr=item->xmlChildrenNode;
    while(attr!=NULL)
    {
      passfile_upd_attr(attr, "title", &data->title);
      passfile_upd_attr(attr, "url", &data->arg);
//      passfile_upd_attr(attr, "arg", &data->arg);
      passfile_upd_attr(attr, "user", &data->user);
      passfile_upd_attr(attr, "notes", &data->notes);
      passfile_upd_attr(attr, "category", &data->category);
      passfile_upd_attr(attr, "launcher", &data->launcher);
      if(!strcmp((char *)attr->name, "default"))
      {
        data->default_list=1;
      }

      if(!strcmp((char *)attr->name, "password"))
        strncpy(data->password, (char *)attr->xmlChildrenNode->content, FPM_PASSWORD_LEN*2);

      attr=attr->next;
    }

    /* Insert item into GList */

    glb_pass_list=g_list_append(glb_pass_list, data);

    item=item->next;
    i++;
  }
  xmlFreeDoc(doc);
  free(file_version);

//  dia_str = g_strdup_printf(_("Loaded %d password(s)."), i);
//  fpm_statusbar_push(dia_str);
//  g_free(dia_str);

//  state->dirty = FALSE;
  fpm_dirty(FALSE);

  return(0);
}

static void new_leaf_encrypt_password
	(xmlDocPtr doc, xmlNodePtr item, const xmlChar *name, char *text)
{
  gchar cipher_text[FPM_PASSWORD_LEN*2+1] = {0};
  gchar plaintext[FPM_PASSWORD_LEN+1] = {0};

  strncpy(plaintext, text, FPM_PASSWORD_LEN-1);
  fpm_encrypt_field(new_context, cipher_text, plaintext, FPM_PASSWORD_LEN);
  new_leaf(doc, item, name, (xmlChar *)cipher_text);
  wipememory(plaintext, FPM_PASSWORD_LEN);
}

void fpm_file_convert(gchar* file_name, gchar *password) {
  gchar plaintext[FPM_PASSWORD_LEN+1] = {0};
  fpm_data* data;
  GList *list;
  guint ix;

    list = g_list_first(glb_pass_list);
    while(list != NULL)
    {
      data = list->data;
      ix = strlen(data->password) / 2;
      fpm_decrypt_field(old_context, plaintext,
	 data->password, ix);
	 memcpy(data->password, plaintext, strlen(plaintext)+1);
      list = g_list_next(list);
    }

    wipememory(plaintext, FPM_PASSWORD_LEN);

    fpm_cipher_init("AES-256");
    old_salt = get_new_salt(cipher->salt_len);
    new_salt = old_salt;
    fpm_crypt_init(password);
    fpm_file_save(file_name, TRUE);
}


gint
fpm_file_import(gchar *file_name, gint import_launchers, gchar *import_category, gchar **message) {
  xmlDocPtr doc;
  xmlNodePtr list, item, attr;
  fpm_data *data;
  fpm_launcher *launcher;
  gint i;
  gchar plaintext[FPM_PASSWORD_LEN+1] = {0};

  LIBXML_TEST_VERSION
  xmlKeepBlanksDefault(0);
  doc = xmlParseFile(file_name);
  if (doc == NULL) {
    *message = g_strdup(_("File is not XML?"));
    return(-1);
  }

  /* Check if document is one of ours */
  if(xmlStrcmp(doc->xmlRootNode->name, (xmlChar *)"FPM")) {
    *message = g_strdup(_("File is not valid FPM2 password file."));
    return(-1);
  }

  list=doc->xmlRootNode->xmlChildrenNode;

  list=list->next;

  if (list==NULL || ( strcmp((char *)list->name, "PasswordList") && strcmp((char *)list->name, "LauncherList") ))
  {
    *message = g_strdup(_("Invalid password file."));
    return(-1);
  }

  if(!strcmp((char *)list->name, "LauncherList"))
  {
  if(import_launchers != 0) {
    if(import_launchers == 2)
	fpm_launcher_remove_all();
    item=list->xmlChildrenNode;
    while(item!=NULL)
    {
      launcher=g_malloc0(sizeof(fpm_launcher));
      launcher->title=g_strdup("");
      launcher->cmdline=g_strdup("");
      launcher->copy_user=0;
      launcher->copy_password=0;
      attr=item->xmlChildrenNode;
      while(attr!=NULL)
      {
	if(!strcmp((char *)attr->name, "title") && attr->xmlChildrenNode && attr->xmlChildrenNode->content)
	{
	  g_free(launcher->title);
	  launcher->title=g_strdup((char *)attr->xmlChildrenNode->content);
	}
	if(!strcmp((char *)attr->name, "cmdline") && attr->xmlChildrenNode && attr->xmlChildrenNode->content)
	{
	  g_free(launcher->cmdline);
	  launcher->cmdline=g_strdup((char *)attr->xmlChildrenNode->content);
	}
	if(!strcmp((char *)attr->name, "copy_user"))
	{
	  if(!strcmp((char *)attr->xmlChildrenNode->content, "1")) launcher->copy_user=1;
	  if(!strcmp((char *)attr->xmlChildrenNode->content, "2")) launcher->copy_user=2;
	}
	if(!strcmp((char *)attr->name, "copy_password"))
	{
	  if(!strcmp((char *)attr->xmlChildrenNode->content, "1")) launcher->copy_password=1;
	  if(!strcmp((char *)attr->xmlChildrenNode->content, "2")) launcher->copy_password=2;
	}
      attr=attr->next;
      }

	if((import_launchers == 2) || ((import_launchers == 1) && !fpm_launcher_search(launcher->title)))
	    glb_launcher_list = g_list_append(glb_launcher_list, launcher);

      item=item->next;
    }
    fpm_create_launcher_string_list();
}
    /* Incurement top-level list from launcher list to password list. */
    list=list->next; 

  }

  if (list==NULL || strcmp((char *)list->name, "PasswordList"))
  {
    *message = g_strdup(_("Invalid password file."));
    return(-1);
  }

  i=0;

  item=list->xmlChildrenNode;
  while(item!=NULL)
  {

    /* Start with blank data record */
    data = g_malloc0(sizeof(fpm_data));
    data->title=g_strdup("");
    data->arg=g_strdup("");
    data->user=g_strdup("");
    data->notes=g_strdup("");
    if(strcmp(import_category, FPM_NONE_CAT_MSG))
	data->category=g_strdup(import_category);
    else
	data->category=g_strdup("");
    data->launcher=g_strdup("");
    data->default_list=0;

    /* Update data record with each type of attribute */
    attr=item->xmlChildrenNode;
    while(attr!=NULL)
    {
      passfile_upd_attr(attr, "title", &data->title);
      passfile_upd_attr(attr, "url", &data->arg);
      passfile_upd_attr(attr, "user", &data->user);
      passfile_upd_attr(attr, "notes", &data->notes);
      if(!strcmp(import_category,""))
        passfile_upd_attr(attr, "category", &data->category);
      passfile_upd_attr(attr, "launcher", &data->launcher);

      if(!strcmp((char *)attr->name, "default"))
      {
        data->default_list=1;
      }

      if(!strcmp((char *)attr->name, "password")) {
	gchar *xmlplain;

	xmlplain = (gchar *)xmlNodeListGetString(doc, attr->xmlChildrenNode, 1);
	if(xmlplain != NULL) {
	    strncpy(plaintext, xmlplain , FPM_PASSWORD_LEN);
	} else {
	    strcpy(plaintext,"");
	}
	fpm_encrypt_field(old_context, data->password, plaintext, FPM_PASSWORD_LEN);
	g_free(xmlplain);
      }

      attr=attr->next;
    }

    /* Insert item into GList */
    glb_pass_list=g_list_append(glb_pass_list, data);

    item=item->next;
    i++;
  }
  xmlFreeDoc(doc);
/*  free(file_version); */

//  state->dirty = TRUE;
  fpm_dirty(TRUE);

  *message = g_strdup_printf(_("Imported %d password(s)."), i);

  return(0);
}
