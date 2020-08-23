#ifndef CONTOUR_PROCESSING_H
#define CONTOUR_PROCESSING_H

list_t * find_contours( mono_buffer_t * buffer, point_t corner_offset, uint8_t max_r );
contour_t * simplify_contour( contour_t * cont, uint32_t max_d );
list_t * find_rgb_areas( rgb_buffer_t * img, rgb_pixel_t color, uint8_t tolerance );
point_t find_largest_area( rgb_buffer_t * img, rgb_pixel_t color, uint8_t tolerance );

#endif
