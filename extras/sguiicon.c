/*
 * extract_img.c
 * This file is part of sgui
 *
 * Copyright (C) 2012 - David Oberhollenzer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/*
    This utillity programm is supposed to extract RLE encoded icon images
    from sgui source files and generate RLE encoded icon data arrays from
    tga input images.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct {
	unsigned char *buffer;
	unsigned int width;
	unsigned int height;
} tga_image;


static void print_usage(void)
{
	puts("usage: sguiicon [-c|-e] infile [cmapfile]");
	puts("   -e extract sgui RLE compressed images from C source");
	puts("   -c compress a tga image to sgui RLE compressed C source\n");
	puts("   when compressing, an input source file with a color map");
	puts("   is requried\n");
}

static void read_array(FILE *in, unsigned char *data)
{
	char buffer[128], *end;
	int i;

	while (!feof(in)) {
		fgets(buffer, sizeof(buffer), in);

		if (!strncmp(buffer, "};", 2))
			break;

		for (i = 0; buffer[i] != '\n'; ++i) {
			if (isdigit(buffer[i])) {
				*(data++) = strtol(buffer + i, &end, 0);
				i = end - buffer;

				if (buffer[i]=='\n')
					break;
			}
		}
	}
}

static void print_array(const char *name, unsigned char *data, int bytes,
			unsigned int width, unsigned int height)
{
	char buffer[128], *ptr;
	int i, j;

	ptr = strrchr(name, '/');
	if (ptr)
		name = ptr;

	while (*name == '.')
		++name;

	strcpy(buffer, name);
	ptr = strchr(buffer, '.');
	if (ptr)
		*ptr = '\0';

	printf("/* SIZE %ux%u */\n", width, height);
	printf("static const unsigned char %s[%d] = {\n", buffer, bytes);

	for (j = 0, i = 0; i < bytes; ++i, ++data) {
		if (*data > 077) {
			printf("0x%X", *data);
			j += 4;
		} else if (*data > 007) {
			printf("0%d%d", ((*data)>>3 & 007), (*data) & 007);
			j += 3;
		} else {
			printf( "%d", *data );
			j += 1;
		}

		if ((i + 1) < bytes) {
			putchar(',');
			++j;
		}

		if ((j + 5) >= 80) {
			j = 0;
			putchar('\n');
		}
	}

	printf("\n};\n");
}

/****************************************************************************/

static void extract_image(tga_image *img, unsigned char *src,
			unsigned char *cmap, unsigned int colors,
			unsigned int bytes)
{
	unsigned char a, b, c, *dst = img->buffer;
	unsigned int i = 0, num_pixels, count = 0;

	num_pixels = img->height * img->width;

	while (i < num_pixels) {
		if (count) {
			--count;
		} else {
			if (!bytes)
				break;

			if (*src & 0x80) {
				count = (*src & 0x7F) - 1;
				++src;
				--bytes;
			} else if (*src & 0x40) {
				count = 1;
			} else {
				count = 0;
			}

			if (!bytes)
				break;
			c = *src;
			++src;
			--bytes;

			a = ((c & 070) >> 3);
			b = (c & 007);

			a = a < colors ? a : 0;
			b = b < colors ? b : 0;
		}

		*(dst++) = cmap[a * 4    ];
		*(dst++) = cmap[a * 4 + 1];
		*(dst++) = cmap[a * 4 + 2];
		*(dst++) = cmap[a * 4 + 3];
		++i;

		if (i < num_pixels) {
			*(dst++) = cmap[b * 4    ];
			*(dst++) = cmap[b * 4 + 1];
			*(dst++) = cmap[b * 4 + 2];
			*(dst++) = cmap[b * 4 + 3];
			++i;
		}
	}
}

static void convert_image(tga_image *img, unsigned char *dst,
			unsigned char *cmap, unsigned int colors, int *bytes)
{
	unsigned char c, *src = img->buffer;
	unsigned int i, x, y, r, g, b, a;
	*bytes = 0;

	for (y = 0; y < img->height; ++y) {
		for (x = 0; x < img->width; x += 2) {
			r = *(src++);
			g = *(src++);
			b = *(src++);
			a = *(src++);
			c = 0;

			for (i = 0; i < colors; ++i) {
				if (cmap[4 *i] == r && cmap[4 * i + 1] == g &&
					cmap[4 * i + 2] == b &&
					cmap[4 * i + 3] == a) {
					c = (i << 3) & 070;
					break;
				}
			}

			r = *(src++);
			g = *(src++);
			b = *(src++);
			a = *(src++);

			for (i = 0; i < colors; ++i) {
				if (cmap[4 * i] == r && cmap[4 * i + 1] == g &&
					cmap[4 * i + 2] == b &&
					cmap[4 * i + 3] == a) {
					c |= i & 007;
					break;
				}
			}

			*(dst++) = c;
			++(*bytes);
		}
	}
}

static void compress_rle(unsigned char *data, int *bytes)
{
	int i = 0, run;

	for (i = 0; i < *bytes; ++i) {
		for (run = 0; (i + run) < *bytes; ++run) {
			if (data[i + run] != data[i])
				break;
		}
		if (run == 2) {
			data[i] |= 0x40;
			memmove(data + i + 1, data + i + 2, *bytes - (i + 2));
			*bytes -= 1;
		} else if (run > 2) {
			run = run > 0x7F ? 0x7F : run;
			data[i] = 0x80 | run;
			memmove(data + i + 2, data + i + run,
				*bytes - (i + run));
			*bytes -= run - 2;
			++i;
		}
	}
}

