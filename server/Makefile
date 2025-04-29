TARGET = aesdsocket
CFILES := $(wildcard *.c)
OBJS =$(CFILES:.c=.o)


ifneq ($(CROSS_COMPILE),aarch64-none-linux-gnu-)
    CROSS_COMPILE =  
endif

CFLAGS += -g -Wall -Werror
CC = gcc
LD = ld
.PHONY: $(TARGET)
all: $(TARGET)
default: $(TARGET)
clean: 
	rm -f $(OBJS) $(TARGET)
.PHONY: clean
clean:
%.o : %.c
	$(CROSS_COMPILE)$(CC) -c $< $(CFLAGS) -o $@
    
$(TARGET) :   $(OBJS)
	$(CROSS_COMPILE)$(CC) $(OBJS) -o $(TARGET)
