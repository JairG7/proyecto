CC = gcc
CFLAGS = -Wall -Werror
OUTPUT = i2c.out
DEBUG = -g
SOURCES = p4a.c
TREAD = wiringPi
all:
	$(CC) $(SOURCES) $(CFLAGS) -o $(OUTPUT) -l$(TREAD)
debug:
	$(CC) $(SOURCES) $(CFLAGS) $(DEBUG) -o $(OUTPUT) -l$(TREAD)
clear:
	rm -f $(OUTPUT)
fresh:
	make clean
	make all


