#ifndef NET_H
#define NET_H

#include "queue.h"
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define UDP_SERVER_BUF_LEN_KB 2

typedef struct
{
	struct sockaddr_in source; // przydatne po stronie serwera, klient olewa
	void * data;
	uint32_t len;	
} udp_message_t;

typedef struct
{
	int socket_fd;
    struct sockaddr_in servaddr; 
	pthread_t thread_handle;
} net_basic_info_t;

typedef struct
{
	net_basic_info_t basic;

	pthread_queue_t queue;
} udp_client_info_t;

typedef void(*udp_callback_t)(udp_message_t*,void*);

typedef struct
{
	net_basic_info_t basic;

	void * arg;
	udp_callback_t callback;
} udp_server_info_t;

typedef void(*tcp_callback_t)(int,struct sockaddr_in*,void*);

typedef struct
{
	net_basic_info_t basic;
	void * arg;

	tcp_callback_t callback;
} tcp_server_info_t;

int udp_client_init( char * server_ip, uint32_t port, udp_client_info_t * info );
int udp_client_send( udp_client_info_t * info, void * data, uint32_t len );
int udp_client_close( udp_client_info_t * info );
int udp_server_init( uint32_t port, udp_callback_t callback, void * arg, udp_server_info_t * info );
int udp_server_close( udp_server_info_t * info );
int tcp_server_init( uint32_t port, tcp_callback_t callback, void * arg, tcp_server_info_t * info );
int tcp_server_close( tcp_server_info_t * info );

int udp_send_mono_buffer( udp_client_info_t * info, mono_buffer_t * img );
int udp_send_rgb_buffer( udp_client_info_t * info, rgb_buffer_t * img );

int tcp_send_multipacket_buffer( int fd, uint8_t * buf, uint32_t len );
#endif
