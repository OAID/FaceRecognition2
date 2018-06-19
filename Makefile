SUBDIRS := facecaffe

TOPDIR := $(shell pwd)

export PKG_CONFIG_PATH=/usr/local/AID/pkgconfig

OBJS_DIR := $(TOPDIR)/obj/
BIN_DIR := $(TOPDIR)/bin/
FACES_DIR := $(TOPDIR)/faces/

CUR_SRCS := $(wildcard *.cpp)
CUR_OBJS = $(addprefix $(OBJS_DIR), $(patsubst %.cpp, %.o, ${CUR_SRCS}))

BIN := $(BIN_DIR)fp

CFLAGS = -g -O2 -Wall -ffunction-sections -fdata-sections
LDFLAGS = -Wl,--gc-sections

LIBS= -lpthread -ldl -lm

CFLAGS 	+= -DARCH_ARM -std=c++11 -DCPU_ONLY
CROSS_COMPILE = aarch64-linux-gnu-

INCLUDE_DIR := 	-I$(TOPDIR)/include/ \
				`pkg-config --cflags opencv` \
				`pkg-config --cflags caffe-hrt` \
				`pkg-config --cflags tengine` 
								
LIBS 	+=  `pkg-config --libs tengine` `pkg-config --libs caffe-hrt`  `pkg-config --libs computelibrary` `pkg-config --libs opencv`  -lglog -lboost_system

AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
CPP = $(CC) -E
STRIP = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
RM = -rm -rf 

export CC CXX  RM CFLAGS INCLUDE_DIR OBJS_DIR TOPDIR

all: DIRS $(BIN)

DIRS:
	mkdir -p $(OBJS_DIR) 
	mkdir -p $(BIN_DIR)
	mkdir -p $(FACES_DIR)

$(BIN): $(SUBDIRS) $(CUR_OBJS)
	$(CXX) $(CFLAGS)  $(INCLUDE_DIR) $(LDFLAGS) $(OBJS_DIR)*.o -o $@  $(LIBS)

$(SUBDIRS): SUBMAKE

SUBMAKE:
	$(foreach dir, $(SUBDIRS), make -C $(dir);)

$(CUR_OBJS):$(CUR_SRCS) 
	$(CC) -c $(CFLAGS) $(INCLUDE_DIR) $^ -o $@


clean:
	rm -f  ${BIN}  $(OBJS_DIR)/*.o 
