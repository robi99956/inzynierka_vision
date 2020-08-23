#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#define POINT_DELETE_FLAG 		0x01
#define POINT_FIRST_DIVISION_FLAG 	0x02

#define MONO_PIXEL_BLACK ((mono_pixel_t)0)
#define MONO_PIXEL_WHITE ((mono_pixel_t)-1)

#define PACKED __attribute__((__packed__))

#if !defined(BOOL) && !defined(bool)
typedef uint8_t bool;
#endif

typedef struct PACKED_rgb_pixel
{
	uint8_t R;
	uint8_t G;
	uint8_t B;
	uint8_t flags;
} rgb_pixel_t;

typedef struct PACKED _hsv_pixel
{
	uint8_t H;
	uint8_t S;
	uint8_t V;
	uint8_t flags;
} hsv_pixel_t;

typedef struct PACKED _yuv_macropixel
{
	uint8_t y0;
	uint8_t u0;
	uint8_t y1;
	uint8_t v0;
} yuv_macropixel_t;

typedef uint8_t mono_pixel_t;

typedef struct _buffer_info
{
	uint16_t w;
	uint16_t h;	
} buffer_info_t;

typedef struct _rgb_buffer
{
	buffer_info_t info;

	rgb_pixel_t * data;
} rgb_buffer_t;

typedef struct _mono_buffer
{
	buffer_info_t info;

	mono_pixel_t * data;
} mono_buffer_t;

typedef struct _yuv_buffer
{
	buffer_info_t info;

	yuv_macropixel_t * data;
} yuv_buffer_t;

typedef struct _filter_mask
{
	uint16_t size;
	int16_t * values;
} filter_mask_t;

typedef struct _point
{
	int16_t x;
	int16_t y;
	uint8_t flags;
} point_t;

typedef struct _contour
{
	uint32_t size;
	uint32_t base_size;
	uint32_t memory_size;

	point_t * points;	
} contour_t;

typedef void*(*destructor_t)(void*);

typedef struct _list
{
	destructor_t destructor;
	void * data;
	struct _list * next;	
} list_t;

typedef struct _index_pair
{
	uint32_t idx1;
	uint32_t idx2;
} index_pair_t;

typedef struct _index_pair_stack
{
	uint32_t size;
	uint32_t base_size; 
	uint32_t memory_size;

	index_pair_t * pairs;	
} index_pair_stack_t;

rgb_buffer_t * create_rgb_buffer( uint16_t w, uint16_t h );
rgb_buffer_t * release_rgb_buffer( rgb_buffer_t * buffer );
mono_buffer_t * create_mono_buffer( uint16_t w, uint16_t h );
mono_buffer_t * release_mono_buffer( mono_buffer_t * buffer );
yuv_buffer_t * create_yuv_buffer( uint16_t w, uint16_t h );
yuv_buffer_t * release_yuv_buffer( yuv_buffer_t * buffer );

#ifdef __GTK_H__
rgb_buffer_t * c_pixbuf_rgb_buffer( GdkPixbuf * pixbuf, rgb_buffer_t * out );
GdkPixbuf * c_rgb_buffer_pixbuf( rgb_buffer_t * buffer );
#endif

mono_buffer_t * c_rgb_buffer_bw_buffer( rgb_buffer_t * rgb, mono_pixel_t min_white, mono_buffer_t * out );
rgb_buffer_t * c_mono_buffer_rgb_buffer( mono_buffer_t * mono, rgb_buffer_t * out );
mono_buffer_t * c_rgb_buffer_mono_buffer( rgb_buffer_t * rgb, mono_buffer_t * out );
mono_buffer_t * c_yuv_buffer_mono_buffer( yuv_buffer_t * yuv, mono_buffer_t * out );
rgb_buffer_t * c_yuv_buffer_rgb_buffer( yuv_buffer_t * yuv, rgb_buffer_t * out );

contour_t * create_contour( uint32_t base_size );
contour_t * release_contour( contour_t * contour );
void contour_add_point( contour_t * contour, point_t point );
contour_t * contour_delete_marked( contour_t * in, contour_t * out );
void contour_clear( contour_t * contour );

list_t * create_list( destructor_t destructor );
list_t * release_list( list_t * list );
void list_add( list_t * list, void * data );
void list_clear( list_t * list );

index_pair_stack_t * create_index_pair_stack( uint32_t base_size );
index_pair_stack_t * release_index_pair_stack( index_pair_stack_t * stack );
void index_pair_stack_push( index_pair_stack_t * stack, index_pair_t pair );
index_pair_t index_pair_stack_pop( index_pair_stack_t * stack );

filter_mask_t * create_filter_LP( uint16_t size );
filter_mask_t * release_filter( filter_mask_t * filter );

#define LIST_FOR_EACH(__list_in, __data, code) do{ 		\
		list_t * __list = __list_in;			\
		while(__list && __list->data) 					\
		{						\
			__data = (typeof(__data))__list->data;			\
			code;					\
			__list = __list->next;			\
		}						\
	}while(0)

#define POINT(_x, _y) ((point_t){.x=_x, .y=_y, .flags=0})
#define RGB_PIXEL(r, g, b) ((rgb_pixel_t){.R=r, .G=g, .B=b, .flags=0})
#define INDEX_PAIR(_idx1, _idx2) ((index_pair_t){.idx1=_idx1, .idx2=_idx2})
#define BUFFER_INFO(_w, _h) ((buffer_info_t){.w=_w, .h=_h})
#endif
