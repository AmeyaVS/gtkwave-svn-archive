/* 
 * Copyright (c) Tony Bybell 1999-2009
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"
#include <config.h>
#include "currenttime.h"
#include "print.h"
#include "menu.h"
#include <errno.h>

static char *render_targets[]=
        {"PDF", "PS", "MIF"};

static char *page_size[]=
        {"Letter (8.5\" x 11\")", "A4 (11.68\" x 8.26\")", "Legal (14\" x 8.5\")", "Letter Prop (6.57\" x 8.5\")", "A4 Prop (8.26\" x 5.84\")"};

static char *render_type[]=
        {"Full", "Minimal"};

static gdouble px[]={11.00, 11.68, 14.00, 8.50, 8.26};
static gdouble py[]={ 8.50,  8.26,  8.50, 6.57, 5.84};


/*
 * button/menu/entry activations..
 */
static void render_clicked(GtkWidget *widget, gpointer which)
{
int i;

for(i=0;i<3;i++) GLOBALS->target_mutex_renderopt_c_1[i]=0;

i = (int)((long)which);
GLOBALS->target_mutex_renderopt_c_1[i] = 1; /* mark our choice */

DEBUG(printf("picked: %s\n", render_targets[i]));
}

static void pagesize_clicked(GtkWidget *widget, gpointer which)
{
int i;

for(i=0;i<5;i++) GLOBALS->page_mutex_renderopt_c_1[i]=0;

GLOBALS->page_size_type_renderopt_c_1 = (int)((long)which);
GLOBALS->page_mutex_renderopt_c_1[GLOBALS->page_size_type_renderopt_c_1] = 1; /* mark our choice */

DEBUG(printf("picked: %s\n", page_size[GLOBALS->page_size_type_renderopt_c_1]));
}

static void rendertype_clicked(GtkWidget *widget, gpointer which)
{
int i;

for(i=0;i<3;i++) GLOBALS->render_mutex_renderopt_c_1[i]=0;

i = (int)((long)which);
GLOBALS->render_mutex_renderopt_c_1[i] = 1; /* mark our choice */

DEBUG(printf("picked: %s\n", render_type[i]));
}


static void
ps_print_cleanup(GtkWidget *widget, gpointer data)
{
FILE *wave;

if(GLOBALS->filesel_ok)
        {
        DEBUG(printf("PS Print Fini: %s\n", *GLOBALS->fileselbox_text));
                
        if(!(wave=fopen(*GLOBALS->fileselbox_text,"wb")))
                {
                fprintf(stderr, "Error opening PS output file '%s' for writing.\n",*GLOBALS->fileselbox_text);
                perror("Why");
                errno=0;
                }
                else
                {
                print_ps_image(wave,px[GLOBALS->page_size_type_renderopt_c_1],py[GLOBALS->page_size_type_renderopt_c_1]);
                fclose(wave);
                }
        }  
}

