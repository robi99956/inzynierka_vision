#include "util.h"
#include "net0.h"
#include <pthread.h>

#include "psoc_com.h"

void net0_frame_rec_callback(net0_handle_t * net0, net0_frame_info_t info, void * data );
void net0_event_callback( net0_handle_t * net0, net0_event_type_t event );
void * net0_rec_thread( net0_handle_t * net0 );

static uint8_t net0_omega_buf[NET0_BUF_SIZE];
static net0_handle_t net0_handle = {
	.rec_buf = net0_omega_buf,
	.rec_buf_size = NET0_BUF_SIZE,
	.frame_rec_callback = net0_frame_rec_callback,
	.event_callback = net0_event_callback,
	.ll_write_byte = net0_ll_write,
};

net0_handle_t * psoc_com_init( char * uart_path )
{
	net0_init(&net0_handle);
	int uart_fd = uart_init(uart_path);
	if( uart_fd < 0 ) exit(-1);	

	uart_data_t * uart_data = MAKE(uart_data_t, 
					.fd = uart_fd);
	net0_handle.user_data = uart_data;

	pthread_t rec_thread_handle;
	pthread_create(&rec_thread_handle, NULL, (void*(*)(void*))net0_rec_thread, &net0_handle);

	return &net0_handle;
}

void net0_frame_rec_callback( net0_handle_t * net0, net0_frame_info_t info, void * data )
{
    (void)net0;
    (void)info;
    (void)data;
}

void net0_event_callback( net0_handle_t * net0, net0_event_type_t event )
{
    (void)net0;
    (void)event;
}

void * net0_rec_thread( net0_handle_t * net0 )
{
	while( 2 )
	{
		uint8_t byte = net0_ll_read(net0);
		net0->ll_new_byte_rec(net0, byte);
	}

	return NULL;
}
