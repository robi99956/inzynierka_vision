#include "util.h"
#include "data_types.h"

// inaczej niż zwykle - out == NULL sprawia, że pracujemy na oryginale
mono_buffer_t * mono_buffer_threshold( mono_buffer_t * in, mono_pixel_t min_white, mono_buffer_t * out )
{
	uint16_t w = in->info.w;
	uint16_t h = in->info.h;

	if( out == NULL ) out = in;

	for( uint16_t y = 0; y<h; y++ )
	{
		for( uint16_t x = 0; x<w; x++ )
		{
			uint32_t idx = y*w+x;
			if( in->data[idx] < min_white )
				out->data[idx] = 0;
			else
				out->data[idx] = (mono_pixel_t)-1;
		}
	}

	return out;
}

mono_buffer_t * rgb_buffer_threshold( rgb_buffer_t * in, rgb_pixel_t color, uint8_t tolerance, mono_buffer_t * out )
{
	uint16_t w = in->info.w;
	uint16_t h = in->info.h;

	if( out == NULL ) out = create_mono_buffer(w, h);

	uint32_t pixel_cnt = w*h;
	for( uint32_t i=0; i<pixel_cnt; i++ )
	{
		rgb_pixel_t pixel = in->data[i];
		int16_t r_diff = pixel.R-color.R;
		int16_t g_diff = pixel.G-color.G;
		int16_t b_diff = pixel.B-color.B;

		if( ABS(r_diff) < tolerance && ABS(g_diff) < tolerance && ABS(b_diff) < tolerance )
		{
			out->data[i] = (mono_pixel_t)-1;
		}
	}

	return out;
}

#if 0
#define EXPANSION_LEN 3

static bool expand_direction( mono_buffer_t * in, int16_t x, int16_t y, int16_t x_dir, int16_t y_dir, mono_pixel_t color, mono_buffer_t * out )
{
	uint16_t w = in->info.w;
	uint16_t h = in->info.h;

	// badany piksel nie ma być rozszerzony
	if( in->data[x + y*w] != color ) return 0;
#if 0
	x += x_dir;
	y += y_dir;

	if( x < 0 || x > w ) return 0;
	if( y < 0 || y > h ) return 0;

	int16_t x2 = x + x_dir;
	int16_t y2 = y + y_dir;

	if( x2 < 0 || x2 > w ) return 0;
	if( y2 < 0 || y2 > h ) return 0;
	
	if( in->data[x + y*w] != color && in->data[x2 + y2*w] != color ) 
	{
		out->data[x + y*w] = color;
		return 1;
	}

	return 0;
#else
	x += x_dir;
	y += y_dir;
	int16_t x2 = x;
	int16_t y2 = y;

	for( uint8_t i=0; i<EXPANSION_LEN; i++ )
	{
		if( x2 < 0 || x2 > w ) return 0;
		if( y2 < 0 || y2 > h ) return 0;

		if( in->data[x2+y2*w] == color ) return 0;

		x2 += x_dir;
		y2 += y_dir;
	}

	out->data[x + y*w] = color;
	return 1;
#endif
}

static bool expand_mono_pixel( mono_buffer_t * in, int16_t x, int16_t y, mono_pixel_t color, mono_buffer_t * out )
{
	bool expanded = 0;

	expanded |= expand_direction(in, x, y, 0, 1, color, out);
	expanded |= expand_direction(in, x, y, 0, -1, color, out);
	expanded |= expand_direction(in, x, y, 1, 0, color, out);
	expanded |= expand_direction(in, x, y, -1, 0, color, out);

	return expanded;
}

mono_buffer_t * mono_buffer_color_expand( mono_buffer_t * in, mono_pixel_t color, mono_buffer_t * out )
{
	uint16_t w = in->info.w;
	uint16_t h = in->info.h;

	mono_buffer_t * buf = create_mono_buffer(w, h);
	memcpy(buf->data, in->data, w*h);

	if( out == NULL ) out = in;
	else
	{
		memcpy(out, in, sizeof(mono_buffer_t));
		memcpy(out->data, in->data, w*h*sizeof(mono_pixel_t));
	}

	mono_buffer_t * buffers[2] = {out, buf};
	uint8_t buffer_idx = 0;

	uint32_t expanded_pixels = 0;

	do
	{
		expanded_pixels = 0;	
		for( uint16_t y = 0; y<h; y++ )
		{
			for( uint16_t x = 0; x<w; x++ )
			{
				expanded_pixels += expand_mono_pixel( buffers[buffer_idx], x, y, color, buffers[!buffer_idx] );
			}
		}

		memcpy(buffers[buffer_idx]->data, buffers[!buffer_idx]->data, w*h*sizeof(mono_pixel_t));
		buffer_idx ^= 1;
			
	} while( expanded_pixels != 0 );

	// out nie jest aktualnym buforem
	if( buffer_idx == 0 )
	{
		memcpy(out->data, buf->data, w*h*sizeof(mono_pixel_t));
	}

	release_mono_buffer(buf);

	return out;
}
#endif
