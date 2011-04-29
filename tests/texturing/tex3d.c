/*
 * Copyright (c) The Piglit project 2008
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 * Tests 3D textures.
 */

#include "piglit-util.h"

int piglit_width = 128, piglit_height = 128;
int piglit_window_mode = GLUT_RGB | GLUT_ALPHA;

static GLuint Texture;

static int nrcomponents(GLenum format)
{
	switch(format) {
	case GL_RGBA: return 4;
	case GL_RGB: return 3;
	case GL_ALPHA: return 1;
	default: abort();
	}
}

static const char* formatname(GLenum format)
{
	switch(format) {
	case GL_RGBA: return "GL_RGBA";
	case GL_RGB: return "GL_RGB";
	case GL_ALPHA: return "GL_ALPHA";
	default: abort();
	}
}

static void expected_rgba(GLenum format, const unsigned char* texdata, unsigned char* expected)
{
	switch(format) {
	case GL_RGBA:
		expected[0] = texdata[0];
		expected[1] = texdata[1];
		expected[2] = texdata[2];
		expected[3] = texdata[3];
		return;
	case GL_RGB:
		expected[0] = texdata[0];
		expected[1] = texdata[1];
		expected[2] = texdata[2];
		expected[3] = 255;
		return;
	case GL_ALPHA:
		expected[0] = 255;
		expected[1] = 255;
		expected[2] = 255;
		expected[3] = texdata[0];
		return;
	}
}

static int render_and_check(int w, int h, int d, GLenum format, float q, unsigned char* data, const char* test)
{
	int x, y, z;
	int layer;
	unsigned char* readback;
	unsigned char* texp;
	unsigned char* readp;
	int ncomp = 0;

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_TEXTURE_3D);

	x = y = 0;
	for(layer = 0; layer < d; ++layer) {
		float r = (layer+0.5)/d;
		glBegin(GL_QUADS);
			glTexCoord4f(0, 0, r*q, q);
			glVertex2f(x, y);
			glTexCoord4f(q, 0, r*q, q);
			glVertex2f(x+w, y);
			glTexCoord4f(q, q, r*q, q);
			glVertex2f(x+w, y+h);
			glTexCoord4f(0, q, r*q, q);
			glVertex2f(x, y+h);
		glEnd();
		x += w;
		if (x >= piglit_width) {
			y += h;
			x = 0;
		}
	}

	readback = (unsigned char*)malloc(w*h*d*4);
	x = y = 0;
	for(layer = 0; layer < d; ++layer) {
		glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, readback+layer*w*h*4);
		x += w;
		if (x >= piglit_width) {
			y += h;
			x = 0;
		}
	}

	texp = data;
	readp = readback;
	ncomp = nrcomponents(format);
	for(z = 0; z < d; ++z) {
		for(y = 0; y < h; ++y) {
			for(x = 0; x < w; ++x, readp += 4, texp += ncomp) {
				unsigned char expected[4];
				int i;
				expected_rgba(format, texp, expected);
				for(i = 0; i < 4; ++i) {
					if (expected[i] != readp[i]) {
						fprintf(stderr, "%s: Mismatch at %ix%ix%i\n", test, x, y, z);
						fprintf(stderr, " Expected: %i,%i,%i,%i\n",
							expected[0], expected[1], expected[2], expected[3]);
						fprintf(stderr, " Readback: %i,%i,%i,%i\n",
							readp[0], readp[1], readp[2], readp[3]);
						free(readback);
						return 0;
					}
				}
			}
		}
	}
	free(readback);

	glutSwapBuffers();

	return 1;
}


/**
 * Load non-mipmapped 3D texture of the given size
 * and check whether it is rendered correctly.
 */
static void test_simple(int w, int h, int d, GLenum format)
{
	int size;
	unsigned char *data;
	int i;
	int success = 1;

	assert(1 <= w && w <= 16);
	assert(1 <= h && h <= 16);
	assert(1 <= d && d <= 16);
	assert(format == GL_RGBA || format == GL_RGB || format == GL_ALPHA);

	size = w*h*d*nrcomponents(format);
	data = (unsigned char*)malloc(size);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	srand(size); /* Generate reproducible image data */
	for(i = 0; i < size; ++i)
		data[i] = rand() & 255;

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage3D(GL_TEXTURE_3D, 0, format, w, h, d, 0, format, GL_UNSIGNED_BYTE, data);

	success = success && render_and_check(w, h, d, format, 1.0, data, "Render 3D texture");
	success = success && render_and_check(w, h, d, format, 1.4, data, "Render 3D texture (q != 1.0)");

	free(data);

	if (!success) {
		fprintf(stderr, "Failure with texture size %ix%ix%i, format = %s\n",
			w, h, d, formatname(format));
		piglit_report_result(PIGLIT_FAIL);
	}
}

enum piglit_result
piglit_display(void)
{
	GLenum formats[] = { GL_RGBA, GL_RGB, GL_ALPHA };
	int w, h, d, fmt;

	for(fmt = 0; fmt < sizeof(formats)/sizeof(formats[0]); ++fmt) {
		for(w = 1; w <= 16; w *= 2) {
			for(h = 1; h <= 16; h *= 2) {
				for(d = 1; d <= 16; d *= 2) {
					test_simple(w, h, d, formats[fmt]);
				}
			}
		}
	}

	return PIGLIT_PASS;
}


static void Reshape(int width, int height)
{
	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}


void
piglit_init(int argc, char **argv)
{
	if (!GLEW_VERSION_1_2) {
		printf("Requires OpenGL 1.2\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_automatic = GL_TRUE;

	glutReshapeFunc(Reshape);

	glDisable(GL_DITHER);

	glGenTextures(1, &Texture);
	glBindTexture(GL_TEXTURE_3D, Texture);
	Reshape(piglit_width, piglit_height);
}
