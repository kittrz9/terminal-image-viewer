#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <ctype.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define UNUSED __attribute((unused))

typedef struct {
	unsigned int r, g, b, a;
} terminalColor;

bool loadPNGtoBuffer(const char* filePath, terminalColor* buffer, unsigned int w, unsigned int h) {
	int imgWidth, imgHeight, channels;
	
	unsigned char* data = stbi_load(filePath, &imgWidth, &imgHeight, &channels, 4);
	
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
				.r=data[i*4],
				.g=data[i*4 + 1],
				.b=data[i*4 + 2],
				.a=data[i*4 + 3],
			};
			buffer[(y*w)+x] = color;
		}
	}
	
	return true;
}

// kinda dumb to have these unused parameters at the end but it's so that I don't get warnings when I assign the function pointer later
// since I'm assigning the function pointer with either this or the LUT function to avoid having to check the color mode every time in the loop
void printColorAtLocation(unsigned int x, unsigned int y, terminalColor c, UNUSED terminalColor* lut, UNUSED size_t lutSize) {
	printf("\033[%i;%iH", y, x);
	// arbitrary cutoff point
	if(c.a < 250) {
		// to make it show the actual terminal background
		printf("\033[0m ");
	} else {
		printf("\033[48;2;%i;%i;%im ", c.r, c.g, c.b);
	}
}

// colors are based on the color scheme of my terminal (the default one that comes with kitty)
// if you want it to have your color scheme you'd need to edit these yourself
terminalColor colorLUT[256] = {
	{0x00,0x00,0x00,0xff},  // black
	{0xcd,0x00,0x00,0xff},  // red
	{0x00,0xcd,0x00,0xff},  // green
	{0xcd,0xcd,0x00,0xff},  // yellow
	{0x00,0x00,0xee,0xff},  // blue
	{0xcd,0x00,0xcd,0xff},  // magenta
	{0x00,0xcd,0xcd,0xff},  // cyan
	{0xe5,0xe5,0xe5,0xff},  // white
	{0x7f,0x7f,0x7f,0xff},  // bright black
	{0xff,0x00,0x00,0xff},  // bright red
	{0x00,0xff,0x00,0xff},  // bright green
	{0xff,0xff,0x00,0xff},  // bright yellow
	{0x5c,0x5c,0xff,0xff},  // bright blue
	{0xff,0x00,0xff,0xff},  // bright magenta
	{0x00,0xff,0xff,0xff},  // bright cyan
	{0xff,0xff,0xff,0xff},  // bright white
};


// basically copied from https://github.com/kovidgoyal/kitty/blob/master/kitty/colors.c
void initLUT() {
	uint8_t offset = 16;
	// 6x6x6 color cube
	uint8_t valueRange[6] = {0x00, 0x5f, 0x87, 0xaf, 0xd7, 0xff};
	for(uint8_t i = 0; i < 216; ++i) {
		terminalColor c = {
			valueRange[(i / 36) % 6],
			valueRange[(i /  6) % 6],
			valueRange[ i       % 6],
			0xff,
		};
		colorLUT[i+offset] = c;
	}

	// grayscale
	offset = 232;
	for(uint8_t i = 0; i < 24; ++i) {
		uint8_t v = 8 + i * 10;
		terminalColor c = {
			v,
			v,
			v,
			0xff,
		};
		colorLUT[i+offset] = c;
	}
}

void printColorAtLocationWithLUT(unsigned int x, unsigned int y, terminalColor c, terminalColor* lut, size_t lutSize) {
	printf("\033[%i;%iH", y, x);
	if(c.a < 250) {
		printf("\033[0m ");
		return;
	}

	uint8_t newColor = 0;
	uint32_t newColorDistance = UINT32_MAX;
	uint16_t i;

	for(i = 0; i < lutSize; ++i) {
// signed int so it doesn't underflow if lut[i] is greater, will always be positive when squared
		int16_t deltaR = c.r - lut[i].r;
		int16_t deltaG = c.g - lut[i].g;
		int16_t deltaB = c.b - lut[i].b;
// https://stackoverflow.com/questions/4485229/rgb-to-closest-predefined-color
// still doesn't work perfectly but seems to not be going to gray as much
		uint32_t currentColorDistance = 
			deltaR*deltaR*0.299 +
			deltaG*deltaG*0.587 +
			deltaB*deltaB*0.114;
		if(currentColorDistance < newColorDistance) {
			newColor = i;
			newColorDistance = currentColorDistance;
		}
	}

	printf("\033[48;5;%im ", newColor);
}

typedef enum {
	COLOR_MODE_RGB,
	COLOR_MODE_8,
	COLOR_MODE_16,
	COLOR_MODE_256,
} colorModeEnum;

int main(int argc, char** argv) {
	unsigned int termWidth = 0;
	unsigned int termHeight = 0;
	colorModeEnum colorMode = COLOR_MODE_RGB;
	
	if(argc < 2) {
		printf(\
"Usage:\n\
\t%s [path to image] [parameters]\n\n\
\t-w\tSet the width of the displayed image\n\
\t-h\tSet the height of the displayed image\n\
\t-8\tRender the image in 8 color mode\n\
\t-x\tRender the image in 16 color mode\n\
\t-f\tRender the image in 256 color mode\n", argv[0]);
		exit(1);
	}
	
	char* filePath = NULL;
	for(int i = 1; i < argc; ++i){
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
				case '8':
					colorMode = COLOR_MODE_8;
					break;
				case 'x':
					colorMode = COLOR_MODE_16;
					break;
				case 'f':
					initLUT();
					colorMode = COLOR_MODE_256;
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

	size_t lutSize;
	if(colorMode == COLOR_MODE_8)   { lutSize = 8;   }
	if(colorMode == COLOR_MODE_16)  { lutSize = 16;  }
	if(colorMode == COLOR_MODE_256) { lutSize = 256; }

	// probably really dumb but I'm doing this to get the color mode checks out of the loop
	void (*functionPointer)(unsigned int x, unsigned int y, terminalColor c, terminalColor* lut, size_t lutSize);
	if(colorMode == COLOR_MODE_RGB) {
		functionPointer = printColorAtLocation;
	} else {
		functionPointer = printColorAtLocationWithLUT;
	}
	
	for(unsigned int x = 0; x < termWidth; ++x){
		for(unsigned int y = 0; y < termHeight; ++y){
			(*functionPointer)(x, y, terminalImage[(y*termWidth)+x], colorLUT, lutSize);
		}
	}
	// moves to the bottom since it messes up when displaying transparent images for some reason
	printf("\033[H\033[%iB\033[0m\n", termHeight);
	
	free(terminalImage);
	
	return 0;
}
