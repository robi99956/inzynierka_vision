#include "util.h"
#include "data_types.h"

static inline void set_pixel( rgb_buffer_t * buffer, uint16_t x, uint16_t y, rgb_pixel_t color )
{
	buffer->data[y*buffer->info.w + x] = color;
	buffer->data[(y+1)*buffer->info.w + x] = color;
	buffer->data[y*buffer->info.w + x+1] = color;
	buffer->data[(y+1)*buffer->info.w + x+1] = color;
}

static inline void circle_set_pixel( rgb_buffer_t * buffer, uint16_t cx, uint16_t cy, int16_t x, int16_t y, rgb_pixel_t color )
{
	set_pixel(buffer, cx+x, cy+y, color);
	set_pixel(buffer, cy+y, cx+x, color);
	set_pixel(buffer, cy+y, cx-x, color);
	set_pixel(buffer, cx+x, cy-y, color);

	set_pixel(buffer, cx-x, cy-y, color);
	set_pixel(buffer, cy-y, cx-x, color);
	set_pixel(buffer, cy-y, cx+x, color);
	set_pixel(buffer, cx-x, cy+y, color);
}

void rgb_buffer_draw_circle( rgb_buffer_t * buffer, uint16_t cx, uint16_t cy, uint16_t r, rgb_pixel_t color )
{
	int16_t x=0, y=r, d=1-r;

	circle_set_pixel(buffer, cx, cy, x, y, color);

	while( y>x )
	{
		if( d<0 )
		{
			d = d+2*x+3;
			x++;
		}
		else
		{
			d = d+2*(x-y)+5;
			x++;
			y--;
		}

		circle_set_pixel(buffer, cx, cy, x, y, color);
	}
}

void rgb_buffer_draw_rect( rgb_buffer_t * buffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, rgb_pixel_t color )
{
	for( uint16_t xi=x; xi<=x+w+1; xi+=2 )
	{
		set_pixel(buffer, xi, y, color);
		set_pixel(buffer, xi, y+h, color);
	}

	for( uint16_t yi=y; yi<=y+h+1; yi+=2 )
	{
		set_pixel(buffer, x, yi, color);
		set_pixel(buffer, x+w, yi, color);
	}
}

void rgb_buffer_draw_contour( rgb_buffer_t * buffer, contour_t * contour, rgb_pixel_t color  )
{
	for( uint32_t i = 0; i<contour->size; i++ )
	{
		point_t point = contour->points[i];
		set_pixel(buffer, point.x, point.y, color);
	}
}
