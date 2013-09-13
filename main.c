#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include "libpng/png.h"
#include "exoquant.h"

#ifndef NULL
#define NULL (0)
#endif

void print_usage();
void convert_image(const char *pInName, const char *pOutName, int numColors,
				   int highQuality, int dither);

int main(int argn, const char **argc)
{
	const char *pOutputName;
	const char *pSuffix;
	int numColors;
	int highQuality;
	int dither;
	int arg;
	const char *pCurrentOption;

	if(argn < 2)
	{
		print_usage();
	}

	// default options
	pOutputName = NULL;
	pSuffix = "_8";
	numColors = 256;
	highQuality = 0;
	dither = 1;
	
	arg = 1;
	while(arg < argn)
	{
		pCurrentOption = argc[arg++];
		if(*pCurrentOption == '-')
		{
			if(arg == argn)
				print_usage();

			pCurrentOption++;
			if(*pCurrentOption == '-')
			{
				pCurrentOption++;
				if(strcmp(pCurrentOption, "output") == 0)
					pOutputName = argc[arg++];
				else if(strcmp(pCurrentOption, "num-colors") == 0)
					numColors = atoi(argc[arg++]);
				else if(strcmp(pCurrentOption, "high-quality") == 0)
					highQuality = 1;
				else if(strcmp(pCurrentOption, "auto") == 0)
					numColors = 0;
				else if(strcmp(pCurrentOption, "no-dither") == 0)
					dither = 0;
				else if(strcmp(pCurrentOption, "overwrite") == 0)
					pSuffix = "";
				else
					print_usage();
			}
			else
			{
				while(*pCurrentOption != 0)
				{
					switch(*pCurrentOption++)
					{
					case 'o':
						if(arg == argn)
							print_usage();
						pOutputName = argc[arg++];
						break;

					case 'n':
						if(arg == argn)
							print_usage();
						numColors = atoi(argc[arg++]);
						break;

					case 'h':
						highQuality = 1;
						break;

					case 'a':
						numColors = 0;
						break;

					case 'd':
						dither = 0;
						break;

					case 'v':
						pSuffix = "";
						break;

					default:
						printf("Unknown option '%c'\n", pCurrentOption[-1]);
						print_usage();
					}
				}
			}
		}
		else
		{
			if(pOutputName)
				convert_image(pCurrentOption, pOutputName, numColors,
					highQuality, dither);
			else
			{
				const char *ext;
				int len;
				char *buffer;

				ext = strrchr(pCurrentOption, '.');
				if(ext == NULL)
					ext = pCurrentOption + strlen(pCurrentOption);
				len = (int)(ext - pCurrentOption);
				
				buffer = (char *)malloc(len + strlen(pSuffix) + 5);
				memcpy(buffer, pCurrentOption, len);
				sprintf(buffer + len, "%s.png", pSuffix);
				
				convert_image(pCurrentOption, buffer, numColors, highQuality,
					dither);
				
				free(buffer);
			}
		}
	}

	return 0;
}

void print_usage()
{
	printf(
		"usage:  exoquant [options] pngfile [[options] pngfile ...]\n"
		"\n"
		"options:\n"
		"   -n  --num-colors    numbers of colors to quantize to. [256]\n"
		"   -o  --output        output name\n"
		"   -v  --overwrite     overwrite input\n"
		"   -a  --auto          automatically chose 16 or 256 colors\n"
		"                       for each image\n"
		"   -h  --high-quality  high-quality quantization. SLOW! VERY!\n"
		"   -d  --no-dither     disable dithering\n"
		"\n");

	exit(1);
}

typedef struct
{
	int width, height, ncolors;
	unsigned char *pData, *pPal;
} Image;

Image load_image(const char *pFilename);
void quantize_image(Image *pImage, int numColors, int highQuality, int dither);
void save_image(Image image, const char *pFilename);

void convert_image(const char *pInName, const char *pOutName, int numColors,
				   int highQuality, int dither)
{
	Image image;
	if(numColors > 0)
		printf("'%s' -> '%s', %d colors\n", pInName, pOutName, numColors);
	else
		printf("'%s' -> '%s', auto\n", pInName, pOutName);

	image = load_image(pInName);
	if(image.pData == NULL)
	{
		free(image.pData);
		return;
	}

	quantize_image(&image, numColors, highQuality, dither);

	save_image(image, pOutName);

	free(image.pData);
	free(image.pPal);
}

