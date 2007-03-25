#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <png.h>
#include <GL/gl.h>

static void abortf(const char * s, ...)
{
	va_list args;
	va_start(args, s);
	vfprintf(stderr, s, args);
	fprintf(stderr, "\n");
	va_end(args);
	abort();
}

/**
 * Write RGB or RGBA data to a PNG file.
 * format must be GL_RGB or GL_RGBA.
 */
void WritePNGImage(const char* filename,
		GLenum format, int width, int height, GLubyte* data, int reverse)
{
	FILE* fp;
	png_structp png;
	png_infop info;
	int bytes;
	int colortype;
	int y;
	GLubyte* row;

	if (format == GL_RGBA) {
		colortype = PNG_COLOR_TYPE_RGB_ALPHA;
		bytes = 4;
	} else if (format == GL_RGB) {
		colortype = PNG_COLOR_TYPE_RGB;
		bytes = 3;
	} else {
		abortf("Unknown format %04x", format);
	}

	fp = fopen(filename, "wb");
	if (!fp)
		abortf("Failed to open %s", filename);

	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png)
		abortf("png_create_write_struct failed");

	info = png_create_info_struct(png);
	if (!info)
		abortf("png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png)))
		abortf("png_init_io failed");

	png_init_io(png, fp);


	/* write header */
	if (setjmp(png_jmpbuf(png)))
		abortf("Write error");

	png_set_IHDR(png, info, width, height,
	             8, colortype, PNG_INTERLACE_NONE,
	             PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png, info);

	if (reverse)
		row = data + (height*width*bytes);
	else
		row = data;
	for(y = 0; y < height; ++y) {
		if (reverse)
			row -= width*bytes;
		png_write_row(png, row);
		if (!reverse)
			row += width*bytes;
	}

	png_write_end(png, 0);

	fclose(fp);
}
