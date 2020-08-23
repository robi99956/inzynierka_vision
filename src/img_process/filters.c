#include "util.h"
#include "data_types.h"

rgb_buffer_t * filter_rgb_buffer( rgb_buffer_t * in, filter_mask_t * filter, rgb_buffer_t * out )
{
	uint32_t w = in->info.w;
	uint32_t h = in->info.h;
	rgb_pixel_t * in_img = in->data;
	uint16_t fsize2 = filter->size/2;

	if( out == NULL ) out = create_rgb_buffer(w, h);

	rgb_pixel_t * out_img = out->data;

	int32_t filter_sum = 0;
	for( uint16_t i=0; i<filter->size*filter->size; i++ ) filter_sum += ABS(filter->values[i]);

	// pętla po piksekach WEJŚCIOWEGO obrazu
	for( uint16_t y = fsize2; y<h-fsize2; y++ )
	{
		// pętla po piksekach WEJŚCIOWEGO obrazu
		for( uint16_t x = fsize2; x<w-fsize2; x++ )
		{
			int new_pixel_R = 0, new_pixel_G = 0, new_pixel_B = 0;

			// pętla po wierszach części pokrytej z filtrem
			for( int16_t i = -fsize2, f_idx = 0; i<=fsize2; i++ )
			{
				// pętla po kolumnach części pokrytej z filtrem
				for( int16_t j=-fsize2; j<=fsize2; j++, f_idx++ )
				{
					new_pixel_R += in_img[(y+i)*w+x+j].R * filter->values[f_idx];
					new_pixel_G += in_img[(y+i)*w+x+j].G * filter->values[f_idx];
					new_pixel_B += in_img[(y+i)*w+x+j].B * filter->values[f_idx];
				}
			}	

			out_img[ y*w + x].R = saturate( new_pixel_R/filter_sum );
			out_img[ y*w + x].G = saturate( new_pixel_G/filter_sum );
			out_img[ y*w + x].B = saturate( new_pixel_B/filter_sum );
		}
	}

	return out;
}

mono_buffer_t * filter_mono_buffer( mono_buffer_t * in, filter_mask_t * filter, mono_buffer_t * out )
{
	uint32_t w = in->info.w;
	uint32_t h = in->info.h;
	mono_pixel_t * in_img = in->data;
	uint16_t fsize2 = filter->size/2;

	if( out == NULL ) out = create_mono_buffer(w, h);

	mono_pixel_t * out_img = out->data;

	int32_t filter_sum = 0;
	for( uint16_t i=0; i<filter->size*filter->size; i++ ) filter_sum += ABS(filter->values[i]);

	// pętla po piksekach WEJŚCIOWEGO obrazu
	for( uint16_t y = fsize2; y<h-fsize2; y++ )
	{
		// pętla po piksekach WEJŚCIOWEGO obrazu
		for( uint16_t x = fsize2; x<w-fsize2; x++ )
		{
			int new_pixel = 0;

			// pętla po wierszach części pokrytej z filtrem
			for( int16_t i = -fsize2, f_idx = 0; i<=fsize2; i++ )
			{
				// pętla po kolumnach części pokrytej z filtrem
				for( int16_t j=-fsize2; j<=fsize2; j++, f_idx++ )
				{
					new_pixel += in_img[(y+i)*w+x+j] * filter->values[f_idx];
				}
			}	

			out_img[ y*w + x] = saturate( new_pixel/filter_sum );
		}
	}

	return out;
}

