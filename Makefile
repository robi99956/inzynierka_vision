CC = /home/robertk/omega_gcc/bin/mipsel-openwrt-linux-gcc
INCLUDE_DIR = /home/robertk/omega_gcc/include
CC_PC = gcc
EXEC_NAME = vision
EXEC_NAME_PC = control
TARGET_PASSWORD = onioneer
#TARGET_ADDRESS = omega-5BB1.local
TARGET_ADDRESS = 192.168.0.28
TARGET_USER = root
TARGET_PATH = /root/inzynierka/bin

GTK_LIBS = `pkg-config --libs gtk+-2.0` -lm
GTK_FLAGS = `pkg-config --cflags gtk+-2.0` 

CFLAGS = -std=gnu99 -Wall -g -Iinclude -I$(INCLUDE_DIR) -pthread
CFLAGS_PC = -std=gnu99 -Wall -g -Iinclude -pthread -DUSE_GTK $(GTK_FLAGS)

### Obiekty dla Omegi ###

CAMERA_OBJECTS = 	build/camera/v4l2uvc.o \
			build/camera/camera.o
IMG_PROCESS_OBJECTS = 	build/img_process/contour_processing.o \
			build/img_process/conversions.o \
			build/img_process/data_types.o \
			build/img_process/draw.o \
			build/img_process/filters.o \
			build/img_process/maze.o \
			build/img_process/scaling.o \
			build/img_process/transform.o \
			build/img_process/display.o
NET_OBJECTS = 		build/net/net.o
UTIL_OBJECTS = 		build/util/queue.o
PSOC_COM_OBJECTS = 	build/net0/net0.o \
			build/net0/uart.o \
			build/net0/psoc_com.o
MAIN_OBJECTS = 		build/omega_main.o

ALL_OMEGA_OBJECTS = $(CAMERA_OBJECTS) $(IMG_PROCESS_OBJECTS) $(NET_OBJECTS) \
		$(UTIL_OBJECTS) $(PSOC_COM_OBJECTS) $(MAIN_OBJECTS)

### Obiekty dla PC ###

CAMERA_PC_OBJECTS = 	build/camera/v4l2uvc_pc.o \
			build/camera/camera_pc.o
NET_PC_OBJECTS = 	build/net/net_pc.o 
UTIL_PC_OBJECTS = 	build/util/queue_pc.o
MAIN_PC_OBJECTS = 	build/pc_main_pc.o

ALL_PC_OBJECTS = $(MAIN_PC_OBJECTS) $(CAMERA_PC_OBJECTS) $(NET_PC_OBJECTS) $(UTIL_PC_OBJECTS)

### Reguły ###

all: prepare omega pc final

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

build/%_pc.o: src/%.c
	$(CC_PC) $(CFLAGS_PC) -c $< -o $@

prepare:
	@mkdir -p bin
	@mkdir -p build
	@mkdir -p build/camera
	@mkdir -p build/img_process
	@mkdir -p build/net
	@mkdir -p build/util
	@mkdir -p build/net0

omega: $(ALL_OMEGA_OBJECTS) 
	$(CC) $(CFLAGS) $(ALL_OMEGA_OBJECTS) -o bin/$(EXEC_NAME)

final:
	$(info Make OK!)

upload:
	$(info Uploading binary to target device...)
	sshpass -p '$(TARGET_PASSWORD)' scp bin/$(EXEC_NAME) $(TARGET_USER)@$(TARGET_ADDRESS):$(TARGET_PATH)/$(EXEC_NAME)

pc: $(ALL_PC_OBJECTS)
	$(CC_PC) $(CFLAGS_PC) $(ALL_PC_OBJECTS) $(GTK_LIBS) -o bin/$(EXEC_NAME_PC)

clean:
	rm -r build/ bin/ 2> /dev/null

