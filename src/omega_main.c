#ifdef USE_GTK
#include <gtk/gtk.h>
#include <glib.h>
#endif

#include <stdio.h>

#include "util.h"
#include "queue.h"
#include "data_types.h"
#include "camera.h"
#include "display.h"
#include "contour_processing.h"
#include "transform.h"
#include "maze.h"
#include "psoc_com.h"
#include "net.h"

#define TCP_COMMAND_READ 		0
#define TCP_COMMAND_CONFIRM 	1

#define CM_TO_PIXEL_RATIO (100.0/146.0)
#define X_POINT_OFFSET_CM 200
#define Y_POINT_OFFSET_CM 200

typedef struct
{
	char * camera_file;
	pthread_queue_t * cmd_queue;
	udp_client_info_t * udp_info;
	contour_t * path;
	net0_handle_t * net0;
} camera_thread_info_t;

typedef struct PACKED
{
	uint8_t cmd;
	int16_t x;
	int16_t y;
	int16_t z;
} net0_goto_command_t;

void print_point( char * txt, point_t p )
{
	fprintf(stderr, "%s%d-%d\n", txt, p.x, p.y);
}

// po wyjściu z tej funkcji pamięć spod img jest zwolniona!
contour_t * process_maze_image( rgb_buffer_t * img )
{
#define WHITE 25
	contour_t * result = NULL;
	point_t start = find_largest_area(img, RGB_PIXEL(110, 156, 60), 30);
	point_t stop = find_largest_area(img, RGB_PIXEL(150, 0, 0), 30);

	print_point("Start = ", start);	
	print_point("Stop = ", stop);	

	// kolor już nie potrzebny, zmień na mono i wywal
	mono_buffer_t * mono = c_rgb_buffer_bw_buffer(img, WHITE, NULL);
	if( mono == NULL ) goto end;
	INFO("Progowanie OK");

	filter_mask_t * lp_filter = create_filter_LP(3);
	mono_buffer_t * filtered = filter_mono_buffer(mono, lp_filter, NULL); 
	mono_buffer_threshold(filtered, WHITE, filtered);

	mono_buffer_expand_black(filtered, 0, 100, 15, mono);
	//mono = release_mono_buffer(mono);
	INFO("Rozciagniecie OK");
	if( filtered == NULL ) goto end;

	contour_t * way = djikstra_solve_maze(mono, start, stop, filtered);
	INFO("Labirynt OK");
	if( way == NULL ) goto end;

	//c_mono_buffer_rgb_buffer(filtered, img);


	result = simplify_contour(way, 15);
	rgb_buffer_draw_contour(img, result, RGB_PIXEL(0, 0, 255));

	end:
	release_filter(lp_filter);
	release_mono_buffer(filtered);
	release_contour(way);
	return result;	
}

void net0_send_path( net0_handle_t * net0, contour_t * path )
{
	fprintf(stderr, "net0_send start\n");	
	for( int16_t i=path->size-1; i>=0; i-- )
	{
		net0_goto_command_t cmd = {
			.cmd = 0,
			.x = path->points[i].x,
			.y = path->points[i].y,
			.z = 0
		};

		if( i != 0 ) cmd.z = 40;
		else cmd.z = -40;

		net0_frame_info_t info = {.size = sizeof(net0_goto_command_t)};

		fprintf(stderr, "Point set to (%d,%d)\n", cmd.x, cmd.y);
		net0_send_frame(net0, &info, &cmd);

		delay_ms(5000);
		//return;
	}
}

void contour_convert_to_cm( contour_t * contour )
{
	for( int32_t i=0; i<contour->size; i++ )
	{
		int16_t new_x = contour->points[i].y*(0.69) + 194; 
		int16_t new_y = contour->points[i].x*(0.92)  - 228; 

		fprintf(stderr, "Point conversion (%d,%d) to (%d,%d)\n", 
						contour->points[i].x, 
						contour->points[i].y,
						new_x,
						new_y);
		contour->points[i].x = new_x;
		contour->points[i].y = new_y;
	}
}

// zwraca obraz z zaznaczona droga
rgb_buffer_t * vision_process( camera_thread_info_t * info )
{
		int status = 0;
		fprintf(stderr, "Camera thread fired!\n");

		struct vdIn camera_handle;
		if( (status=camera_init(info->camera_file, &camera_handle)) != 0 ) 
		{
			fprintf(stderr, "Cant open camera! %d\n", status);
			return NULL;
		}

		yuv_buffer_t raw_data = {};
		
		if( (status=camera_grab_frame_yuv(&camera_handle, &raw_data)) != 0 )
		{
			fprintf(stderr, "Cant grab frame! %d\n", status);
			return NULL;
		}

		rgb_buffer_t * rgb = c_yuv_buffer_rgb_buffer(&raw_data, NULL);
		fprintf(stderr, "Conversion done\n");

		// TODO przetwarzanie obrazu
		info->path = process_maze_image( rgb );
		if( info->path )
		{
			contour_convert_to_cm( info->path );
		}

		camera_close(&camera_handle);

		return rgb;
}

void tcp_new_connection( int fd, struct sockaddr_in * addr, void * arg )
{
		pthread_t handle;
		rgb_buffer_t * rgb = NULL;
		camera_thread_info_t * cam_info = (camera_thread_info_t*)arg;

		fprintf(stderr, "New connection\n");
		// odczytanie co PC chce
		uint8_t command;
		recv(fd, &command, 1, 0);
		fprintf(stderr, "Command = %d\n", command);

		switch( command )
		{
			case TCP_COMMAND_READ:
			{
				// w tym watku kamera nie dziala, nie wiadomo czemu
				pthread_create(&handle, NULL, (void*(*)(void*))vision_process, arg);
				pthread_join(handle, (void**)&rgb);

				if( rgb != NULL )
				{
						fprintf(stderr, "Img read OK\n");
						uint32_t data_size = rgb->info.w*rgb->info.h*sizeof(rgb_pixel_t);
						send(fd, (uint8_t*)&rgb->info, sizeof(buffer_info_t), 0);
						fprintf(stderr, "Header send OK\n");
						send(fd, (uint8_t*)rgb->data, data_size, 0);
						fprintf(stderr, "Img data send OK\n");

						release_rgb_buffer(rgb);
				}
				else
						fprintf(stderr, "img read failed\n");
			} break;

			case TCP_COMMAND_CONFIRM:
			{
				if( cam_info->path != NULL )
				{
						fprintf(stderr, "Confirm command rec!, net0 = %p\n", cam_info->net0);
						net0_send_path(cam_info->net0, cam_info->path);
				}
			} break;

			default:
				fprintf(stderr, "Unknown command!\n");
		}

		close(fd);
}

int main( int argc, char ** argv )
{
	int status = 0;

	camera_thread_info_t camera_info = 
	{
		.camera_file = "/dev/video0",
		.cmd_queue = NULL,
		.udp_info = NULL,
		.net0 = psoc_com_init( "/dev/ttyS1" ),
	};

	tcp_server_info_t tcp_info = {};
	status = tcp_server_init(9999, tcp_new_connection, &camera_info, &tcp_info);
	if( status < 0 )
	{
		printf("TCP server init error\n");
		exit(-1);
	}

	pthread_join(tcp_info.basic.thread_handle, NULL);
}
