#include "util.h"
#include "data_types.h"
#include "transform.h"

#include <math.h>

static contour_t * link_points( mono_buffer_t * buffer, mono_buffer_t * flags, point_t point, uint8_t max_r )
{
	uint16_t w = buffer->info.w;
	uint16_t h = buffer->info.h;
	contour_t * contour = create_contour(w);
	//contour_add_point(contour, point);
	bool point_found = 0;


	for( uint8_t r=1; r <= max_r; r++ )
	{
		for( int16_t y=point.y-r; y< point.y+r; y++ )
		{
			if( y<0 || y >= h ) continue;

			for( int16_t x=point.x-r; x<point.x+r; x++ )
			{
				if( x<0 || x >= w ) continue;
				
				uint32_t idx = y*w+x;
				if( buffer->data[idx] > 0 && flags->data[idx] == 0 ) 
				{
					point_t next_point = POINT(x, y);
					contour_add_point(contour, next_point);
					point = next_point;
					
					r = 0;
					point_found = 1;
					flags->data[idx] = 1;
					break;
				}
			}

			if( point_found )
			{
				r = 0;
				point_found = 0;
				break;
			}
		}	

	}


	return contour;
}

list_t * find_contours( mono_buffer_t * buffer, point_t corner_offset, uint8_t max_r )
{
	uint16_t w = buffer->info.w;
	uint16_t h = buffer->info.h;
	list_t * contours = create_list( (destructor_t)release_contour );
	mono_buffer_t * flags = create_mono_buffer(w, h);

	for( uint16_t y=corner_offset.y; y<h-2*corner_offset.y; y++ )
	{
		for( uint16_t x=corner_offset.x; x<w-2*corner_offset.x; x++ )
		{
			uint32_t idx = y*w+x;

			if( buffer->data[idx] > 0 && flags->data[idx] == 0 )
			{
				contour_t * new_contour = link_points(buffer, flags, POINT(x, y), max_r);
				list_add(contours, new_contour);
			}		
		}	
	}

	release_mono_buffer(flags);

	return contours;
}

// odległość punktu od prostej
uint32_t calculate_d( point_t p, point_t line_start, point_t line_end )
{
#if 0
	double dx = line_end.x - line_start.x;
	double dy = line_end.y - line_start.y;
 
	//Normalise
	double mag = pow(pow(dx,2.0)+pow(dy,2.0),0.5);
	if(mag > 0.0)
	{
		dx /= mag; dy /= mag;
	}
 
	double pvx = p.x - line_start.x;
	double pvy = p.y - line_start.y;
 
	//Get dot product (project pv onto normalized direction)
	double pvdot = dx * pvx + dy * pvy;
 
	//Scale line direction vector
	double dsx = pvdot * dx;
	double dsy = pvdot * dy;
 
	//Subtract this from pv
	double ax = pvx - dsx;
	double ay = pvy - dsy;
 
	return pow(ax,2.0)+pow(ay,2.0);
#else
	int16_t dx = line_end.x - line_start.x;
	int16_t dy = line_end.y - line_start.y;
 
	int32_t dx2 = dx*dx;
	int32_t dy2 = dy*dy;
	int32_t dxdy = dx*dy;

	int32_t mag = dx2+dy2;

	int16_t pvx = p.x - line_start.x;
	int16_t pvy = p.y - line_start.y;
 
	int32_t dsx = (dx2*pvx+dxdy*pvy)/mag;
	int32_t dsy = (dxdy*pvx+dy2*pvy)/mag;
 
	int16_t ax = pvx - dsx;
	int16_t ay = pvy - dsy;
 
	return ax*ax+ay*ay;
#endif
}

uint32_t calculate_point_d( point_t p1, point_t p2 )
{
	int16_t dx = p1.x-p2.x;
	int16_t dy = p1.y-p2.y;

	return (dx*dx + dy*dy);
}

uint32_t next_line_start( contour_t * cont, uint32_t start_idx, uint32_t end_idx, uint32_t max_allowed_d )
{
	uint32_t d = 0, max_d = 0, max_idx = 0;
	point_t line_start = cont->points[start_idx];
	point_t line_end = cont->points[end_idx];

	for( uint32_t i=start_idx+1; i<end_idx; i++ )
	{
		d = calculate_d( cont->points[i], line_start, line_end );

		if( d > max_d ) 
		{
			max_d = d;
			max_idx = i;
		}
	}

	if( max_d > max_allowed_d ) return max_idx;
	
	return 0;
}

void mark_points_to_delete( contour_t * cont, uint32_t start_idx, uint32_t end_idx )
{
	for( uint32_t i=start_idx+1; i < end_idx; i++ )
	{
		cont->points[i].flags |= POINT_DELETE_FLAG;
	}
} 

// usuwa ewentualny dodatkowy wierzchołek wynikający ze wstępnego podziału
contour_t * delete_first_division_point( contour_t * cont, uint32_t max_d )
{
	for( uint32_t i=0; i<cont->size; i++ )
	{
		point_t * point = &cont->points[i];
		point_t prev = (i==0) ? cont->points[cont->size-1] : cont->points[i-1];
		point_t next = (i==(cont->size-1)) ? cont->points[0] : cont->points[i+1];

		if( calculate_d(*point, prev, next) < max_d ) point->flags |= POINT_DELETE_FLAG;
	}

	return contour_delete_marked(cont, NULL);
}

contour_t * simplify_contour( contour_t * cont, uint32_t max_d )
{
	index_pair_stack_t * stack = create_index_pair_stack(20);
	max_d *= max_d;

	// pierwszy podział robiony ręcznie, ze względu na to, że początek i koniec
	// krzywej na 99% są sąsiednimi punktami
	uint32_t middle = cont->size/2;
	index_pair_stack_push( stack, INDEX_PAIR(0, middle) );
	index_pair_stack_push( stack, INDEX_PAIR(middle, cont->size-1) );

	cont->points[middle].flags |= POINT_FIRST_DIVISION_FLAG;

	while( stack->size )
	{
		index_pair_t pair = index_pair_stack_pop(stack);
		uint32_t next_point = next_line_start( cont, pair.idx1, pair.idx2, max_d );

		// najdalszy punkt nie mieści się w tolerancji, dziel dalej
		if( next_point ) 
		{
			index_pair_stack_push( stack, INDEX_PAIR(pair.idx1, next_point));
			index_pair_stack_push( stack, INDEX_PAIR(next_point, pair.idx2));
		}
		else
		{
			mark_points_to_delete(cont, pair.idx1, pair.idx2);
		}
	}		

	release_index_pair_stack(stack);

	point_t * first = &cont->points[0];
	point_t * last = &cont->points[ cont->size-1 ];

	// jeśli początek blisko końca, usuń koniec
	if( calculate_point_d(*first, *last) < max_d ) last->flags = POINT_DELETE_FLAG;
	
	contour_t * simple = contour_delete_marked(cont, NULL);
	contour_t * simple_corrected = delete_first_division_point(simple, max_d);

	release_contour(simple);
	
	return simple_corrected;
}
