#ifndef DISPLAY_H
#define DISPLAY_H

#ifdef USE_GTK

GtkImage * display_pixbuf( GdkPixbuf * pixbuf, const char * name, GtkWidget * target );
GtkImage * display_rgb_buffer( rgb_buffer_t * buffer, const char * name, GtkWidget * target );
GtkImage * display_mono_buffer( mono_buffer_t * buffer, const char * name, GtkWidget * target );

#endif

#endif
