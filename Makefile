CC = gcc
CFLAGS = -Iinclude

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:%.c=%.o)

TARGET = bin/huffman

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDLIBS)

.PHONY: tmpclean clean

tmpclean:
	find . -name *~ -exec rm -vf {} \;
clean: tmpclean
	rm -f $(OBJS) $(TARGET)
