################################################
# Makefile for CommTrafficcenter app
#################################################
# include ../../Arch.make

.PHONY: all

# CC = $(CROSS_COMPILE)gcc
CC = gcc
CFLAGS = -fPIC -Wall -pthread -ggdb -c

INCLUDE_DIR = -I./ -I../infra/ -I../api/ -I../../Common/ -I../utils
SOURCE_DIR = ./
SOURCES += $(wildcard $(SOURCE_DIR)*.c)
OBJS = $(patsubst $(SOURCE_DIR)%.c, $(OBJS_DIR)%.o, $(SOURCES))
OBJS_DIR = ../obj/

all:$(OBJS)

$(OBJS_DIR)%.o:$(SOURCE_DIR)%.c
	@echo $(SOURCE_DIR)
	@echo $<
# @mkdir -p $(OBJS_DIR)
	$(CC) $(CFLAGS) $(INCLUDE_DIR) $< -o $@

clean:
	@echo "CommTrafficcenter app clean"
	@rm -rf $(OBJS_DIR)
	@echo "commTrafficcenter app clean finished"