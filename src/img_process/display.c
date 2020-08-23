#ifdef USE_GTK
#include <gtk/gtk.h>
#include <glib.h>

#include "util.h"
#include "data_types.h"

GtkImage * display_pixbuf( GdkPixbuf * pixbuf, const char * name, GtkWidget * target )
{
	GtkWidget * window;
	GtkWidget * image;

	if( target == NULL )
	{
		window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
		gtk_window_set_title( (GtkWindow*)window, name);

		image = gtk_image_new_from_pixbuf(pixbuf);

		gtk_container_add( GTK_CONTAINER(window), image );

		gtk_widget_show_all( window );
	}
	else
	{
		gtk_image_set_from_pixbuf(GTK_IMAGE(target), pixbuf);
		image = target;
	}

	return (GtkImage*)image;
}

GtkImage * display_rgb_buffer( rgb_buffer_t * buffer, const char * name, GtkWidget * target )
{
	GdkPixbuf * pixbuf = c_rgb_buffer_pixbuf(buffer);

	return display_pixbuf(pixbuf, name, target);
}

GtkImage * display_mono_buffer( mono_buffer_t * buffer, const char * name, GtkWidget * target )
{
	rgb_buffer_t * rgb = c_mono_buffer_rgb_buffer(buffer, NULL);

	GtkImage * img = display_rgb_buffer(rgb, name, target);

	release_rgb_buffer(rgb);

	return img;
}
#endif
