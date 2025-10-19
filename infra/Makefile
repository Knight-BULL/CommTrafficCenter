###################################################
# Makefile for commTrafficcenter infra
###################################################

# include ../../Arch.make

.PHONY: all

# CC=$(CROSS COMPILE)gcc
CC = gcc
CFLAGS = -fPIC -Wall -pthread -ggdb -c -lrt

INCLUDE_DIR = -I./ -I../../common/ -I../../drivers -I../utils
SOURCE_DIR = ./
SOURCES += $(wildcard $(SOURCE_DIR)*.c)
OBJS = $(patsubst $(SOURCE_DIR)%.c, $(OBJS_DIR)%.o, $(SOURCES))
OBJS_DIR = ../obj/

all:$(OBJS)

$(OBJS_DIR)%.o:$(SOURCE_DIR)%.c
	@mkdir -p $(OBJS_DIR)
	$(CC) $(CFLAGS) $(INCLUDE_DIR) $< -o $@

clean:
	@echo "CommTrafficcenter infra clean"
	@rm -rf $(OBJS_DIR)
	@echo "CommTrafficcenter infra clean finished"
	