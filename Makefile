.SILENT:

CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -O2
FILES = src/sideFunctions.c src/main.c

build: $(FILES)
	mkdir -p ./build
	$(CC) $(CFLAGS) $(FILES) -o ./build/del
