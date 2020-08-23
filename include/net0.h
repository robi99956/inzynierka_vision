#ifndef NET0_H
#define NET0_H
    
#include <stdint.h>

#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

// *** Znaki specjalne
#define NET0_DLE 0x10
#define NET0_ACK 0x06
#define NET0_NAK 0x15
#define NET0_STX 0x02
#define NET0_ETX 0x03

typedef enum PACKED _net0_next_action 
{
	NET0_ACTION_DEFAULT,
	NET0_ACTION_DECODE_DLE,
	NET0_ACTION_CHECK_CONTROL
} net0_next_action_t;

typedef enum PACKED _net0_byte_type 
{
	NET0_BYTE_TYPE_STANDARD,
	NET0_BYTE_TYPE_SPECIAL
} net0_byte_type_t;

typedef enum PACKED _net0_event_type
{
	NET0_EVENT_ACK,
	NET0_EVENT_NAK,
	NET0_EVENT_OVERRUN,
	NET0_EVENT_CONTROL_ERROR,
} net0_event_type_t;

typedef enum PACKED _net0_cmd_flags
{
	NET0_CMD_NETWORK_COM = 0x08,
	NET0_CMD_RESPONSE = 0x20,
	NET0_CMD_REQUEST = 0x40,
	NET0_CMD_REQUIRE_ACK = 0x80,
} net0_cmd_flags_t;

typedef struct PACKED _net0_frame_info 
{
	uint8_t dst;
	uint8_t src;
	uint8_t cmd;
	uint8_t nco; // ignorowane przy nadawaniu, powinno być 0
	uint16_t size;
} net0_frame_info_t;

typedef struct _net0_handle
{
	uint8_t * rec_buf;
	uint16_t rec_buf_size;
	uint16_t rec_buf_idx;
	uint8_t rec_control;
	net0_next_action_t next_action;

	uint8_t nco;

	void(*frame_rec_callback)(struct _net0_handle*, net0_frame_info_t, void*);
	void(*event_callback)(struct _net0_handle*, net0_event_type_t);
	void(*send_frame)(struct _net0_handle*, net0_frame_info_t*, void*);
	void(*ll_write_byte)(struct _net0_handle*, uint8_t);
	void(*ll_new_byte_rec)(struct _net0_handle*, uint8_t);

	void * user_data;
} net0_handle_t;

void net0_init( net0_handle_t * net0_handle );

#define NET0_DEFAULT_INFO(_size) (&(net0_frame_info_t){.size = _size})

#endif
