# Application name
CORE=delorean

OBJ_DIR=.
BIN_DIR=bin
LIB_DIR=../../lib
SRC_DIR=.
INC_DIR=../../include .
BIN_NAME=$(CORE)

CORE_SRC_DIR=../core
CORE_OBJ_DIR=../core

TOOL_SRCS=$(wildcard $(SRC_DIR)/*.c)
TOOL_INC=$(wildcard $(INC_DIR)/*.h)
CORE_SRCS=$(wildcard $(CORE_SRC_DIR)/*.c)

TOOL_OBJS=$(TOOL_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
CORE_OBJS=$(CORE_SRCS:$(CORE_SRC_DIR)/%.c=$(CORE_OBJ_DIR)/%.o)

#CFLAGS=-g -std=c99 -Wall -Werror -fPIC -I$(PROJ_BASE)/include -I$(HWETRACE)/include -I. -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -O2 
CFLAGS=-g -std=gnu99 -Wall -Werror -fPIC -I$(PROJ_BASE)/include -I$(HWETRACE)/include -I. -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DGDB_ENABLED $(OPT)

LDFLAGS= -lJudy -lrt

all:

HWE_HANDLER_LIB_DIR:=$(LIB_DIR)
HWE_HANDLER_BIN_DIR:=$(BIN_DIR)
HWE_HANDLER_BIN_NAME:=$(BIN_NAME)
HWE_HANDLER_OBJS:=$(TOOL_OBJS) $(CORE_OBJS)
HWETRACE_CFLAGS_ADD+=-DGDB_ENABLED -I$(PROJ_BASE)/include  
HWETRACE_LDFLAGS_ADD+= -lJudy

include $(HWETRACE)/makefile.handler

all: hwetrace_handler $(HWETRACE)/makefile.handler

%.o: %.c
	@echo "Compiling: $@"
	gcc -c -o $@ $< $(CFLAGS)

install: 
	@echo "Copying $(CORE) to bin folder"
	@mv  bin/$(CORE) $(PROJ_BASE)/bin/

clean:
	@rm -rf  $(OBJ_DIR)/*.o $(OBJ_DIR)/*.so
	@rm -rf  $(CORE_OBJ_DIR)/*.o $(CORE_OBJ_DIR)/*.so

mrproper: clean
	@rm -rf $(BIN) $(LIB)

.PHONY: clean mrproper all install

