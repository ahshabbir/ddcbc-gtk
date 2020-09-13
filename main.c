#include <gtk/gtk.h>

#include "ddcbc-api.c"

// brightness_setter is a struct used for passing the ddcbc bundle and
// the display number of a particular gtk scale, so that the brightness
// can be set directly in the handler.
typedef struct brightness_setter {
	ddcbc_bundle *bun;
	unsigned int dispno;
} brightness_setter;

// set_brightness is the handler that it called when the user releases the
// mouse on the brightness control.
gboolean
set_brightness(GtkWidget *widget, GdkEvent *event, gpointer data) 
{
	brightness_setter *bset = data;
	ddcbc_bundle_set_brightness(bset->bun, bset->dispno, 
		gtk_range_get_value(GTK_RANGE(widget)));
	return FALSE;
}

static void
activate (GtkApplication *app, gpointer data)
{
	// TODO: Refactor window and grid to a *.ui file and use GTKBuilder
	GtkWidget *window;
	GtkWidget *grid;

	ddcbc_bundle *bun = data;
	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "DDC/CI Monitor Brightness Control");
	gtk_window_set_default_size (GTK_WINDOW (window), 400, 400);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	
	grid = gtk_grid_new();
	gtk_container_add (GTK_CONTAINER (window), grid);

	GtkWidget **labels;
	GtkWidget **scales;

	labels = malloc(sizeof(gpointer) * bun->supported_count);
	scales = malloc(sizeof(gpointer) * bun->supported_count);
	for (unsigned int i = 0; i < bun->ct; i++) {
		char buffer[128];
		DDCA_Display_Info info = bun->dlist->info[i];
		sprintf(buffer, "Display No. %u, Model: %s", 
			info.dispno, info.model_name);
		
		if (bun->supported[i]) {
			labels[i] = gtk_label_new(buffer);

			// For the first monitor, attach it directly onto the grid,
			// other times, attach it belove the previos scale.
			if (i == 0)
				gtk_grid_attach(GTK_GRID(grid), labels[i], 0, 0, 1, 1);
			else
				gtk_grid_attach_next_to(GTK_GRID(grid), labels[i], 
					scales[i - 1], GTK_POS_BOTTOM, 1, 1);
			
			// Get the brightness of the i+1th moniter and set the scale to it.
			brightness_stats bs = ddcbc_bundle_get_brightness(bun, i + 1);
			scales[i] = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 
				0, bs.max_val, 5);
			gtk_range_set_value(GTK_RANGE(scales[i]), bs.cur_val);
			
			// Pass bundle reference and display number into the callback:
			brightness_setter *bset = malloc(sizeof(gpointer));
			bset->bun = bun;
			bset->dispno = (unsigned int) (i + 1);
			g_signal_connect(scales[i], "button-release-event",  G_CALLBACK(set_brightness), bset);

			gtk_grid_attach_next_to(GTK_GRID(grid), scales[i], 
				labels[i], GTK_POS_BOTTOM, 1, 1);
		}
	}

	gtk_widget_show_all (window);
}

int
main (int argc, char **argv)
{
	GtkApplication *app;
	int status;

	ddcbc_bundle bun = ddcbc_bundle_init();
	
	app = gtk_application_new ("org.gtk.ddcbc-gtk", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "activate", G_CALLBACK (activate), &bun);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	ddcbc_bundle_free(&bun);
	return status;
}