#include "util.h"
#include "data_types.h"

#ifdef USE_GTK
#include <gtk/gtk.h>
#include <glib.h>
rgb_buffer_t * c_pixbuf_rgb_buffer( GdkPixbuf * pixbuf, rgb_buffer_t * out )
{
	uint8_t * pixels = gdk_pixbuf_get_pixels(pixbuf);
	int w = gdk_pixbuf_get_width(pixbuf);
	int h = gdk_pixbuf_get_height(pixbuf);

	int pixel_cnt = w*h;

	if( out == NULL ) out = create_rgb_buffer(w, h);

	for( int i=0; i<pixel_cnt; i++ )
	{
		out->data[i].R = pixels[3*i];		
		out->data[i].G = pixels[3*i+1];
		out->data[i].B = pixels[3*i+2];
	}

	return out;
}

static void c_release_pixel_buffer( guchar * pixels, gpointer data )
{
	UNUSED(data);

	RELEASE(pixels);
}

GdkPixbuf * c_rgb_buffer_pixbuf( rgb_buffer_t * buffer )
{
	int w = buffer->info.w;
	int h = buffer->info.h;
	uint32_t pixel_cnt = w*h;
	guchar * pixels = TABLE(pixel_cnt*3, guchar);
	guchar * pixels_ptr = pixels;

	for( uint32_t i=0; i<pixel_cnt; i++ )
	{
		pixels_ptr = pixels + 3*i;

		*(pixels_ptr) = buffer->data[i].R;
		*(pixels_ptr+1) = buffer->data[i].G;
		*(pixels_ptr+2) = buffer->data[i].B;
	}

	GdkPixbuf * pixbuf = gdk_pixbuf_new_from_data(pixels, GDK_COLORSPACE_RGB, 0, 8, 
				w, h, w*3, 
				c_release_pixel_buffer, NULL);

	return pixbuf;
}
#endif

mono_buffer_t * c_rgb_buffer_bw_buffer( rgb_buffer_t * rgb, mono_pixel_t min_white, mono_buffer_t * out )
{
	uint16_t w = rgb->info.w;
	uint16_t h = rgb->info.h;

	if( out == NULL ) out = create_mono_buffer(w, h);

	for( uint16_t y = 0; y<h; y++ )
	{
		for( uint16_t x = 0; x<w; x++ )
		{
			uint32_t idx = y*w+x;

			mono_pixel_t brightness = (rgb->data[idx].R +
				rgb->data[idx].B + rgb->data[idx].G)/3;
			if( brightness > min_white )
			{
				out->data[idx] = (mono_pixel_t)-1;
			}
			else
			{
				out->data[idx] = 0;
			}

		}
	}	

	return out;
}

mono_buffer_t * c_rgb_buffer_mono_buffer( rgb_buffer_t * rgb, mono_buffer_t * out )
{
	uint16_t w = rgb->info.w;
	uint16_t h = rgb->info.h;

	if( out == NULL ) out = create_mono_buffer(w, h);

	for( uint16_t y = 0; y<h; y++ )
	{
		for( uint16_t x = 0; x<w; x++ )
		{
			uint32_t idx = y*w+x;

			out->data[idx] = (rgb->data[idx].R+rgb->data[idx].G+rgb->data[idx].B)/3;
		}
	}	

	return out;
}

rgb_buffer_t * c_mono_buffer_rgb_buffer( mono_buffer_t * mono, rgb_buffer_t * out )
{
	uint16_t w = mono->info.w;
	uint16_t h = mono->info.h;

	if( out == NULL ) out = create_rgb_buffer(w, h);

	for( uint16_t y=0; y<h; y++ )
	{
		for( uint16_t x=0; x<w; x++ )
		{
			uint32_t idx = y*w+x;

			out->data[idx].R = out->data[idx].G = out->data[idx].B = mono->data[idx]; 
		}
	}

	return out;
}

mono_buffer_t * c_yuv_buffer_mono_buffer( yuv_buffer_t * yuv, mono_buffer_t * out )
{
	uint16_t w = yuv->info.w;
	uint16_t h = yuv->info.h;

	if( out == NULL ) out = create_mono_buffer(w, h);

	uint8_t * y_ptr = (uint8_t*)yuv->data;
	uint32_t pixel_cnt = w*h;
	for( uint32_t i=0; i<pixel_cnt; i++ )
	{
		out->data[i] = *y_ptr;
		y_ptr += 2;
	}

	return out;
}

// Color space conversion for RGB
//#define GET_R_FROM_YUV(y, u, v) ((298*y+409*v+128)>>8)
//#define GET_G_FROM_YUV(y, u, v) ((298*y-100*u-208*v+128)>>8)
//#define GET_B_FROM_YUV(y, u, v) ((298*y+516*u+128)>>8)
//#define GET_R_FROM_YUV(y, u, v) (1.164*(y-16)+1.596*(v-128))
//#define GET_G_FROM_YUV(y, u, v) (1.164*(y-16)-0.813*(v-128)-0.391*(u-128))
//#define GET_B_FROM_YUV(y, u, v) (1.164*(y-16)+2.018*(u-128))
#define GET_R_FROM_YUV(y, u, v) ((1164*(y-16)+1596*(v-128))/1000)
#define GET_G_FROM_YUV(y, u, v) ((1164*(y-16)-813*(v-128)-391*(u-128))/1000)
#define GET_B_FROM_YUV(y, u, v) ((1164*(y-16)+2018*(u-128))/1000)

rgb_buffer_t * c_yuv_buffer_rgb_buffer( yuv_buffer_t * yuv, rgb_buffer_t * out )
{
	uint16_t w = yuv->info.w;
	uint16_t h = yuv->info.h;

	if( out == NULL ) out = create_rgb_buffer(w, h);

	uint32_t pixel_cnt = w*h;	
	for( uint32_t i=0, j=0; i<pixel_cnt; i += 2, j++ )
	{
		rgb_pixel_t * pixels = &out->data[i];
		yuv_macropixel_t * macropixel = &yuv->data[j];

		pixels[0].R = saturate( GET_R_FROM_YUV( macropixel->y0, macropixel->u0, macropixel->v0 ) );
		pixels[0].G = saturate( GET_G_FROM_YUV( macropixel->y0, macropixel->u0, macropixel->v0 ) );
		pixels[0].B = saturate( GET_B_FROM_YUV( macropixel->y0, macropixel->u0, macropixel->v0 ) );

		pixels[1].R = saturate( GET_R_FROM_YUV( macropixel->y1, macropixel->u0, macropixel->v0 ) );
		pixels[1].G = saturate( GET_G_FROM_YUV( macropixel->y1, macropixel->u0, macropixel->v0 ) );
		pixels[1].B = saturate( GET_B_FROM_YUV( macropixel->y1, macropixel->u0, macropixel->v0 ) );
	}

	return out;
}






