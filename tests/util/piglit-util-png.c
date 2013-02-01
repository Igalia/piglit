/*
 * Copyright © 2007 Nicolai Haehnle
 * Copyright © 2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#ifdef PIGLIT_HAS_PNG
#include <png.h>
#endif

#define aborts(s) _abortf("piglit_write_png: %s", s)
#define abortf(s, ...) _abortf("piglit_write_png" s, __VA_ARGS__)
static void
_abortf(const char *s, ...)
{
	va_list args;
	va_start(args, s);
	vfprintf(stderr, s, args);
	fprintf(stderr, "\n");
	va_end(args);
	abort();
}

/* Write a PNG file.
 *
 * \param filename    The filename to write (i.e. "foo.png")
 * \param base_format GL_RGBA or GL_RGB
 * \param width       The width of the image
 * \param height      The height of the image
 * \param data        The image data stored as unsigned bytes
 * \param flip_y      Whether to flip the image upside down (for FBO data)
 */
void
piglit_write_png(const char *filename,
		 GLenum base_format,
		 int width,
		 int height,
		 GLubyte *data,
		 bool flip_y)
{
#ifndef PIGLIT_HAS_PNG
	aborts("Piglit not built with libpng support.");
#else
	FILE *fp;
	png_structp png;
	png_infop info;
	GLubyte *row;
	int bytes;
	int color_type;
	int y;

	switch (base_format) {
	case GL_RGBA:
		color_type = PNG_COLOR_TYPE_RGB_ALPHA;
		bytes = 4;
		break;
	case GL_RGB:
		color_type = PNG_COLOR_TYPE_RGB;
		bytes = 3;
		break;
	default:
		abortf("unknown format %04x", base_format);
		break;
	}

	fp = fopen(filename, "w");
	if (!fp)
		abortf("failed to open `%s'", filename);

	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png)
		aborts("png_create_write_struct() failed");

	info = png_create_info_struct(png);
	if (!info)
		aborts("png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png)))
		aborts("png_init_io failed");

	png_init_io(png, fp);

	/* write header */
	if (setjmp(png_jmpbuf(png)))
		aborts("Write error");

	png_set_IHDR(png, info, width, height,
		     8, color_type, PNG_INTERLACE_NONE,
		     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png, info);

	if (flip_y) {
		row = data + (height * width * bytes);
		for (y = 0; y < height; ++y) {
			row -= width * bytes;
			png_write_row(png, row);
		}
	} else {
		row = data;
		for (y = 0; y < height; ++y) {
			png_write_row(png, row);
			row += width * bytes;
		}
	}

	png_write_end(png, 0);

	fclose(fp);
#endif
}