Image load_image(const char *pFilename)
{
	Image image;
	FILE *file;
	unsigned char header[8];
	int bit_depth;
	int channels;
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep *row_pointers;
	unsigned char *ptr, *row;
	int y, x;

	image.pData = NULL;
	image.pPal = NULL;

	file = fopen(pFilename, "rb");
	if(file == NULL)
	{
		printf("Unable to open '%s'\n", pFilename);
		return image;
	}

	fread(header, 8, 1, file);
	if(png_sig_cmp(header, 0, 8))
	{
		printf("'%s' is not a valid PNG file\n", pFilename);
		fclose(file);
		return image;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info_ptr = png_create_info_struct(png_ptr);

	png_init_io(png_ptr, file);
	png_set_sig_bytes(png_ptr, 8);

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, NULL);

	image.width = png_get_image_width(png_ptr, info_ptr);
	image.height = png_get_image_height(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	channels = png_get_channels(png_ptr, info_ptr);

	if(bit_depth != 8 || (channels != 3 && channels != 4))
	{
		printf("PNG format of file '%s' not supported\n", pFilename);
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return image;
	}

	row_pointers = png_get_rows(png_ptr, info_ptr);

	image.pData = (unsigned char*)malloc(image.width * image.height * 4);
	ptr = image.pData;
	for(y = 0; y < image.height; y++)
	{
		row = row_pointers[y];
		for(x = 0; x < image.width * 4; x++)
		{
			if(channels == 4 || (x & 3) != 3)
				*ptr++ = *row++;
			else
				*ptr++ = 255;
		}
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	return image;
}

void quantize_image(Image *pImage, int numColors, int highQuality, int dither)
{
	exq_data *pExq;
	unsigned char *pData;

	pExq = exq_init();
	exq_feed(pExq, pImage->pData, pImage->width * pImage->height);

	if(numColors > 0)
		exq_quantize_ex(pExq, numColors, highQuality);
	else
	{
		float err;
		exq_quantize_hq(pExq, 16);
		err = exq_get_mean_error(pExq);
		printf("error for 16 colors: %f, ", err);
		if(err < 8)
			numColors = 16;
		else
		{
			exq_quantize_ex(pExq, 256, highQuality);
			numColors = 256;
		}
		printf("using %d colors\n", numColors);
	}

	pImage->pPal = (unsigned char*)malloc(numColors * 4);
	pImage->ncolors = numColors;
	exq_get_palette(pExq, pImage->pPal, numColors);

	pData = (unsigned char*)malloc(pImage->width * pImage->height);
	if(dither)
		exq_map_image_ordered(pExq, pImage->width, pImage->height,
			pImage->pData, pData);
	else
		exq_map_image(pExq, pImage->width * pImage->height, pImage->pData,
			pData);

	exq_free(pExq);

	free(pImage->pData);
	pImage->pData = pData;
}

void save_image(Image image, const char *pFilename)
{
	FILE *file;
	png_structp png_ptr;
	png_infop info_ptr;
	int numbits, i, y;
	png_bytep *row_pointers;
	png_color pal[256];
	png_byte trans[256];

	file = fopen(pFilename, "wb");
	if(file == NULL)
	{
		printf("Unable to open output file\n");
		return;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info_ptr = png_create_info_struct(png_ptr);

	png_init_io(png_ptr, file);
	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

	numbits = 8;
	if(image.ncolors <= 2)
		numbits = 1;
	else if(image.ncolors <= 4)
		numbits = 2;
	else if(image.ncolors <= 16)
		numbits = 4;

	png_set_IHDR(png_ptr, info_ptr, image.width, image.height, numbits,
		PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	for(i = 0; i < image.ncolors; i++)
	{
		pal[i].red = image.pPal[i*4+0];
		pal[i].green = image.pPal[i*4+1];
		pal[i].blue = image.pPal[i*4+2];
		trans[i] = image.pPal[i*4+3];
	}
	png_set_PLTE(png_ptr, info_ptr, pal, image.ncolors);
	png_set_tRNS(png_ptr, info_ptr, trans, image.ncolors, NULL);

	row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * image.height);
	for(y = 0; y < image.height; y++)
		row_pointers[y] = image.pData + y * image.width;
	png_set_rows(png_ptr, info_ptr, row_pointers);

	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_PACKING, NULL);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	free(row_pointers);
}
