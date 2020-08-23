#include <stdio.h>

#include "util.h"
#include "queue.h"
#include "net.h"
#include "data_types.h"
#include "camera.h"
#include "display.h"
#include "contour_processing.h"
#include "psoc_com.h"

#define MAX_UDP_PACKET 64000

int udp_send_image( udp_client_info_t * info, mono_buffer_t * img )
{
	uint32_t data_len = img->info.w*img->info.h;
	
	uint16_t iterations = data_len/MAX_UDP_PACKET;
	uint16_t rest = data_len%MAX_UDP_PACKET;

	uint8_t * data_ptr = (uint8_t*)img->data;

	for( uint16_t i=0; i<iterations; i++ )
	{
		udp_client_send(info, data_ptr, MAX_UDP_PACKET);
		data_ptr += MAX_UDP_PACKET;
	}
	if( iterations ) data_ptr -= MAX_UDP_PACKET;

	if( rest ) udp_client_send(info, data_ptr, rest);

	return 0;
}

int main( int argc, char ** argv )
{
#if 0
	gtk_init(&argc, &argv);
	struct vdIn camera_handle = {};
	if( camera_init("/dev/video2", &camera_handle) < 0 )
	{
		printf("Cant open camera\n");
		exit(-1);
	}
	printf("camera open\n");

	g_timeout_add(1000/15, (GSourceFunc)frame_timer_callback, &camera_handle);

	gtk_main();
#else
	net0_handle_t * net0 = psoc_com_init("/dev/ttyS1");

	net0->send_frame(net0, NET0_DEFAULT_INFO(4), "dupa");
	printf("send done\n");
	usleep(1000UL*3000);
#endif
}

#if 0
	GError * error = NULL;
	GdkPixbuf * pixbuf = gdk_pixbuf_new_from_file("labirynt512.png", &error);

	rgb_buffer_t * in = c_pixbuf_rgb_buffer(pixbuf, NULL);

	point_t start = find_largest_area(in, RGB_PIXEL(0, 255, 0), 30);
	point_t stop = find_largest_area(in, RGB_PIXEL(255, 0, 0), 30);

	mono_buffer_t * mono = c_rgb_buffer_bw_buffer(in, WHITE, NULL);

	filter_mask_t * filter = create_filter_LP(17); 

	mono_buffer_t * filtered = filter_mono_buffer(mono, filter, NULL);
	mono_buffer_threshold(filtered, 255, NULL);
	display_mono_buffer(filtered, "filtered");

	contour_t * way = djikstra_solve_maze(filtered, start, stop);

	if( way == NULL )
	{
		printf("Nie wyszlo!\n");
		goto end;
	}

	//contour_t * simple = simplify_contour(way, 15);
	contour_t * simple = way;
	
	rgb_buffer_draw_contour(in, simple, RGB_PIXEL(0, 0, 255));

	GtkImage * image = display_pixbuf(pixbuf, "in");
	display_rgb_buffer(in, "solved");

	end:
#endif
