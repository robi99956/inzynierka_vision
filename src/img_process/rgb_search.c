#include "util.h"
#include "data_types.h"
#include "transform.h"
list_t * find_contours( mono_buffer_t * buffer, point_t corner_offset, uint8_t max_r );

list_t * find_rgb_areas( rgb_buffer_t * img, rgb_pixel_t color, uint8_t tolerance )
{
	mono_buffer_t * flags = rgb_buffer_threshold(img, color, tolerance, NULL);	

	list_t * areas_list = find_contours(flags, POINT(0, 0), 3);

	release_mono_buffer( flags );

	return areas_list;
} 

point_t find_largest_area( rgb_buffer_t * img, rgb_pixel_t color, uint8_t tolerance )
{
	list_t * areas = find_rgb_areas(img, color, tolerance);

	contour_t * max_cont = (contour_t*)areas->data;

	contour_t * cont;
	LIST_FOR_EACH(areas, cont, {
		if( cont->size > max_cont->size )
		{
			max_cont = cont;
		}		
	});

	uint32_t cent_x=0, cent_y=0;
	uint32_t size = max_cont->size;

	for( uint32_t i=0; i<size; i++ )
	{
		cent_x += max_cont->points[i].x;
		cent_y += max_cont->points[i].y;
	}

	release_list(areas);

	return POINT( cent_x / size, cent_y / size );
}
