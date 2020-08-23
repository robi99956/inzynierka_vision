#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/serial.h>

#include "net0.h"
#include "psoc_com.h"

int uart_init( char * path )
{
	int fd = open(path, O_RDWR | O_NOCTTY);
	if( fd < 0 ) return fd;

        struct termios port_info;
	cfmakeraw(&port_info);

	cfsetispeed(&port_info, PSOC_UART_BAUDRATE);
	cfsetospeed(&port_info, PSOC_UART_BAUDRATE);
	
	port_info.c_cflag &= ~PARENB;	/* Disables the Parity Enable bit(PARENB),So No Parity   */
	port_info.c_cflag &= ~CSTOPB;	/* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
	port_info.c_cflag &= ~CSIZE;	/* Clears the mask for setting the data size             */
	port_info.c_cflag |= CS8;		/* Set the data bits = 8                                 */
	
	port_info.c_cflag &= ~CRTSCTS;   		/* No Hardware flow Control                      */
	port_info.c_cflag |= CREAD | CLOCAL; 	/* Enable receiver,Ignore Modem Control lines    */ 

	port_info.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG );

	if( tcsetattr(fd, TCSANOW, &port_info) != 0 )
	{
		close(fd);
		fprintf(stderr, "Port %s config error\n", path);
		return -1;
	}

	fprintf(stderr, "Port %s config OK\n", path);
	return fd;
}


void net0_ll_write( net0_handle_t * net0, uint8_t byte )
{
	uart_data_t * uart = (uart_data_t*)net0->user_data;

	write(uart->fd, &byte, 1);
}

uint8_t net0_ll_read( net0_handle_t * net0 )
{
	uart_data_t * uart = (uart_data_t*)net0->user_data;
	uint8_t byte;

	read(uart->fd, &byte, 1);
	return byte;
}
