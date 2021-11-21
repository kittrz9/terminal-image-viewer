#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef struct {
	unsigned int r, g, b;
} terminalColor;

bool loadPNGtoBuffer(const char* filePath, terminalColor* buffer, unsigned int w, unsigned int h) {
	int imgWidth, imgHeight, channels;
	
	unsigned char* data = stbi_load(filePath, &imgWidth, &imgHeight, &channels, 3);
	
	if(!data) {
		printf("Couldn't load image at location \"%s\"\n", filePath);
		return false;
	}
	
	for(unsigned int y = 0; y < h; ++y) {
		for(unsigned int x = 0; x < w; ++x) {
			// has to be float for some reason
			unsigned int imgX = x * (imgWidth/(float)w);
			unsigned int imgY = y * (imgHeight/(float)h);
			unsigned int i = (imgY*imgWidth)+imgX;
			terminalColor color = {
				.r=data[i*3],
				.g=data[i*3 + 1],
				.b=data[i*3 + 2],
			};
			buffer[(y*w)+x] = color;
		}
	}
	
	return true;
}

void printColorAtLocation(unsigned int x, unsigned int y, terminalColor c) {
	// ansi escape codes are a mess lmao
	printf("\033[%i;%iH\033[38;2;%i;%i;%im\033[48;2;%i;%i;%im ", y, x, c.r, c.g, c.b, c.r, c.g, c.b);
}

int main(int argc, char** argv) {
	if(argc != 2) {
		printf("Usage:\n\t%s [path to image]\n", argv[0]);
		exit(1);
	}
	
	// https://iqcode.com/code/c/terminal-size-in-c
	unsigned int termWidth, termHeight;
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	
	termWidth = w.ws_col;
	termHeight = w.ws_row;
	
	terminalColor* terminalImage = malloc(sizeof(terminalColor) * termWidth * termHeight);
	
	if(!loadPNGtoBuffer(argv[1], terminalImage, termWidth, termHeight)){
		exit(1);
	}
	
	for(unsigned int x = 0; x < termWidth; ++x){
		for(unsigned int y = 0; y < termHeight; ++y){
			printColorAtLocation(x, y, terminalImage[(y*termWidth)+x]);
		}
	}
	printf("\n");
	
	free(terminalImage);
	
	return 0;
}
