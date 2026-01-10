##############
# Makefile for CTC
##############
# include ../Arch.make
#暂时不用
.PHONY: all

CROSS_COMPILE ?=
CC = $(CROSS_COMPILE)gcc
CFLAGS = -fPIC -Wall -pthread -ggdb -c -lrt 
LDFLAGS = -pthread -lrt -L../obj/ -lctc_api
TARGET = ctc 

INCLUDE_DIR = -I../api -I../utils

# 默认目标：生成可执行文件ctc
all: $(TARGET)
	@echo "✅ 可执行文件生成完成: $(TARGET)"
	@echo "源文件列表: $(SOURCES)"
	@echo "目标文件列表: $(OBJS)"

# 核心规则：链接所有.o文件生成可执行文件
$(TARGET): 
	make -C ../ 
	@echo "🔗 链接生成可执行文件 $@"
	$(CC) main.c $(LDFLAGS) $(INCLUDE_DIR) -o $@ 
# $^是所有依赖的.o文件，$@是目标文件(ctc)

clean:
	@echo " CTC clean"
	make -C ../ clean
	@rm -rf $(OBJS_DIR) $(TARGET)
	@echo "CTC clean finished"