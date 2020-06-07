#ifndef PBM_H
#define PBM_H
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MSG_LEN               20
#define HEADER_LEN            10
#define TYPE_LEN              2

typedef unsigned char uchar;

typedef struct {
	int width;
	int height;
	int maxValue;
	uchar *pixel;
	uchar type[3];
} image;

void readInput(const char * fileName, image *img) {

	/* we first open the file */
	FILE *input_file = fopen(fileName, "rb");
	if(input_file == 0) {
		puts("The file couldn't be opened");
	}

	/* reads the header of the file to see the format */
	char file_header[HEADER_LEN];
	fgets(file_header, HEADER_LEN, input_file);

	/* sets the type of the image */
	memcpy(img->type, file_header, TYPE_LEN);
	img->type[2] = '\0';
	if(memcmp(img->type, "P5", TYPE_LEN) != 0) {
        puts("The header type is wrong");
	}

	/* reads with and height of the image */
	if(fscanf(input_file, "%d %d\n", &(img->width), &(img->height)) != 2) {
		puts("The height and the width were not red corectly");
	}

	img->pixel = (uchar*) malloc(sizeof(char*) * img->height  * img->width);

	/* reads maxValue */
	if(fscanf(input_file, "%d\n", &(img->maxValue)) != 1) {
		puts("The height and the width were not red corectly");
	}

	/* reading the image content */
	if(memcmp(img->type, "P5", TYPE_LEN) == 0) {
		fread(img->pixel, img->width, img->height, input_file);
	}

  fclose(input_file);
}

void writeData(const char * fileName, image *img) {

	FILE *output_file = fopen(fileName, "wb+");
	if(output_file == 0) {
		puts("The file couldn't be opened");
	}

	/* wrinting the type of the image */
	fprintf(output_file, "%s\n", img->type);

	/* writing the width and the height of the image */
	fprintf(output_file, "%d %d\n", img->width, img->height);

	/* writng the maxValue*/
	fprintf(output_file, "%d\n", img->maxValue);

	/* write data back to file */
	fwrite(img->pixel, img->width, img->height, output_file);
	free(img->pixel);
	
	fclose(output_file);
}

#endif