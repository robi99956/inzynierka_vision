#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <errno.h>

#include "util.h"
#include "queue.h"
#include "data_types.h"
#include "net.h"
 
void * udp_client_thread( udp_client_info_t * info )
{
	int retval = 0;
	
	while(3)
	{
		udp_message_t * msg = pthread_queue_pop_front(&info->queue);

		if( msg == NULL ) break;

		retval = sendto(info->basic.socket_fd, (const char*)msg->data, msg->len, 0, 
			(const struct sockaddr*)&info->basic.servaddr, sizeof(info->basic.servaddr));
		
		RELEASE(msg->data);
		RELEASE(msg);

		(void)retval;
	}

	return NULL;
}

int udp_client_init( char * server_ip, uint32_t port, udp_client_info_t * info )
{
	bzero(info, sizeof(udp_client_info_t));
	info->basic.socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if( info->basic.socket_fd < 0 ) return -1;
	
	info->basic.servaddr.sin_family = AF_INET;
	info->basic.servaddr.sin_port = htons(port);

	pthread_queue_init(&info->queue);

	if( inet_aton(server_ip, (struct in_addr*)&info->basic.servaddr.sin_addr.s_addr) == 0 ) return -1;	

	if( pthread_create(&info->basic.thread_handle, NULL, (void*(*)(void*))udp_client_thread, info) != 0 ) return -1;

	return 0;
}

int udp_client_send( udp_client_info_t * info, void * data, uint32_t len )
{
	udp_message_t * msg = MAKE(udp_message_t, .len = len);
	if( msg == NULL ) return -1;

	msg->data = TABLE(len, uint8_t);
	if( msg->data == NULL ) return -1;

	memcpy(msg->data, data, len);

	return pthread_queue_push_back(&info->queue, msg);
}

int udp_client_close( udp_client_info_t * info )
{
	pthread_queue_push_back(&info->queue, NULL);
	pthread_join( info->basic.thread_handle, NULL );

	return close( info->basic.socket_fd );
}

void * udp_server_thread( udp_server_info_t * info )
{
	uint8_t packet_buffer[UDP_SERVER_BUF_LEN_KB*1024UL] = {};
	struct sockaddr_in client_addr = {};
	socklen_t client_addr_len = sizeof(client_addr);
	ssize_t packet_len;

	while(8)
	{
		packet_len = recvfrom(info->basic.socket_fd, packet_buffer, sizeof(packet_buffer),
					MSG_WAITALL, (struct sockaddr*)&client_addr, &client_addr_len);

		// prawidłowy pakiet
		if( packet_len > 0 )
		{
			udp_message_t msg = {
				.source = client_addr, 
				.data = packet_buffer, 
				.len = packet_len
				};
			if( info->callback ) info->callback(&msg, info->arg);
		}
		else
		{
			// ...
		}
	}

	return NULL;
}

int udp_server_init( uint32_t port, udp_callback_t callback, void * arg, udp_server_info_t * info )
{
	bzero(info, sizeof(udp_server_info_t));
	info->basic.socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	int status = 0;
	if( (status=info->basic.socket_fd) < 0 ) return status;
	
	info->basic.servaddr.sin_family = AF_INET;
    info->basic.servaddr.sin_addr.s_addr = INADDR_ANY; 
	info->basic.servaddr.sin_port = htons(port);

	if( (status=bind(info->basic.socket_fd, 
		(const struct sockaddr*)&info->basic.servaddr, 
		sizeof(info->basic.servaddr))) < 0 )
	{
		return status;
	}	

	info->callback = callback;
	info->arg = arg;

	if( (status=pthread_create(&info->basic.thread_handle, 
		NULL, 
		(void*(*)(void*))udp_server_thread, 
		info)) != 0 ) return status;

	return 0;
}

int udp_server_close( udp_server_info_t * info )
{
	return close( info->basic.socket_fd );
}

void * tcp_server_thread( tcp_server_info_t * info )
{
	int client_fd = -1;
	struct sockaddr_in client_addr = {};
	socklen_t addr_len = sizeof(client_addr);

	while( 1 )
	{
		client_fd = accept(info->basic.socket_fd, (struct sockaddr*)&client_addr, &addr_len);

		if( client_fd > 0 )
				fprintf(stderr, "%s:%d connected\n", 
						inet_ntoa(client_addr.sin_addr), 
						ntohs(client_addr.sin_port));
		else
				fprintf(stderr, "connection error\n");

		if( info->callback ) info->callback(client_fd, &client_addr, info->arg);

		close( client_fd );
	}

	return NULL;
}

int tcp_server_init( uint32_t port, tcp_callback_t callback, void * arg, tcp_server_info_t * info )
{
	int status = 0;

	bzero(info, sizeof(udp_server_info_t));
	status = info->basic.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if( status < 0 ) 
	{
		fprintf(stderr, "cant create socket\n");
		return status;
	}

	status = setsockopt(info->basic.socket_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	if( status < 0 ) 
	{
		fprintf(stderr, "cant set socket opt\n");
		return status;
	}
	
	info->basic.servaddr.sin_family = AF_INET;
    info->basic.servaddr.sin_addr.s_addr = INADDR_ANY; 
	info->basic.servaddr.sin_port = htons(port);

	if( (status=bind(info->basic.socket_fd, 
		(const struct sockaddr*)&info->basic.servaddr, 
		sizeof(info->basic.servaddr))) < 0 )
	{
		fprintf(stderr, "cant bind socket %s\n", strerror(errno));
		return status;
	}	

	if( (status = listen(info->basic.socket_fd, 20)) < 0 )
	{
		fprintf(stderr, "cant listen socket\n");
		return status;
	}

	info->callback = callback;
	info->arg = arg;

	if( (status=pthread_create(&info->basic.thread_handle, 
		NULL, 
		(void*(*)(void*))tcp_server_thread, 
		info)) != 0 ) return status;

	return 0;
}


int tcp_server_close( tcp_server_info_t * info )
{
	return close( info->basic.socket_fd );
}
