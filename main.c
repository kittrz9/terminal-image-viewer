#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <ctype.h>
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
	unsigned int termWidth = 0;
	unsigned int termHeight = 0;
	
	if(argc < 2) {
		printf("Usage:\n\t%s [path to image] [parameters]\n\n\t-w\tSet the width of the displayed image\n\t-h\tSet the height of the displayed image\n", argv[0]);
		exit(1);
	}
	
	char* filePath = NULL;
	for(unsigned int i = 1; i < argc; ++i){
		if(argv[i][0] == '-') {
			// check second character in the parameter
			switch(argv[i][1]){
				case 'w':
					if(isdigit(argv[++i][0])){
						termWidth = atoi(argv[i]);
					}
					break;
				
				case 'h':
					if(isdigit(argv[++i][0])){
						termHeight = atoi(argv[i]);
					}
					break;
				
				default:
					printf("Unrecognized parameter \"%s\"\n", argv[i]);
					exit(1);
					break;
			}
		} else {
			filePath = argv[i];
		}
	}
	if(filePath == NULL) {
		printf("You need to provide an image\n");
		exit(1);
	}
	
	// https://iqcode.com/code/c/terminal-size-in-c
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	
	if(termWidth < 1) {
		termWidth = w.ws_col;
	}
	if(termHeight < 1) {
		termHeight = w.ws_row;
	}
	
	terminalColor* terminalImage = malloc(sizeof(terminalColor) * termWidth * termHeight);
	
	if(!loadPNGtoBuffer(filePath, terminalImage, termWidth, termHeight)){
		exit(1);
	}
	
	for(unsigned int x = 0; x < termWidth; ++x){
		for(unsigned int y = 0; y < termHeight; ++y){
			printColorAtLocation(x, y, terminalImage[(y*termWidth)+x]);
		}
	}
	printf("\033[38;2;255;255;255m\033[48;2;0;0;0m\n");
	
	free(terminalImage);
	
	return 0;
}
