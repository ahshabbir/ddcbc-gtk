#include <gtk/gtk.h>
#include "ddcbc-api/ddcbc-api.c"

typedef struct display_section {
	GtkWidget *label;
	GtkWidget *scale;
	GtkWidget *seperator;
	ddcbc_display *disp;
} display_section;

const int MARGIN_UNIT = 8;

typedef struct set_brightness_data {
	display_section **disp_sections;
	guint disp_sections_len;
	guint index;
	GtkWidget *linkBtn;
} set_brightness_data;

// set_brightness is the event handler that changes the brightness of a ddcbc
// display passed in through 'data'. 
gboolean
set_brightness(GtkWidget *widget, GdkEvent *event, gpointer data) 
{
	
	set_brightness_data *dat = data;
	display_section *disp_sec = dat->disp_sections[dat->index]; 
	ddcbc_display *disp = disp_sec->disp;

	guint16 new_val = gtk_range_get_value(GTK_RANGE(widget));         
	DDCBC_Status rc = ddcbc_display_set_brightness(disp, new_val);

	// If the link checkbox is checked, set all other displays to the same value:
	// TODO: It might be the case that the new value will exceed the max of other
	// displays. 
	gboolean linkSlides = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dat->linkBtn));
	if (linkSlides) {
		for (guint it = 0; it < dat->disp_sections_len; it++) {
			GtkWidget *scale = dat->disp_sections[it]->scale;
			gtk_range_set_value(GTK_RANGE(scale), new_val);

			ddcbc_display *disp = dat->disp_sections[it]->disp;
			ddcbc_display_set_brightness(disp, new_val);
		}
	}

	if (rc == 1)
		g_printerr(
			"Partial sucess in setting the brightness of display no"
			" %d to %u. Code: %d\n", 
			disp->info.dispno, new_val, rc);	
	else if (rc != 0)
		g_printerr(
			"An error occured when setting the brightness of display no"
			" %d to %u. Code: %d\n", 
			disp->info.dispno, new_val, rc);	
	return FALSE;
}

// display_section_init initializes a display section from a disp object.
display_section*
display_section_init (ddcbc_display *disp, set_brightness_data *dat) 
{
	display_section *ds = malloc(sizeof(display_section));
	ds->disp = disp;

	ds->label = gtk_label_new (disp->info.model_name);
	gtk_widget_set_hexpand (ds->label, FALSE);
	gtk_widget_set_valign(ds->label, GTK_ALIGN_CENTER);
	gtk_label_set_xalign (GTK_LABEL (ds->label), 0.0);
	gtk_widget_set_margin_start(ds->label, MARGIN_UNIT);
	gtk_widget_set_margin_top(ds->label, 2 * MARGIN_UNIT);

	ds->scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 
		disp->max_val, 1);
	gtk_widget_set_halign (ds->scale, 0.0);
	gtk_range_set_value (GTK_RANGE(ds->scale), disp->last_val);
	gtk_scale_set_value_pos(GTK_SCALE(ds->scale), GTK_POS_RIGHT);
	gtk_widget_set_hexpand (ds->scale, TRUE);
	gtk_widget_set_margin_top(ds->scale, MARGIN_UNIT);
	gtk_widget_set_margin_bottom(ds->scale, MARGIN_UNIT);
	
	g_signal_connect (ds->scale, "button-release-event", 
		G_CALLBACK (set_brightness), dat);

	ds->seperator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_set_margin_start(ds->seperator, 0);
	gtk_widget_set_halign(ds->seperator, 0);
	return ds;
}

