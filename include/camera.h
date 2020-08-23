#ifndef CAMERA_H
#define CAMERA_H

#include "v4l2uvc.h"

#define CAMERA_FRAME_WIDTH 	640
#define CAMERA_FRAME_HEIGTH 	480

#define GRAB_METHOD_MMAP 1

int camera_init( char * device, struct vdIn * handle );
int camera_grab_frame_yuv( struct vdIn * handle, yuv_buffer_t * out );
int camera_close( struct vdIn * handle );
#endif
