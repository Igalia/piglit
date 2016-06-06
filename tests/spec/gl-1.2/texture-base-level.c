/*
 * Copyright 2016 VMware, Inc.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * Test a texture construction / base level issue in Mesa/gallium state tracker.
 * The texture images are defined for levels 2, 3, ... and the height of
 * all images is one.
 * Mesa was asserting in this case.
 *
 * Brian Paul
 * 6 June 2016
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 12;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


const GLubyte gray = 220;


void
piglit_init(int argc, char **argv)
{
	const int width0 = 512;
	const int height = 1;
	const int base_level = 2;
	GLuint tex;
	int level;
	int nr_bytes;
	GLubyte *texdata;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	nr_bytes = (width0 >> base_level) * height * 4 * sizeof(GLubyte);
	texdata = malloc(nr_bytes);
	memset(texdata, gray, nr_bytes);

	for (level = base_level; ; level++) {
		int width = width0 >> level;
		if (width == 0)
			break;
		printf("level %d: %d x %d\n", level, width, height);
		glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, width, height, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, texdata);
	}

	free(texdata);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, base_level);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}


enum piglit_result
piglit_display(void)
{
	const float exp_color[4] = {gray / 255.0f, gray / 255.0f,
				    gray / 255.0f, gray / 255.0f};
	bool pass;

	glClearColor(1, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);

	piglit_draw_rect_tex(-1, -1, 2, 2, 0, 0, 1, 1);

	pass = piglit_probe_pixel_rgba(piglit_width / 2, piglit_height / 2,
				       exp_color);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
