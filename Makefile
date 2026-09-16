.SILENT:
CC = gcc
CFLAGS = -Wall
FILES = src/sideFunctions.c src/main.c
build: main.c
	$(CC) $(CFLAGS) $(FILES) -o build/del
run:
	./build/del
