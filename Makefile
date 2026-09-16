.SILENT:
CC = gcc
CFLAGS = -Wall 

build: 
	$(CC) $(CFLAGS) main.c -o del
