##############
# Makefile for CTC
##############
# include ../Arch.make
#暂时不用
.PHONY: all objs

CC = $(CROSS_COMPILE)gcc
CFLAGS = -fPIC -Wall -pthread -ggdb -c -lrt -DSM2002_MULTI_IN_DEV
LDFLAGS = -pthread -lrt

# 存放c文件的文件夹
SOURCE_DIR = app/\
             infra/\
			 utils/

INCLUDE_DIR = -I./ \
              -Iinfra/\
              -Iapi/\
              -I./Common/\
              -I./Include\
              -I./drivers/\
              -I./Hal/api \
			  -I./utils

OBJS_DIR = ./obj/
SOURCES := $(foreach dir, $(SOURCE_DIR), $(wildcard $(dir)*.c))
FILES := $(notdir $(SOURCES))
OBJS = $(patsubst %.c, $(OBJS_DIR)%.o, $(FILES))
LIB = $(OBJS_DIR)libctc_api.a


all:$(LIB)
	@echo "make ctc"
	@echo "SOURCES:$(SOURCES)"
	@echo "FILES $(FILES)"
	@echo "OBJS $(OBJS)"

$(LIB):$(OBJS)
	@echo $(OBJS)
	ar rcs $@ $^

VPATH = $(SOURCE_DIR)

$(OBJS_DIR)%.o:%.c
	@echo $<
	@mkdir -p $(OBJS_DIR)
	$(CC) $(CFLAGS) $(INCLUDE_DIR) $< -o $@

clean:
	@echo " CTC clean"
	@rm -rf $(OBJS_DIR)
	@echo "CTC clean finished"