/****************************************************************************/

static void write_tga(FILE *out, tga_image *img)
{
	unsigned char header[18], temp[4], *image = img->buffer;
	unsigned int x, y;

	memset(header, 0, 18);
	header[ 2] = 2;				/* rgba format */
	header[12] =  img->width      & 0xFF;
	header[13] = (img->width>>8)  & 0xFF;
	header[14] =  img->height     & 0xFF;
	header[15] = (img->height>>8) & 0xFF;
	header[16] = 32;			/* bits per pixel */
	header[17] = 0x28;			/* origin at the top */
	fwrite(header, 1, 18, out);

	for (y = 0; y < img->height; ++y) {
		for (x = 0; x < img->width; ++x) {
			temp[2] = *(image++);
			temp[1] = *(image++);
			temp[0] = *(image++);
			temp[3] = *(image++);
			fwrite(temp, 1, 4, out);
		}
	}
}

static int load_tga(tga_image *img, FILE *file)
{
	unsigned char header[18], temp, *ptr;
	int width, height, x, y;

	fread(header, 1, 18, file);
	fseek(file, header[0], SEEK_CUR);	/* Skip the image ID field */

	width = header[12] | (header[13] << 8);
	height = header[14] | (header[15] << 8);

	if (header[2] != 2 || header[1]) {
		fprintf(stderr, "input image must be uncompressed RGBA!\n");
		return 0;
	}
	if (header[16] != 32) {
		fprintf(stderr, "input image must have 32 bit per pixel!\n");
		return 0;
	}
	if (!width || !height) {
		fprintf(stderr, "input image has no size set!\n");
		return 0;
	}
	if (!(header[17] & 1 << 5) || (header[17] & 1 << 4)) {
		fprintf(stderr, "input image must have origin at top left!\n");
		return 0;
	}

	img->buffer = malloc(width * height * 4);
	img->width = width;
	img->height = height;
	fread(img->buffer, 4, width * height, file);

	for (ptr = img->buffer, y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x, ptr += 4) {
			temp = ptr[0];
			ptr[0] = ptr[2];
			ptr[2] = temp;
		}
	}
	return 1;
}

/****************************************************************************/

int main(int argc, char **argv)
{
	unsigned char *cmap = NULL, *data = NULL;
	int i, bytes, compress, colors;
	char linebuffer[128];
	tga_image image;
	FILE *in, *out;

	if (argc < 3) {
		print_usage();
		return 0;
	}

	if (!strcmp(argv[1], "-c")) {
		compress = 1;
	} else if (!strcmp(argv[1], "-e")) {
		compress = 0;
	} else {
		print_usage();
		return 0;
	}

	in = fopen(argv[2], "rb");
	if (!in) {
		perror(argv[2]);
		return -1;
	}

	if (compress) {
		if (argc < 4) {
			print_usage();
			fclose(in);
			return 0;
		}

		if (!load_tga(&image, in)) {
			fclose(in);
			return -1;
		}

		fclose(in);

		in = fopen(argv[3], "rb");
		if (!in) {
			perror(argv[3]);
			free(image.buffer);
			return -1;
		}

		colors = 0;
		while (!feof(in)) {
			fgets(linebuffer, sizeof(linebuffer), in);

			if (sscanf(linebuffer, "/* COLORMAP %d */",
				&colors) == 1) {
				cmap = malloc(4 * colors);
				fgets(linebuffer, sizeof(linebuffer), in);
				read_array(in, cmap);
				break;
			}
		}

		if (!colors) {
			fprintf(stderr, "No colormap found!\n");
			free(image.buffer);
			fclose(in);
			return -1;
		}

		data = malloc(image.width * image.height * 2);
		convert_image(&image, data, cmap, colors, &bytes);

		compress_rle(data, &bytes);
		print_array(argv[2], data, bytes, image.width, image.height);
	} else {
		image.buffer = NULL;
		image.width = 0;
		image.height = 0;

		while (!feof(in)) {
			fgets(linebuffer, sizeof(linebuffer), in);

			if (sscanf(linebuffer, "/* COLORMAP %d */",
				&colors) == 1) {
				free(cmap);
				cmap = malloc(4 * colors);

				fgets(linebuffer, sizeof(linebuffer), in);
				read_array(in, cmap);
			} else if (sscanf(linebuffer, "/* SIZE %ux%u */",
					&image.width, &image.height) == 2) {
				fgets( linebuffer, sizeof(linebuffer), in );

				i = strlen( "static const unsigned char " );
				memmove(linebuffer, linebuffer + i,
					strlen(linebuffer + i) + 1);

				i = strchr(linebuffer, '[') - linebuffer;
				sscanf(linebuffer + i, "[%d]", &bytes);

				strcpy(linebuffer + i, ".tga");
				printf("extracting '%s' size: %ux%u; "
					"compressed size: %d\n", linebuffer,
					image.width, image.height, bytes);

				free(image.buffer);
				free(data);
				image.buffer = malloc(image.width *
							image.height * 4);
				data = malloc(bytes);
				out = fopen(linebuffer, "wb");

				read_array(in, data);
				extract_image(&image, data, cmap,
						colors, bytes);

				write_tga(out, &image);
				fclose(out);
			}
		}
	}

	free(cmap);
	free(data);
	free(image.buffer);
	fclose(in);
	return 0;
}
