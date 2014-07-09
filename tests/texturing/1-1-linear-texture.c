/*
 * Copyright © 2011 Intel Corporation
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
 *
 * Authors:
 *    Yuanhan Liu <yuanhan.liu@linux.intel.com>
 *
 */

/*
 * Tests that the 1:1 texture with filter set to linear is sampled correctly.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define DATA_SIZE	(piglit_width * piglit_height * 4)

static void * make_tex_data(void)
{
	int i;
	GLubyte *data = malloc(DATA_SIZE);

	for (i = 0; i < DATA_SIZE; i++) {
		if (i % 4 == 3)
			data[i] = 255;
		else
			data[i] = rand() % 256;
	}

	return data;
}

static GLboolean test(GLubyte *expected_data, GLubyte *data, int size)
{
	int i;
	int err_count = 0;
	GLubyte R0, G0, B0, A0;
	GLubyte R1, G1, B1, A1;

	for (i = 0; i < size; i += 4) {
		R0 = expected_data[i + 0];
		G0 = expected_data[i + 1];
		B0 = expected_data[i + 2];
		A0 = expected_data[i + 3];

		R1 = data[i + 0];
		G1 = data[i + 1];
		B1 = data[i + 2];
		A1 = data[i + 3];

		if (R0 != R1 || G0 != G1 || B0 != B1 || A0 != A1) {
			err_count++;
			if (err_count <= 10) {
				int x = (i / 4) % piglit_width;
				int y = (i / 4) / piglit_width;
				fprintf(stderr, "Error pixel at (%2d, %2d): "
				                "got (%3d, %3d, %3d, %3d), "
						"expected (%3d, %3d, %3d, %3d)\n",
						x, y,
						R1, G1, B1, A1,
						R0, G0, B0, A0);
			}
		}
	}

	if (err_count) {
		fprintf(stderr, "Got %d error pixels(%d total), "
			        "just first %d pixels shown\n",
				err_count, size / 4,
				err_count > 10 ? 10 : err_count);
	}

	return err_count == 0;
}

enum piglit_result
piglit_display(void)
{
	GLuint tex;
	GLboolean pass;
	GLubyte *tex_data = make_tex_data();
	GLubyte *out_data = malloc(DATA_SIZE);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, piglit_width, piglit_height,
	             0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_TEXTURE_2D);

	piglit_draw_rect_tex(-1, -1, 2, 2, 0, 0, 1, 1);

	glReadPixels(0, 0, piglit_width, piglit_height, GL_RGBA,
	             GL_UNSIGNED_BYTE, out_data);
	piglit_present_results();
	pass = test(tex_data, out_data, DATA_SIZE);

	free(tex_data);
	free(out_data);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_texture_rectangle");
}
