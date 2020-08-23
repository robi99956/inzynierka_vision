#include "util.h"
#include "data_types.h"

rgb_buffer_t * scale_rgb_buffer( rgb_buffer_t * in, int8_t factor )
{
	uint16_t w = in->info.w, h = in->info.h;
	rgb_buffer_t * out;

	// zmniejszanie
	if( factor < 0 )
	{
		factor *= -1;
		w /= factor;
		h /= factor;

		out = create_rgb_buffer(w, h);

		for( uint16_t y = 0, out_idx = 0; y < in->info.h; y += factor )
		{
			for( uint16_t x = 0; x<in->info.w; x += factor, out_idx++ )
			{
				out->data[out_idx] = in->data[ y*in->info.w + x ];
			}	
		}
	}
	// zwiększanie
	else if( factor > 0 )
	{
		w *= factor;
		h *= factor;

		out = create_rgb_buffer(w, h);

		for( uint16_t yo = 0, yi = 0; yo<h; yo++, yi += (yo%factor == 0) )
		{
			for( uint16_t xo = 0, xi = 0; xo<w; xo++, xi += (xo%factor == 0) )
			{
				out->data[yo*w+xo] = in->data[ yi*in->info.w + xi ];
			}
		}	
	}

	return out;
}
