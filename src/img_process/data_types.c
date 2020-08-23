#include "assert.h"
#include "util.h"
#include "data_types.h"

// ************************************************************************************* //
// ************************************ Bufor RGB ************************************** //
// ************************************************************************************* //

rgb_buffer_t * create_rgb_buffer( uint16_t w, uint16_t h )
{
	rgb_buffer_t * buffer = NEW(rgb_buffer_t);
	if( buffer == NULL ) 
	{
		fprintf(stderr, "Buffer(%dx%d) allocation failed!\n", w, h);
		return NULL;
	}

	buffer->data = TABLE(w*h, rgb_pixel_t);
	if( buffer->data == NULL )
	{
		fprintf(stderr, "Buffer(%dx%d) data allocation failed!\n", w, h);
		RELEASE(buffer);
		return NULL;
	}

	buffer->info = BUFFER_INFO(w, h);

	return buffer;
}

rgb_buffer_t * release_rgb_buffer( rgb_buffer_t * buffer )
{
	RELEASE( buffer->data );
	return RELEASE(buffer);
}

// ******************************************************************************************** //
// *************************************** Bufor mono ***************************************** //
// ******************************************************************************************** //

mono_buffer_t * create_mono_buffer( uint16_t w, uint16_t h )
{
	mono_buffer_t * buffer = NEW(mono_buffer_t);
	if( buffer == NULL ) 
	{
		fprintf(stderr, "Buffer(%dx%d) allocation failed!\n", w, h);
		return NULL;
	}

	buffer->data = TABLE(w*h, mono_pixel_t);
	if( buffer->data == NULL )
	{
		fprintf(stderr, "Buffer(%dx%d) data allocation failed!\n", w, h);
		RELEASE(buffer);
		return NULL;
	}

	buffer->info = BUFFER_INFO(w, h);

	return buffer;
}

mono_buffer_t * release_mono_buffer( mono_buffer_t * buffer )
{
	RELEASE( buffer->data );
	return RELEASE(buffer);
}

// ******************************************************************************************** //
// *************************************** Bufor yuv ****************************************** //
// ******************************************************************************************** //

yuv_buffer_t * create_yuv_buffer( uint16_t w, uint16_t h )
{
	yuv_buffer_t * buffer = NEW(yuv_buffer_t);
	if( buffer == NULL ) 
	{
		fprintf(stderr, "Buffer(%dx%d) allocation failed!\n", w, h);
		return NULL;
	}

	buffer->data = TABLE(w*h/2, yuv_macropixel_t);
	if( buffer->data == NULL )
	{
		fprintf(stderr, "Buffer(%dx%d) data allocation failed!\n", w, h);
		RELEASE(buffer);
		return NULL;
	}

	buffer->info = BUFFER_INFO(w, h);

	return buffer;
}

yuv_buffer_t * release_yuv_buffer( yuv_buffer_t * buffer )
{
	RELEASE( buffer->data );
	return RELEASE(buffer);
}

// ******************************************************************************************** //
// ********************************* Punkty i kształty **************************************** //
// ******************************************************************************************** //

contour_t * create_contour( uint32_t base_size )
{
	contour_t * out = NEW(contour_t);
	if( out == NULL ) return NULL;

	out->points = TABLE(base_size, point_t);
	if( out->points == NULL )
	{
		RELEASE(out);
		return NULL;
	}

	out->base_size = base_size;
	out->memory_size = base_size;
	out->size = 0;

	return out;
}

contour_t * release_contour( contour_t * contour )
{
	RELEASE(contour->points);
	return RELEASE(contour);
}

void contour_add_point( contour_t * contour, point_t point )
{
	// punkt się nie zmieści, zwiększ bufor
	if( contour->memory_size == contour->size )
	{
		contour->memory_size += contour->base_size;
		contour->points = REALLOC(contour->points, contour->memory_size);
		assert(contour->points);
	}

	contour->points[ contour->size++ ] = point;
}

contour_t * contour_delete_marked( contour_t * in, contour_t * out )
{
	if( out == NULL ) out = create_contour(in->base_size);

	for( uint32_t i=0; i<in->size; i++ )
	{
		point_t point = in->points[i];

		if( (point.flags & POINT_DELETE_FLAG) == 0 ) contour_add_point(out, point);
	}

	return out;
}

void contour_clear( contour_t * contour )
{
	contour->points = RELEASE(contour->points);
	// wymusza alokację przy kolejnym dodaniu punktu
	contour->memory_size = contour->size = 0;
}

// ****************************************************************************************** //
// ***************************************** Lista ****************************************** //
// ****************************************************************************************** //

list_t * create_list( destructor_t destructor )
{
	list_t * list = MAKE(list_t, .destructor = destructor, .data = NULL, .next = NULL);

	return list;
}

list_t * release_list( list_t * list )
{
	assert(list);

	list_t * next_node;

	while( list )
	{
		next_node = list->next;

		if( list->destructor ) list->destructor( list->data );
		RELEASE(list);

		list = next_node;
	}

	return list;
}

void list_add( list_t * list, void * data )
{
	assert(list);

	if( list->data == NULL )
	{
		list->data = data;
	}
	else
	{
		list_t * new_node = MAKE(list_t, .data = data);

		while( list->next )
		{
			list = list->next;
		}

		list->next = new_node;
	}
}

void list_clear( list_t * list )
{
	assert(list);

	if( list->destructor ) list->destructor(list->data);

	if( list->next ) 
	{
		list->next = release_list(list->next);		
	}
}

// ****************************************************************************************** //
// ************************************* Stos indeksów ************************************** //
// ****************************************************************************************** //

index_pair_stack_t * create_index_pair_stack( uint32_t base_size )
{
	index_pair_stack_t * stack = MAKE(index_pair_stack_t, .base_size = base_size, .memory_size = base_size, .size = 0);
	
	stack->pairs = TABLE(base_size, index_pair_t);
	assert( stack->pairs );

	return stack;
}

index_pair_stack_t * release_index_pair_stack( index_pair_stack_t * stack )
{
	RELEASE( stack->pairs );
	return RELEASE(stack);
}

void index_pair_stack_push( index_pair_stack_t * stack, index_pair_t pair )
{
	if( stack->size == stack->memory_size )
	{
		stack->memory_size += stack->base_size;
		stack->pairs = REALLOC(stack->pairs, stack->memory_size);
		assert(stack->pairs);
	}

	stack->pairs[ stack->size++ ] = pair; 
}

index_pair_t index_pair_stack_pop( index_pair_stack_t * stack )
{
	assert(stack->size);	

	index_pair_t pair = stack->pairs[ stack->size-1 ];
	stack->size--;
	return pair;
}

// ****************************************************************************************** //
// ***************************************** Filtry ***************************************** //
// ****************************************************************************************** //

filter_mask_t * create_filter_LP( uint16_t size )
{
	uint32_t mask_len = size*size;

	int16_t * values = TABLE(mask_len, int16_t);
	assert(values);

	for( uint32_t i=0; i<mask_len; i++ ) values[i] = 1;

	filter_mask_t * filter = MAKE(filter_mask_t, .size = size, .values = values);
	assert(filter);

	return filter;
}

filter_mask_t * release_filter( filter_mask_t * filter )
{
	RELEASE(filter->values);
	return RELEASE(filter);
}
