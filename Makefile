CC = gcc
SHELL = /bin/bash
CFLAGS = -Wall -Wpedantic 
LIBS = -lm
NAME = imgview

${NAME}: main.c
	${CC} ${CFLAGS} ${LIBS} main.c -o build/${NAME}

clean:
	-rm ./build/${NAME}*
