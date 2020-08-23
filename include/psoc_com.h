#ifndef PSOC_COM_H
#define PSOC_COM_H

#include "net0.h"

#define PSOC_UART_BAUDRATE B115200
#define NET0_BUF_SIZE 512

typedef struct
{
	int fd;
} uart_data_t;

int uart_init( char * path );
void net0_ll_write( net0_handle_t * net0, uint8_t byte );
uint8_t net0_ll_read( net0_handle_t * net0 );

net0_handle_t * psoc_com_init( char * uart_path );
#endif
