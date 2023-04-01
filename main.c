#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_NO_GIF
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include "stb_image.h"
#include "stb_image_write.h"

int table[] = {
	-4, +0, -3, +1,
	+2, -2, +3, -1, 
	-3, +1, -4, +0,
	+3, -1, +2, -2
};

void MakeDither(unsigned char* origBuffer, unsigned char* outBuffer, int width, int height) {
	for(int yy = 0; yy < height; yy++) {
		for(int xx = 0; xx < width; xx++) {
			int pos = (xx + (yy * width)) * 3;
			int offs = table[(xx & 3) + ((yy & 3) << 2)];
			outBuffer[pos + 0] = (char) (CLAMP( (int)origBuffer[pos + 0] + offs, 0, 255) ) & 0b11111000;
			outBuffer[pos + 1] = (char) (CLAMP( (int)origBuffer[pos + 1] + offs, 0, 255) ) & 0b11111000;
			outBuffer[pos + 2] = (char) (CLAMP( (int)origBuffer[pos + 2] + offs, 0, 255) ) & 0b11111000;
		}
	}
}

unsigned char* pathBuf[0x7FFF] = {};

unsigned char* GetFnamePtr(unsigned char* str) {
	unsigned char* poibter = str + strlen(str);
	while(--poibter >= str) {
		if(*poibter == '/' || *poibter == '\\')
			return poibter+1; //i know, just don't be a retard
	}
	return str;
}

int main(int argc, char** argv) {
	puts(
		"Usage:\n"
		"ditherthingy single IMAGEPATH OUTPATH\n"
		"or\n"
		"ditherthingy batch IMAGESFOLDER OUTFOLDER\n"
	);
	
	if(argc != 1 + 3) { //needs 3 args
		puts("fuck you");
		return 1;
	}
	//printf("%s %s\n", argv[2], argv[3]);
	
	int width, height, channels;
	if(!strcmp(argv[1], "single")) {
		unsigned char* data = stbi_load(argv[2], &width, &height, &channels, 3);
		unsigned char* ditherData = (unsigned char*)malloc(width * height * 3);
		MakeDither(data, ditherData, width, height);
		stbi_image_free(data);
		stbi_write_png(argv[3], width, height, 3, ditherData, width*3);
		free(ditherData);
	} else if(!strcmp(argv[1], "batch")) {
		struct dirent* dir;
		DIR* d = opendir(argv[2]);
		if(d) {
			while(dir = readdir(d)) {
				struct stat lmao;
				stat(dir->d_name, &lmao);
				if(!(lmao.st_mode & S_IFDIR)) {
					strcpy(pathBuf, argv[2]);
					strcat(pathBuf, "/");
					strcat(pathBuf, dir->d_name);
					unsigned char* data = stbi_load(pathBuf, &width, &height, &channels, 3);
					unsigned char* ditherData = (unsigned char*)malloc(width * height * 3);
					MakeDither(data, ditherData, width, height);
					stbi_image_free(data);
					strcpy(pathBuf, argv[3]);
					strcat(pathBuf, "/");
					strcat(pathBuf, GetFnamePtr(dir->d_name));
					strcat(pathBuf, ".dither.png");
					//printf("%s\n", pathBuf);
					stbi_write_png(pathBuf, width, height, 3, ditherData, width*3);
					free(ditherData);
				}
			}
		}
	}
	return 0;
}