static void
pdf_print_cleanup(GtkWidget *widget, gpointer data)
{
FILE *wave;
FILE *wave2;

if(GLOBALS->filesel_ok)
        {
        DEBUG(printf("PDF Print Fini: %s\n", *GLOBALS->fileselbox_text));
                
        if(!(wave=fopen(*GLOBALS->fileselbox_text,"wb")))
                {
                fprintf(stderr, "Error opening PDF output file '%s' for writing.\n",*GLOBALS->fileselbox_text);
                perror("Why");
                errno=0;
                }
                else
                {
		int len = strlen(*GLOBALS->fileselbox_text) ;
		char *zname = malloc_2(len + 4);
		strcpy(zname, *GLOBALS->fileselbox_text);

#ifdef MAC_INTEGRATION
		if((len > 4)&&(!strcmp(".pdf", zname+len-4)))
			{
			zname[len-4] = 0;
			len-=4;
			}
#endif
		strcpy(zname+len, ".ps");

		if(!(wave2=fopen(zname,"wb")))		
			{
	                fprintf(stderr, "Error opening PS output tempfile '%s' for writing.\n",zname);
	                perror("Why");
			fclose(wave);
			unlink(*GLOBALS->fileselbox_text);			
	                errno=0;
			}
			else
			{
			char *sysname = malloc_2(7 + 1 + len + 3 + 1 + len + 1);
			int rc;	

#ifdef MAC_INTEGRATION
			sprintf(sysname, "pstopdf"	/* 7 */
#else
			sprintf(sysname, "ps2pdf"	/* 6 */
#endif
					 " "		/* 1 */
					 "%s"		/* len + 3 */
					 " "		/* 1 */
					 "%s"		/* len */
					 , zname, *GLOBALS->fileselbox_text);
	                print_ps_image(wave2,px[GLOBALS->page_size_type_renderopt_c_1],py[GLOBALS->page_size_type_renderopt_c_1]);
	                fclose(wave2);
			fclose(wave);
			rc = system(sysname);
			if(rc)
				{
				printf("GTKWAVE | ERROR: rc for '%s' = %d\n", sysname, rc);
				unlink(*GLOBALS->fileselbox_text);
				}
			free_2(sysname);
printf("'%s'\n", zname);
			unlink(zname);
			}

		free_2(zname);
                }
        }  
}

static void
mif_print_cleanup(GtkWidget *widget, gpointer data)
{
FILE *wave;

if(GLOBALS->filesel_ok)
        {
        DEBUG(printf("MIF Print Fini: %s\n", *GLOBALS->fileselbox_text));
                
        if(!(wave=fopen(*GLOBALS->fileselbox_text,"wb")))
                {
                fprintf(stderr, "Error opening MIF output file '%s' for writing.\n",*GLOBALS->fileselbox_text);
                perror("Why");
                errno=0;
                }
                else
                {
		print_mif_image(wave,px[GLOBALS->page_size_type_renderopt_c_1],py[GLOBALS->page_size_type_renderopt_c_1]);
                fclose(wave);
                }
        }  
}


static void ok_callback(void)
{
GLOBALS->ps_fullpage=GLOBALS->render_mutex_renderopt_c_1[0];
if(GLOBALS->target_mutex_renderopt_c_1[0])
	{
	fileselbox("Print To PDF File",&GLOBALS->filesel_print_pdf_renderopt_c_1,GTK_SIGNAL_FUNC(pdf_print_cleanup), GTK_SIGNAL_FUNC(NULL), "*.pdf", 1);
	}
else
if(GLOBALS->target_mutex_renderopt_c_1[1])
	{
	fileselbox("Print To PS File",&GLOBALS->filesel_print_ps_renderopt_c_1,GTK_SIGNAL_FUNC(ps_print_cleanup), GTK_SIGNAL_FUNC(NULL), "*.ps", 1);
	}
else
/* if(GLOBALS->target_mutex_renderopt_c_1[2]) */
	{
	fileselbox("Print To MIF File",&GLOBALS->filesel_print_mif_renderopt_c_1,GTK_SIGNAL_FUNC(mif_print_cleanup), GTK_SIGNAL_FUNC(NULL), "*.fm", 1);
	}
}

static void destroy_callback(GtkWidget *widget, GtkWidget *nothing)
{
  GLOBALS->is_active_renderopt_c_3=0;
  gtk_widget_destroy(GLOBALS->window_renderopt_c_6);
  GLOBALS->window_renderopt_c_6 = NULL;
}


void renderbox(char *title)
{
    GtkWidget *menu, *menuitem, *optionmenu;
    GSList *group; 
    GtkWidget *vbox, *hbox, *small_hbox;
    GtkWidget *button1, *button2;
    int i;

    if(GLOBALS->wave_script_args)
        {
        char *s1 = NULL;
        char *s2 = NULL;
        char *s3 = NULL;

        while((!s1)&&(GLOBALS->wave_script_args->curr)) s1 = wave_script_args_fgetmalloc_stripspaces(GLOBALS->wave_script_args);
        while((!s2)&&(GLOBALS->wave_script_args->curr)) s2 = wave_script_args_fgetmalloc_stripspaces(GLOBALS->wave_script_args);
        while((!s3)&&(GLOBALS->wave_script_args->curr)) s3 = wave_script_args_fgetmalloc_stripspaces(GLOBALS->wave_script_args);

        if(s1 && s2 && s3)
                {
		memset(GLOBALS->target_mutex_renderopt_c_1, 0, 2); GLOBALS->target_mutex_renderopt_c_1[0] = 1; /* PS */
		for(i=0;i<3;i++)
			{
			if(!strcmp(s1, render_targets[i]))
				{
				fprintf(stderr, "GTKWAVE | Print using '%s'\n",  render_targets[i]);
				memset(GLOBALS->target_mutex_renderopt_c_1, 0, 3); GLOBALS->target_mutex_renderopt_c_1[i] = 1; break;
				}
			}

		memset(GLOBALS->page_mutex_renderopt_c_1, 0, 5); GLOBALS->page_mutex_renderopt_c_1[0] = 1; /* 8.5 x 11 */
		GLOBALS->page_size_type_renderopt_c_1 = 0;
		for(i=0;i<5;i++)
			{
			if(!strcmp(s2, page_size[i]))
				{
				fprintf(stderr, "GTKWAVE | Print using '%s'\n",  page_size[i]);
				memset(GLOBALS->page_mutex_renderopt_c_1, 0, 5); GLOBALS->page_mutex_renderopt_c_1[i] = 1; 
				GLOBALS->page_size_type_renderopt_c_1 = i;
				break;
				}
			}

		memset(GLOBALS->render_mutex_renderopt_c_1, 0, 3); GLOBALS->render_mutex_renderopt_c_1[0] = 1; /* Full */
		for(i=0;i<2;i++)
			{
			if(!strcmp(s3, render_type[i]))
				{
				fprintf(stderr, "GTKWAVE | Print using '%s'\n",  render_type[i]);
				memset(GLOBALS->render_mutex_renderopt_c_1, 0, 3); GLOBALS->render_mutex_renderopt_c_1[i] = 1; break;
				}
			}

		free_2(s1); free_2(s2); free_2(s3);
		ok_callback();
                }
                else
                {
		fprintf(stderr, "Missing script entries for renderbox, exiting.\n");
		exit(255);
                }

        return;
        }



    if(GLOBALS->is_active_renderopt_c_3) 
	{
	gdk_window_raise(GLOBALS->window_renderopt_c_6->window);
	return;
	}
    GLOBALS->is_active_renderopt_c_3=1;

    /* create a new window */
    GLOBALS->window_renderopt_c_6 = gtk_window_new(GLOBALS->disable_window_manager ? GTK_WINDOW_POPUP : GTK_WINDOW_TOPLEVEL);
    install_focus_cb(GLOBALS->window_renderopt_c_6, ((char *)&GLOBALS->window_renderopt_c_6) - ((char *)GLOBALS));

    gtk_window_set_title(GTK_WINDOW (GLOBALS->window_renderopt_c_6), title);
    gtk_widget_set_usize( GTK_WIDGET (GLOBALS->window_renderopt_c_6), 420, -1); 
    gtkwave_signal_connect(GTK_OBJECT (GLOBALS->window_renderopt_c_6), "delete_event",(GtkSignalFunc) destroy_callback, NULL);
    gtk_window_set_policy(GTK_WINDOW(GLOBALS->window_renderopt_c_6), FALSE, FALSE, FALSE);

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (GLOBALS->window_renderopt_c_6), vbox);
    gtk_widget_show (vbox);

    small_hbox = gtk_hbox_new (TRUE, 0);
    gtk_widget_show (small_hbox);

    menu = gtk_menu_new ();
    group=NULL;

    for(i=0;i<3;i++)
	{
    	menuitem = gtk_radio_menu_item_new_with_label (group, render_targets[i]);
    	group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (menuitem));
    	gtk_menu_append (GTK_MENU (menu), menuitem);
    	gtk_widget_show (menuitem);
        gtkwave_signal_connect(GTK_OBJECT (menuitem), "activate", GTK_SIGNAL_FUNC(render_clicked), (void *)((long)i));
	GLOBALS->target_mutex_renderopt_c_1[i]=0;
	}

	GLOBALS->target_mutex_renderopt_c_1[0]=1;	/* "ps" */

	optionmenu = gtk_option_menu_new ();
	gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu), menu);
	gtk_box_pack_start (GTK_BOX (small_hbox), optionmenu, TRUE, FALSE, 0);
	gtk_widget_show (optionmenu);

    menu = gtk_menu_new ();
    group=NULL;

    for(i=0;i<5;i++)
	{
    	menuitem = gtk_radio_menu_item_new_with_label (group, page_size[i]);
    	group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (menuitem));
    	gtk_menu_append (GTK_MENU (menu), menuitem);
    	gtk_widget_show (menuitem);
        gtkwave_signal_connect(GTK_OBJECT (menuitem), "activate", GTK_SIGNAL_FUNC(pagesize_clicked), (void *)((long)i));
	GLOBALS->page_mutex_renderopt_c_1[i]=0;
	}

	GLOBALS->page_mutex_renderopt_c_1[0]=1;	/* "letter" */

	optionmenu = gtk_option_menu_new ();
	gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu), menu);
	gtk_box_pack_start (GTK_BOX (small_hbox), optionmenu, TRUE, FALSE, 0);
	gtk_widget_show (optionmenu);


	gtk_box_pack_start (GTK_BOX (vbox), small_hbox, FALSE, FALSE, 0);

    menu = gtk_menu_new ();
    group=NULL;

    for(i=0;i<2;i++)
	{
    	menuitem = gtk_radio_menu_item_new_with_label (group, render_type[i]);
    	group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (menuitem));
    	gtk_menu_append (GTK_MENU (menu), menuitem);
    	gtk_widget_show (menuitem);
        gtkwave_signal_connect(GTK_OBJECT (menuitem), "activate", GTK_SIGNAL_FUNC(rendertype_clicked), (void *)((long)i));
	GLOBALS->render_mutex_renderopt_c_1[i]=0;
	}

	GLOBALS->render_mutex_renderopt_c_1[0]=1;	/* "full" */

	optionmenu = gtk_option_menu_new ();
	gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu), menu);
	gtk_box_pack_start (GTK_BOX (small_hbox), optionmenu, TRUE, FALSE, 0);
	gtk_widget_show (optionmenu);


    hbox = gtk_hbox_new (TRUE, 0);
    gtk_widget_show (hbox);

    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

    button1 = gtk_button_new_with_label ("Select Output File");
    gtk_widget_set_usize(button1, 100, -1);
    gtkwave_signal_connect(GTK_OBJECT (button1), "clicked", GTK_SIGNAL_FUNC(ok_callback), NULL);
    gtk_widget_show (button1);
    gtk_container_add (GTK_CONTAINER (hbox), button1);
    GTK_WIDGET_SET_FLAGS (button1, GTK_CAN_DEFAULT);
    gtkwave_signal_connect_object (GTK_OBJECT (button1), "realize", (GtkSignalFunc) gtk_widget_grab_default, GTK_OBJECT (button1));

    button2 = gtk_button_new_with_label ("Exit");
    gtk_widget_set_usize(button2, 100, -1);
    gtkwave_signal_connect(GTK_OBJECT (button2), "clicked", GTK_SIGNAL_FUNC(destroy_callback), NULL);
    GTK_WIDGET_SET_FLAGS (button2, GTK_CAN_DEFAULT);
    gtk_widget_show (button2);
    gtk_container_add (GTK_CONTAINER (hbox), button2);

    gtk_widget_show(GLOBALS->window_renderopt_c_6);
}

