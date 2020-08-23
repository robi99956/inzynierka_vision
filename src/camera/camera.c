
/*******************************************************************************
#             uvccapture: USB UVC Video Class Snapshot Software                #
#This package work with the Logitech UVC based webcams with the mjpeg feature  #
#.                                                                             #
# 	Orginally Copyright (C) 2005 2006 Laurent Pinchart &&  Michel Xhaard   #
#       Modifications Copyright (C) 2006  Gabriel A. Devenyi                   #
#                                                                              #
# This program is free software; you can redistribute it and/or modify         #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; either version 2 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# This program is distributed in the hope that it will be useful,              #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program; if not, write to the Free Software                  #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA    #
#                                                                              #
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <linux/videodev2.h>

#include "data_types.h"
#include "camera.h"

int camera_init( char * device, struct vdIn * handle )
{
	int retval = 0;

	retval = init_videoIn(handle, device, CAMERA_FRAME_WIDTH, CAMERA_FRAME_HEIGTH, V4L2_PIX_FMT_YUYV, GRAB_METHOD_MMAP);
	if( retval < 0 ) return -1;

	//Reset all camera controls
	v4l2ResetControl(handle, V4L2_CID_BRIGHTNESS);
	v4l2ResetControl(handle, V4L2_CID_CONTRAST);
	v4l2ResetControl(handle, V4L2_CID_SATURATION);
	v4l2ResetControl(handle, V4L2_CID_GAIN);

	return 0;
}

int camera_grab_frame_yuv( struct vdIn * handle, yuv_buffer_t * out )
{
	// nie rób wycieków pamięci - zainicjalizowane bufory tu nie mogą być użyte
	if( out->data != NULL ) return -1;

	int retval = uvcGrab(handle);
	if( retval < 0 ) return -1;

	out->info = BUFFER_INFO(CAMERA_FRAME_WIDTH, CAMERA_FRAME_HEIGTH);
	out->data = (yuv_macropixel_t*)handle->framebuffer;

	return 0;
}

int camera_close( struct vdIn * handle )
{
	return close_v4l2(handle);
}
