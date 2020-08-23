#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#include "util.h"
#include "queue.h"
#include "net.h"
 
void * udp_client_thread( udp_client_info_t * info )
{
	int retval = 0;
	
	while(3)
	{
		udp_message_t * msg = pthread_queue_pop_front(&info->queue);

		retval = sendto(info->basic.socket_fd, (const char*)msg->data, msg->len, MSG_CONFIRM, 
			(const struct sockaddr*)&info->basic.servaddr, sizeof(info->basic.servaddr));

		if( retval < 0 )
		{
			// ...
		}
		
		RELEASE(msg->data);
		RELEASE(msg);
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
	if( info->basic.socket_fd < 0 ) return -1;
	
	info->basic.servaddr.sin_family = AF_INET;
    	info->basic.servaddr.sin_addr.s_addr = INADDR_ANY; 
	info->basic.servaddr.sin_port = htons(port);

	if( bind(info->basic.socket_fd, (const struct sockaddr*)&info->basic.servaddr, sizeof(info->basic.servaddr)) < 0 )
	{
		return -1;
	}	

	info->callback = callback;
	info->arg = arg;

	if( pthread_create(&info->basic.thread_handle, NULL, (void*(*)(void*))udp_server_thread, info) != 0 ) return -1;

	return 0;
}

int udp_server_close( udp_server_info_t * info )
{
	return close( info->basic.socket_fd );
}
/*
// Driver code 
int main() { 
    int sockfd; 
    char buffer[MAXLINE]; 
    struct sockaddr_in servaddr, cliaddr; 
      
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

	printf("socket created\n");
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    socklen_t len = sizeof(cliaddr);  //len is value/resuslt 
  
	while(1)
	{
	    recvfrom(sockfd, (char *)buffer, MAXLINE,  
			MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
			&len); 
	    printf("Client: [%d]\n", buffer[0]); 
	}
      
    return 0; 
} */
