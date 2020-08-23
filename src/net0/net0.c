#include <stdint.h>
#include <stdlib.h>

#include "net0.h"

void net0_send_byte( net0_handle_t * net0_handle, uint8_t byte, net0_byte_type_t byte_type, uint8_t * control )
{
	if( byte_type == NET0_BYTE_TYPE_STANDARD )
	{
		if( byte == NET0_DLE || byte == NET0_ACK ||
		  byte == NET0_NAK || byte == NET0_STX ||
		  byte == NET0_ETX ) 
		{
			byte += 0x80;
			net0_handle->ll_write_byte(net0_handle, NET0_DLE);
			*control ^= NET0_DLE;
		}

		*control ^= byte;
	}

	net0_handle->ll_write_byte(net0_handle, byte);
}

void net0_send_frame( net0_handle_t * net0_handle, net0_frame_info_t * info, void * data )
{
	uint8_t control = 0;

	net0_send_byte(net0_handle, NET0_STX, NET0_BYTE_TYPE_SPECIAL, NULL);
	net0_send_byte(net0_handle, info->dst, NET0_BYTE_TYPE_STANDARD, &control);		
	net0_send_byte(net0_handle, info->src, NET0_BYTE_TYPE_STANDARD, &control);		
	net0_send_byte(net0_handle, info->cmd, NET0_BYTE_TYPE_STANDARD, &control);
	net0_send_byte(net0_handle, net0_handle->nco++, NET0_BYTE_TYPE_STANDARD, &control);		
	uint8_t * data_bytes = (uint8_t*)data;
	for( uint16_t i=0; i<info->size; i++ )
	{
		net0_send_byte(net0_handle, data_bytes[i], NET0_BYTE_TYPE_STANDARD, &control);
	}

	net0_send_byte(net0_handle, NET0_ETX, NET0_BYTE_TYPE_SPECIAL, NULL);		
	net0_send_byte(net0_handle, control, NET0_BYTE_TYPE_SPECIAL, NULL);
}

void net0_rx_state_reset( net0_handle_t * net0_handle )
{
	net0_handle->rec_buf_idx = 0;
	net0_handle->rec_control = 0;
	net0_handle->next_action = NET0_ACTION_DEFAULT;
}

void net0_send_ack_nak( net0_handle_t * net0_handle, uint8_t acknak )
{
	net0_handle->ll_write_byte(net0_handle, acknak);
}

// ca�e info - pole size, kt�re nie jest wysy�ane
#define SIZEOF_INFO_FIELD (sizeof(net0_frame_info_t)-sizeof(uint16_t))

void net0_handle_frame_rec( net0_handle_t * net0_handle, uint8_t control )
{
	if( net0_handle->rec_control == control ) // suma kontrolna si� zgadza
	{
		net0_frame_info_t info = *(net0_frame_info_t*)net0_handle->rec_buf;

		if( net0_handle->rec_buf_idx >= SIZEOF_INFO_FIELD ) // odejmowanie nic nie zepsuje
		{
			if( info.cmd & NET0_CMD_REQUIRE_ACK )
				net0_send_ack_nak(net0_handle, NET0_ACK);

			info.size = net0_handle->rec_buf_idx-SIZEOF_INFO_FIELD;
			net0_handle->frame_rec_callback(net0_handle, info, net0_handle->rec_buf+SIZEOF_INFO_FIELD);
		}
	}
	else
	{
		net0_handle->event_callback(net0_handle, NET0_EVENT_CONTROL_ERROR);
	}
}

void net0_process_byte( net0_handle_t * net0_handle, uint8_t byte )
{
	if( net0_handle->next_action == NET0_ACTION_CHECK_CONTROL )
	{
		net0_handle_frame_rec(net0_handle, byte);
		net0_rx_state_reset(net0_handle);
		return;
	}

	switch( byte )
	{
	case NET0_STX: // nowa ramka, reset maszyny stanow
		net0_rx_state_reset( net0_handle );
		break;

	case NET0_DLE: // kolejny bajt bedzie zmieniany
		net0_handle->next_action = NET0_ACTION_DECODE_DLE;
                net0_handle->rec_control ^= NET0_DLE;
		break;

	case NET0_ETX: // koniec danych, kolejny bajt to CRC
		net0_handle->next_action = NET0_ACTION_CHECK_CONTROL;
		break;

	case NET0_NAK:
		net0_handle->event_callback(net0_handle, NET0_EVENT_NAK);
		break;

	case NET0_ACK:
		net0_handle->event_callback(net0_handle, NET0_EVENT_ACK);
		break;

	default: // bajt zwyklych danych
		if( net0_handle->next_action == NET0_ACTION_DECODE_DLE )
		{
                        net0_handle->rec_control ^= byte;
			byte -= 0x80;
			net0_handle->next_action = NET0_ACTION_DEFAULT;
		}
                else
                    	net0_handle->rec_control ^= byte;

		if( net0_handle->rec_buf_idx == net0_handle->rec_buf_size ) 
                {
                	// nie ma wincyj miejsca, zresetuj odbiornik
			net0_rx_state_reset(net0_handle);
			net0_handle->event_callback(net0_handle, NET0_EVENT_OVERRUN);
			break;
                }

               	net0_handle->rec_buf[ net0_handle->rec_buf_idx++ ] = byte; 
	}
}

void net0_init( net0_handle_t * net0_handle )
{
	net0_handle->send_frame = net0_send_frame;
	net0_handle->ll_new_byte_rec = net0_process_byte;
}	
