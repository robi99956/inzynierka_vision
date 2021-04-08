EXEC_NAME = vision
EXEC_NAME_PC = control
TARGET_PASSWORD = onioneer
#TARGET_ADDRESS = omega-5BB1.local
TARGET_ADDRESS = 192.168.0.28
TARGET_USER = root
TARGET_PATH = /root/inzynierka/bin
MODE="RUN"

# Testy na PC
ifeq ($(MODE), "TEST")
		GTK_LIBS = `pkg-config --libs gtk+-2.0`
		GTK_FLAGS = `pkg-config --cflags gtk+-2.0` 
		CC = gcc
		CFLAGS = -std=gnu99 -Wall -g -Iinclude -pthread -DUSE_GTK $(GTK_FLAGS) -DTEST
endif

# Konfiguracja docelowa
ifeq ($(MODE), "RUN")
		GTK_LIBS :=
		GTK_FLAGS :=
		CC = /home/robertk/omega_gcc/bin/mipsel-openwrt-linux-gcc
		INCLUDE_DIR = /home/robertk/omega_gcc/include
		CFLAGS = -std=gnu99 -Wall -g -Iinclude -I$(INCLUDE_DIR) -pthread -DRUN
endif

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
			build/img_process/display.o \
			build/img_process/rgb_search.o
NET_OBJECTS = 		build/net/net.o \
					build/net/send_image.o
UTIL_OBJECTS = 		build/util/queue.o
PSOC_COM_OBJECTS = 	build/net0/net0.o \
			build/net0/uart.o \
			build/net0/psoc_com.o
MAIN_OBJECTS = 		build/omega_main.o

ALL_OMEGA_OBJECTS = $(CAMERA_OBJECTS) $(IMG_PROCESS_OBJECTS) $(NET_OBJECTS) \
		$(UTIL_OBJECTS) $(PSOC_COM_OBJECTS) $(MAIN_OBJECTS)

### Reguły ###

all: prepare omega final

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

prepare:
	@mkdir -p bin
	@mkdir -p build
	@mkdir -p build/camera
	@mkdir -p build/img_process
	@mkdir -p build/net
	@mkdir -p build/util
	@mkdir -p build/net0
	@export STAGING_DIR=/home/robertk/omega_sdk/staging_dir

omega: $(ALL_OMEGA_OBJECTS) 
	STAGING_DIR="" $(CC) $(CFLAGS) $(ALL_OMEGA_OBJECTS) $(GTK_LIBS) -o bin/$(EXEC_NAME)

final:
	$(info Make OK!)

upload:
	$(info Uploading binary to target device...)
	#sshpass -p '$(TARGET_PASSWORD)' pkill $(EXEC_NAME)
	sshpass -p '$(TARGET_PASSWORD)' scp bin/$(EXEC_NAME) $(TARGET_USER)@$(TARGET_ADDRESS):$(TARGET_PATH)/$(EXEC_NAME)

clean:
	rm -r build/ bin/ 2> /dev/null

