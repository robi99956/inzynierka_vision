#include <stdio.h>

#include "util.h"
#include "queue.h"
#include "net.h"

void udp_callback( udp_message_t * msg, udp_client_info_t * client )
{
	fprintf(stderr, "%s\n", (char*)msg->data);
}

int main( int argc, char ** argv )
{
	udp_client_info_t client_info;
	udp_server_info_t server_info = {.arg = &client_info};

	printf("server init: %d\n",
		udp_server_init(8080, (udp_callback_t)udp_callback, NULL, &server_info));

	pthread_join(server_info.basic.thread_handle, NULL);
}
