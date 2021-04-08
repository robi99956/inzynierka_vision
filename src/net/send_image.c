#include <errno.h>

#include "util.h"
#include "data_types.h"
#include "net.h"

#define MAX_UDP_PACKET_SIZE 24000
#define UDP_IMG_HEADER_SIZE 4
#define MAX_UDP_IMG_DATA_SIZE (MAX_UDP_PACKET_SIZE-UDP_IMG_HEADER_SIZE)
#define LAST_IMG_CHUNK_NR 0xffffffff

int udp_send_multipacket_buffer( udp_client_info_t * info, uint8_t * data_ptr, uint32_t data_len );

// payload: indeks + dane
int udp_send_mono_buffer( udp_client_info_t * info, mono_buffer_t * img )
{
	uint32_t data_len = img->info.w*img->info.h*sizeof(mono_pixel_t);
	uint8_t * data_ptr = (uint8_t*)img->data;

	return udp_send_multipacket_buffer(info, data_ptr, data_len);
}

// payload: indeks + dane
int udp_send_rgb_buffer( udp_client_info_t * info, rgb_buffer_t * img )
{
	uint32_t data_len = img->info.w*img->info.h*sizeof(rgb_pixel_t);
	uint8_t * data_ptr = (uint8_t*)img->data;

	return udp_send_multipacket_buffer(info, data_ptr, data_len);
}

int udp_send_multipacket_buffer( udp_client_info_t * info, uint8_t * data_ptr, uint32_t data_len )
{
	uint32_t current_chunk_len = 0;
	uint8_t payload[ MAX_UDP_PACKET_SIZE ];

	for( uint32_t i=0; data_len; i++ )
	{
		if( data_len <= MAX_UDP_IMG_DATA_SIZE ) 
		{
			((uint32_t*)payload)[0] = LAST_IMG_CHUNK_NR;
			current_chunk_len = data_len;
		}
		else 
		{
			((uint32_t*)payload)[0] = i;
			current_chunk_len = MAX_UDP_IMG_DATA_SIZE;
		}

		memcpy(payload+UDP_IMG_HEADER_SIZE, data_ptr, current_chunk_len);
		if( udp_client_send(info, payload, current_chunk_len+UDP_IMG_HEADER_SIZE) < 0 )
		{
			fprintf(stderr, "UDP img send error!\n");
			return -1;
		}

		data_len -= current_chunk_len;
		data_ptr += current_chunk_len;
	}

	return 0;
}

int tcp_send_multipacket_buffer( int fd, uint8_t * buf, uint32_t len )
{
	uint32_t bytes_written = 0;

	while( bytes_written != len )
	{
		int bw = send(fd, buf+bytes_written, len-bytes_written, 0);
		if( bw < 0 )
		{
			fprintf(stderr, "write failed with %d, %s\n", bw, strerror(errno));
			return bw;
		}
		
		fprintf(stderr, "%d bytes send\n", bw);
		bytes_written += bw;
	}

	return 0;
}
