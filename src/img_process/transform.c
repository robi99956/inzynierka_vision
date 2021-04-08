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

mono_buffer_t * mono_buffer_expand_black( mono_buffer_t * in, mono_pixel_t expand_start, 
				mono_pixel_t expand_stop, uint8_t steps, mono_buffer_t * out )
{
	uint16_t w = in->info.w;
	uint16_t h = in->info.h;

	if( out == NULL ) out = create_mono_buffer(w, h);
	mono_buffer_t * buf = create_mono_buffer(w, h);

	// 2 kopie pierwotnego obrazu
	memcpy( buf->data, in->data, w*h*sizeof(mono_pixel_t) );
	memcpy( out->data, in->data, w*h*sizeof(mono_pixel_t) );

	for( uint8_t i=0; i<steps; i++ )
	{
			mono_pixel_t current_color = expand_start + ((expand_stop-expand_start)*i)/steps;

			for( uint16_t x=1; x<w-1; x++ )
			{
					for( uint16_t y=1; y<h-1; y++ )
					{
						uint32_t left_idx = y*w+(x-1);
						uint32_t right_idx = y*w+(x+1);
						uint32_t top_idx = (y-1)*w+x;
						uint32_t bottom_idx = (y+1)*w+x;
						uint32_t center_idx = y*w+x;

						if( (buf->data[ left_idx ] != MONO_PIXEL_WHITE ||
							buf->data[ right_idx ] != MONO_PIXEL_WHITE ||
							buf->data[ top_idx ] != MONO_PIXEL_WHITE ||
							buf->data[ bottom_idx ] != MONO_PIXEL_WHITE) &&
							buf->data[center_idx] == MONO_PIXEL_WHITE )
						{
							out->data[ center_idx ] = current_color;	
						}
					}
			}

			// aktualny postęp do bufora
			memcpy( buf->data, out->data, w*h*sizeof(mono_pixel_t) );
	}

	buf = release_mono_buffer( buf );

	return out;
}
