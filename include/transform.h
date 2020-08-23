#ifndef TRANSFORM_H
#define TRANSFORM_H

rgb_buffer_t * scale_rgb_buffer( rgb_buffer_t * in, int8_t factor, rgb_buffer_t * out );
rgb_buffer_t * filter_rgb_buffer( rgb_buffer_t * in, filter_mask_t * filter, rgb_buffer_t * out );
mono_buffer_t * filter_mono_buffer( mono_buffer_t * in, filter_mask_t * filter, mono_buffer_t * out );
mono_buffer_t * mono_buffer_threshold( mono_buffer_t * in, mono_pixel_t min_white, mono_buffer_t * out );
mono_buffer_t * rgb_buffer_threshold( rgb_buffer_t * in, rgb_pixel_t color, uint8_t tolerance, mono_buffer_t * out );

#endif