// display_section_attach_next_to attaches the section a grid, packed by a
// specified sibling. If sibling is null, the section will be attached 
// to the top of the grid.
static void
display_section_attach_next_to(display_section *ds, GtkGrid *grid, 
	display_section *sibling) 
{
	if (sibling == NULL) {
		gtk_grid_attach(GTK_GRID (grid), ds->label, 1, 0, 1, 1);
	} else {
		gtk_grid_attach_next_to(GTK_GRID(grid), ds->label, 
					sibling->seperator, GTK_POS_BOTTOM, 1, 1);
	}
	gtk_grid_attach_next_to(GTK_GRID(grid), ds->scale, ds->label,
		GTK_POS_BOTTOM, 1, 1);
	gtk_grid_attach_next_to(GTK_GRID(grid), ds->seperator, ds->scale, 
		GTK_POS_BOTTOM, 1, 1);
	
	GtkWidget *icon;
	icon = gtk_image_new_from_icon_name("video-display", GTK_ICON_SIZE_BUTTON);
	gtk_widget_set_margin_start(icon, MARGIN_UNIT);
	gtk_widget_set_margin_top(icon, 2 * MARGIN_UNIT);
	gtk_widget_set_valign(icon, GTK_ALIGN_CENTER);
	gtk_grid_attach_next_to(GTK_GRID(grid), icon, ds->label, GTK_POS_LEFT, 1, 1);
	
}

typedef struct link_scales_dat {
	display_section **sections;
	guint len;
} link_scales_dat;

void
link_scales (GtkToggleButton *tb, gpointer data) {
	link_scales_dat *dat = data;

	guint max_scale = 0;
	for (guint it = 0; it < dat->len; it++) {
		guint val = gtk_range_get_value(GTK_RANGE(dat->sections[it]->scale));
		max_scale = val > max_scale ? val : max_scale;
	}

	for (guint it = 0; it < dat->len; it++) {
		gtk_range_set_value(GTK_RANGE(dat->sections[it]->scale), max_scale);
		ddcbc_display *disp = dat->sections[it]->disp;
		DDCBC_Status rc = ddcbc_display_set_brightness(disp, max_scale);
		
		if (rc == 1)
			g_printerr(
				"Partial success in setting the brightness of display no"
				" %d to %u while linking displays. Code: %d\n", 
				disp->info.dispno, max_scale, rc);	
		else if (rc != 0)
			g_printerr(
				"An error occured when setting the brightness of display no"
				" %d to %u while linking displays. Code: %d\n", 
				disp->info.dispno, max_scale, rc);	
	}
}

static void
activate (GtkApplication *app, gpointer data)
{
	GtkWidget *window;
	GtkWidget *grid;
	GtkWindow *window_icon;
	GtkWidget *linkBtn;

	ddcbc_display_list *dlist = data;

	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "DDC Brightness Control");
	gtk_window_set_default_size (GTK_WINDOW(window), 300, 0);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	
	linkBtn = gtk_check_button_new_with_label("link");
	
	grid = gtk_grid_new();
	gtk_container_add (GTK_CONTAINER (window), grid);
	display_section **sections = malloc(dlist->ct);
	display_section *sibling = NULL;
	for (guint it = 0; it < dlist->ct; it++) {
		ddcbc_display *disp = ddcbc_display_list_get(dlist, it);
		
		set_brightness_data *dat = malloc(sizeof(set_brightness_data));
		dat->disp_sections = sections;
		dat->index = it;
		dat->linkBtn = linkBtn;
		dat->disp_sections_len = dlist->ct;

		sections[it] = display_section_init(disp, dat);
		display_section_attach_next_to(sections[it], GTK_GRID (grid), sibling);
		sibling = sections[it];
	}

	// Attach link button to the seperator of the sibling:
	GtkWidget *gtk_sibling = sibling->seperator;
	gtk_grid_attach_next_to(GTK_GRID(grid), linkBtn, gtk_sibling, GTK_POS_BOTTOM, 1, 1);
	link_scales_dat *dat = malloc(sizeof(link_scales_dat));
	dat->sections = sections;
	dat->len = dlist->ct;

	g_signal_connect(linkBtn, "toggled", G_CALLBACK(link_scales), dat);

	gtk_widget_show_all (window);
}

int
main(int argc, char **argv)
{
	GtkApplication *app;
	int status;

	ddcbc_display_list dlist = ddcbc_display_list_init(FALSE);

	if (dlist.ct <= 0) {
		g_printerr("No supported displays found. Please check if ddcutil is " 
			" properly installed and/or whether you have any "
			" supported monitors.");
		return 1;
	}

	app = gtk_application_new("org.gtk.ddcbc-gtk", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "activate", G_CALLBACK (activate), &dlist);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref(app);

	ddcbc_display_list_free(&dlist);
	return status;
}