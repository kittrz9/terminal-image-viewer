CC = gcc
SHELL = /bin/bash
CFLAGS = -Wall -Wpedantic 
LIBS = -lm
NAME = imgview

${NAME}: build-dir main.c
	${CC} ${CFLAGS} ${LIBS} main.c -o build/${NAME}

build-dir:
	-mkdir -p build

clean:
	-rm ./build/${NAME}*
