##############
# Makefile for CTC
##############
# include ../Arch.make
#暂时不用
.PHONY: all objs

CC = $(CROSS_COMPILE)gcc
CFLAGS = -fPIC -Wall -pthread -ggdb -c -lrt -DSM2002_MULTI_IN_DEV
LDFLAGS = -pthread -lrt
TARGET = ctc 

# 存放c文件的文件夹
SOURCE_DIR = app/\
             infra/\
			 ./

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


# all:$(LIB)
# 	@echo "make ctc"
# 	@echo "SOURCES:$(SOURCES)"
# 	@echo "FILES $(FILES)"
# 	@echo "OBJS $(OBJS)"

# 默认目标：生成可执行文件ctc
all: $(TARGET)
	@echo "✅ 可执行文件生成完成: $(TARGET)"
	@echo "源文件列表: $(SOURCES)"
	@echo "目标文件列表: $(OBJS)"

# 核心规则：链接所有.o文件生成可执行文件
$(TARGET): $(OBJS)
	@echo "🔗 链接生成可执行文件 $@"
	$(CC) $(LDFLAGS) $^ -o $@ 
# $^是所有依赖的.o文件，$@是目标文件(ctc)

# $(LIB):$(OBJS)
# 	@echo $(OBJS)
# 	ar rcs $@ $^

VPATH = $(SOURCE_DIR)

$(OBJS_DIR)%.o:%.c
	@echo $<
	@mkdir -p $(OBJS_DIR)
	$(CC) $(CFLAGS) $(INCLUDE_DIR) $< -o $@

clean:
	@echo " CTC clean"
	@rm -rf $(OBJS_DIR)
	@echo "CTC clean finished"