#include "util.h"
#include "data_types.h"

contour_t * djikstra_solve_maze( mono_buffer_t * img, point_t start, point_t end )
{
	uint16_t w = img->info.w;
	uint16_t h = img->info.h;

	uint32_t * cost_map = TABLE(w*h, uint32_t);
	bool * visited = TABLE(w*h, bool);

	index_pair_stack_t * stack = create_index_pair_stack( w );
	index_pair_stack_push(stack, INDEX_PAIR(start.x, start.y));
	uint32_t current_idx = start.x+start.y*w;

	visited[ current_idx ] = 1;
	cost_map[ current_idx ] = 1;
	
	bool solution_found = 0;
	contour_t * result = NULL;

	// budowa mapy kosztów dotarcia do KAŻDEGO białego punktu
	while( stack->size )
	{
		index_pair_t pair = index_pair_stack_pop(stack);
		point_t point = POINT(pair.idx1, pair.idx2);

		// koniec szukania
		if( point.x == end.x && point.y == end.y ) 
		{
			solution_found = 1;
			break;
		}

		for( int8_t dy = -1; dy<=1; dy++ )
		{
			for( int8_t dx = -1; dx<=1; dx++ )
			{
				// wyjazd za obraz
				if( (point.x+dx) < 0 || (point.x+dx) >= w || (point.y+dy) < 0 || (point.y+dy) >= h ) continue; 
				
				current_idx = (point.x+dx) + (point.y+dy)*w;

				if( img->data[current_idx] == MONO_PIXEL_WHITE && visited[current_idx] == 0 )
				{
					index_pair_stack_push(stack, INDEX_PAIR(point.x+dx, point.y+dy));
					visited[current_idx] = 1;
					cost_map[current_idx] = cost_map[ point.x + point.y*w ] + 1;
				}
			}
		}
	}	

	if( solution_found == 0 )
	{
		goto CLEANUP;	
	}

	result = create_contour(w);

	// cofanie się do punktu początkowego po najtańszej trasie
	uint32_t current_cost = cost_map[ end.x + end.y*w ];
	point_t point = end;
	point_t next_point = end;
#define SEARCH_RADIUS 2

	while( current_cost != 1 )
	{
		for( int8_t dy = -SEARCH_RADIUS; dy<=SEARCH_RADIUS; dy++ )
		{
			for( int8_t dx = -SEARCH_RADIUS; dx<=SEARCH_RADIUS; dx++ )
			{
				// wyjazd za obraz
				if( (point.x+dx) < 0 || (point.x+dx) >= w || (point.y+dy) < 0 || (point.y+dy) >= h ) continue; 
				
				current_idx = (point.x+dx) + (point.y+dy)*w;

				if( cost_map[current_idx] < current_cost && visited[current_idx] == 1 )
				{
					current_cost = cost_map[current_idx];
					next_point = POINT(point.x+dx, point.y+dy);	
				}
			}
		}

		point = next_point;
		contour_add_point(result, point);
	}	

	CLEANUP:
	RELEASE(cost_map);
	RELEASE(visited);
	release_index_pair_stack(stack);

	return result;
}
