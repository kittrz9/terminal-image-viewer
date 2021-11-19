CC = gcc
SHELL = /bin/bash
CFLAGS = $(shell pkg-config --cflags glfw3) -Wall -Wpedantic 
LIBS = -lm
NAME = imgview

${NAME}: main.c
	${CC} ${CFLAGS} ${LIBS} main.c -o build/${NAME}

clean:
	-rm ./build/${NAME}*
