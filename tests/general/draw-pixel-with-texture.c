/*
 * Copyright (C) 2011 Intel Corporation
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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define SCREEN_SIZE_IN_PIXELS	(piglit_width * piglit_height * 4)


enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLfloat tex_data[2 * 2 * 4] = {
		1, 0, 0, 1, 	1, 0, 0, 1,
		1, 0, 0, 1, 	1, 0, 0, 1,
	};
	GLfloat *pixels;
	GLfloat expected[4] = {0.2, 0, 0, 1};
	int i;

	pixels = (GLfloat *) malloc(SCREEN_SIZE_IN_PIXELS * sizeof(GLfloat));

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_FLOAT, tex_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexCoord2f(0.5, 0.5);
	glEnable(GL_TEXTURE_2D);

	for (i = 0; i < SCREEN_SIZE_IN_PIXELS; i += 4) {
		pixels[i + 0] = 0.2;
		pixels[i + 1] = 1;
		pixels[i + 2] = 0;
		pixels[i + 3] = 1;
	}

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawPixels(piglit_width, piglit_height, GL_RGBA, GL_FLOAT, pixels);

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, expected);

	piglit_present_results();

	free(pixels);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
}
