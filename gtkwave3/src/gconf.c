/*
 * Copyright (c) Tony Bybell 2012.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <config.h>
#include "gconf.h"
#include "wavealloca.h"
#include "globals.h"

int wave_rpc_id = 0;

#ifdef WAVE_HAVE_GCONF

static GConfClient* client = NULL;

/************************************************************/

static void
open_callback(GConfClient* client,
                     guint cnxn_id,
                     GConfEntry *entry,
                     gpointer user_data)
{
  if (gconf_entry_get_value (entry) == NULL)
    {
      /* value is unset */
    }
  else
    {
      if (gconf_entry_get_value (entry)->type == GCONF_VALUE_STRING)
        {
	  fprintf(stderr, "GTKWAVE | RPC Open: '%s'\n", gconf_value_get_string (gconf_entry_get_value (entry)) );

	  deal_with_rpc_open(gconf_value_get_string (gconf_entry_get_value (entry)), NULL);
	  gconf_entry_set_value(entry, NULL);
        }
      else
        {
          /* value is of wrong type */
        }
    }
}


static void
quit_callback(GConfClient* client,
                     guint cnxn_id,
                     GConfEntry *entry,
                     gpointer user_data)
{
  if (gconf_entry_get_value (entry) == NULL)
    {
      /* value is unset */
    }
  else
    {
      if (gconf_entry_get_value (entry)->type == GCONF_VALUE_STRING)
        {
	  const char *rc = gconf_value_get_string (gconf_entry_get_value (entry));
	  int rcv = atoi(rc);
	  fprintf(stderr, "GTKWAVE | RPC Quit: exit return code %d\n", rcv);
	  gconf_entry_set_value(entry, NULL);
	  exit(rcv);
        }
      else
        {
          /* value is of wrong type */
        }
    }
}


static void
writesave_callback(GConfClient* client,
                     guint cnxn_id,
                     GConfEntry *entry,
                     gpointer user_data)
{
  if (gconf_entry_get_value (entry) == NULL)
    {
      /* value is unset */
    }
  else
    {
      if (gconf_entry_get_value (entry)->type == GCONF_VALUE_STRING)
        {
	  const char *fni = gconf_value_get_string (gconf_entry_get_value (entry));
	  if(fni)
		{
		  int use_arg = strcmp(fni, "+"); /* plus filename uses default */
		  const char *fn = use_arg ? fni : GLOBALS->filesel_writesave;
		  if(fn)
			{
		  	FILE *wave = fopen(fn, "wb");
	
		  	if(!(wave=fopen(fn, "wb")))
		        	{
		        	fprintf(stderr, "GTKWAVE | RPC Writesave: error opening save file '%s' for writing.\n", fn);
		        	perror("Why");
		        	errno=0;
		        	}
		        	else
		        	{
		        	write_save_helper(fn, wave);
				if(use_arg)
					{
					if(GLOBALS->filesel_writesave) { free_2(GLOBALS->filesel_writesave); }
					GLOBALS->filesel_writesave = strdup_2(fn);
					}
		        	wave_gconf_client_set_string("/current/savefile", fn);
		        	fclose(wave);
		        	fprintf(stderr, "GTKWAVE | RPC Writesave: wrote save file '%s'.\n", GLOBALS->filesel_writesave);
		        	}
			}
		}

	  gconf_entry_set_value(entry, NULL);
        }
      else
        {
          /* value is of wrong type */
        }
    }
}

/************************************************************/

static void remove_client(void)
{
if(client)
	{
	g_object_unref(client);
	}
}


void wave_gconf_init(int argc, char **argv)
{
char *ks = wave_alloca(WAVE_GCONF_DIR_LEN + 32 + 32 + 1);
int len = sprintf(ks, WAVE_GCONF_DIR"/%d", wave_rpc_id);

gconf_init(argc, argv, NULL);
client = gconf_client_get_default();
atexit(remove_client);

gconf_client_add_dir(client,
	ks,
        GCONF_CLIENT_PRELOAD_NONE,
        NULL);

strcpy(ks + len, "/open");
gconf_client_notify_add(client, ks,
                          open_callback,
                          NULL, /* user data */
                          NULL, NULL);

strcpy(ks + len, "/quit");
gconf_client_notify_add(client, ks,
                          quit_callback,
                          NULL, /* user data */
                          NULL, NULL);

strcpy(ks + len, "/writesave");
gconf_client_notify_add(client, ks,
                          writesave_callback,
                          NULL, /* user data */
                          NULL, NULL);
}


gboolean wave_gconf_client_set_string(const gchar *key, const gchar *val)
{
if(key)
	{
	char *ks = wave_alloca(WAVE_GCONF_DIR_LEN + 32 + strlen(key) + 1);
	sprintf(ks, WAVE_GCONF_DIR"/%d%s", wave_rpc_id, key);

	return(gconf_client_set_string(client, ks, val ? val : "", NULL));
	}

return(FALSE);
}


#else

void wave_gconf_init(int argc, char **argv)
{
}

gboolean wave_gconf_client_set_string(const gchar *key, const gchar *val)
{
return(FALSE);
}

#endif

/*

Examples of RPC manipulation:

gconftool-2 --dump /com.geda.gtkwave
gconftool-2 --dump /com.geda.gtkwave --recursive-unset

gconftool-2 --type string --set /com.geda.gtkwave/0/open /pub/systema_packed.fst
gconftool-2 --type string --set /com.geda.gtkwave/0/open `pwd`/`basename -- des.gtkw`

gconftool-2 --type string --set /com.geda.gtkwave/0/writesave /tmp/this.gtkw
gconftool-2 --type string --set /com.geda.gtkwave/0/writesave +

gconftool-2 --type string --set /com.geda.gtkwave/0/quit 0

*/