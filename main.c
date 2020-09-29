#include <gtk/gtk.h>
#include "ddcbc-api/ddcbc-api.c"

typedef struct display_section {
	GtkWidget *label;
	GtkWidget *scale;
} display_section;

gboolean
set_brightness(GtkWidget *widget, GdkEvent *event, gpointer data) 
{
	ddcbc_display *disp = data;
	guint16 new_val = gtk_range_get_value(GTK_RANGE(widget));         
	DDCBC_Status rc = ddcbc_display_set_brightness(disp, new_val);
	if (rc != 0)
		g_printerr(
			"An error occured when setting the brightness of display no"
			"%d to %u.Code: %d\n", 
			disp->info.dispno, new_val, rc);	
	return FALSE;
}

// display_section_init initializes a display section from a disp object.
display_section*
display_section_init (ddcbc_display *disp) 
{
	display_section *ds = malloc(sizeof(display_section));
	
	ds->label = gtk_label_new (disp->info.model_name);
	gtk_widget_set_hexpand (ds->label, FALSE);
	gtk_label_set_xalign (GTK_LABEL (ds->label), 0.0);

	ds->scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 
		disp->max_val, 1);
	gtk_widget_set_halign (ds->scale, 0.0);
	gtk_range_set_value (GTK_RANGE(ds->scale), disp->last_val);
	gtk_widget_set_hexpand (ds->scale, TRUE);
	g_signal_connect (ds->scale, "button-release-event", 
		G_CALLBACK (set_brightness), disp);
	
	return ds;
}

// display_section_attach_next_to attaches the section a grid, packed by a
// specified sibling. If sibling is null, the section will be attached 
// to the top of the grid.
static void
display_section_attach_next_to(display_section *ds, GtkGrid *grid, 
	display_section *sibling) 
{
	
	if (sibling == NULL)
		gtk_grid_attach(GTK_GRID (grid), ds->label, 0, 0, 1, 1);
	else
		gtk_grid_attach_next_to(GTK_GRID(grid), ds->label, 
					sibling->scale, GTK_POS_BOTTOM, 1, 1);
	gtk_grid_attach_next_to(GTK_GRID(grid), ds->scale, ds->label,
		GTK_POS_BOTTOM, 1, 1);
}

static void
activate (GtkApplication *app, gpointer data)
{
	GtkWidget *window;
	GtkWidget *grid;

	ddcbc_display_list *dlist = data;

	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "Brightness Control");
	gtk_window_set_default_size (GTK_WINDOW(window), 300, 200);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	
	grid = gtk_grid_new();
	gtk_container_add (GTK_CONTAINER (window), grid);
		
	GtkCssProvider *cssProvider = gtk_css_provider_new();
	gtk_css_provider_load_from_path(cssProvider, "style.css", NULL);
	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
	
	display_section **sections = malloc(dlist->ct);
	display_section *sibling = NULL;
	for (guint it = 0; it < dlist->ct; it++) {
		ddcbc_display *disp = ddcbc_display_list_get(dlist, it);
		sections[it] = display_section_init(disp);
		display_section_attach_next_to(sections[it], GTK_GRID (grid), sibling);
		sibling = sections[it];
	}

	gtk_widget_show_all (window);
}

int
main(int argc, char **argv)
{
	GtkApplication *app;
	int status;

	ddcbc_display_list dlist = ddcbc_display_list_init(FALSE);

	app = gtk_application_new("org.gtk.ddcbc-gtk", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "activate", G_CALLBACK (activate), &dlist);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref(app);

	ddcbc_display_list_free(&dlist);
	return status;
}