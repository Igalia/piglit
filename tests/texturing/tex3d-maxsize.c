/*
 * Copyright (c) 2010 Marek Olšák <maraeo@gmail.com>
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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "tex3d-maxsize";


/*
 * Use proxy texture to find largest possible 3D texture size.
 */
static void
find_max_tex3d_size(GLint initSize, GLint *width, GLint *height, GLint *depth)
{
	GLint dim = 0, w, h, d, pw, ph, pd;

	w = h = d = initSize;

	while (w >= 1 && h >= 1 && d >= 1) {
		/* try proxy image */
		const int level = 0;

		glTexImage3D(GL_PROXY_TEXTURE_3D, level, GL_RGBA8,
			     w, h, d, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D, level,
					 GL_TEXTURE_WIDTH, &pw);
		glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D, level,
					 GL_TEXTURE_HEIGHT, &ph);
		glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D, level,
					 GL_TEXTURE_DEPTH, &pd);

		if (pw == w && ph == h && pd == d) {
			/* success! */
			*width = w;
			*height = h;
			*depth = d;
			return;
		}

		/* halve one of the dimensions and try again */
		if (dim == 0)
			w /= 2;
		else if (dim == 1)
			h /= 2;
		else
			d /= 2;

		dim = (dim + 1) % 3;
	}
}


enum piglit_result
piglit_display(void)
{
	GLuint tex;
	GLint maxsize, width, height, depth;
	GLenum err;
	char *data;
	int i, j;
	GLboolean pass = GL_TRUE;
	float c1[4] = {0.25, 0.25, 0.25, 1.0};
	float c2[4] = {0.75, 0.75, 0.75, 1.0};

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &maxsize);

	/* Create the texture. */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_3D, tex);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	if (glGetError())
		return PIGLIT_FAIL;

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, maxsize, maxsize, maxsize, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	err = glGetError();

	if (err == GL_OUT_OF_MEMORY) {

		/* use proxy texture to find working max texture size */
		width = height = depth = 0;
		find_max_tex3d_size(maxsize, &width, &height, &depth);

		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, width, height, depth, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		err = glGetError();

		if (err == GL_OUT_OF_MEMORY)
			return PIGLIT_PASS;
	}
	else {
		/* the max 3D texture size actually worked */
		width = height = depth = maxsize;
	}

	if (err != GL_NO_ERROR) {
		printf("%s: unexpected glTexImage3D error: 0x%x\n",
		       TestName, err);
		return PIGLIT_FAIL;
	}

	if (0)
		printf("max 3D texture size = %d x %d x %d\n",
		       width, height, depth);

	/* Set its pixels, slice by slice. */
	data = malloc(width * height * 4);
	for (j = 0; j < height; j++)
		for (i = 0; i < width; i++) {
			int a = (j * width + i) * 4;
			data[a+0] =
			data[a+1] =
			data[a+2] =
			data[a+3] = (i * 255) / (width - 1);
		}

	for (i = 0; i < depth; i++) {
		glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, i, width, height, 1,
				GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	free(data);

	/* Now try basic rendering. */
	glEnable(GL_TEXTURE_3D);
	glBegin(GL_QUADS);
	glTexCoord3f(0, 0, 1);
	glVertex2f(0, 0);
	glTexCoord3f(0, 1, 1);
	glVertex2f(0, piglit_height);
	glTexCoord3f(1, 1, 1);
	glVertex2f(piglit_width, piglit_height);
	glTexCoord3f(1, 0, 1);
	glVertex2f(piglit_width, 0);
	glEnd();
	glDeleteTextures(1, &tex);

	pass = piglit_probe_pixel_rgb(piglit_width * 1 / 4,
				      piglit_height * 1 / 4, c1) && pass;
	pass = piglit_probe_pixel_rgb(piglit_width * 3 / 4,
				      piglit_height * 1 / 4, c2) && pass;
	pass = piglit_probe_pixel_rgb(piglit_width * 1 / 4,
				      piglit_height * 3 / 4, c1) && pass;
	pass = piglit_probe_pixel_rgb(piglit_width * 3 / 4,
				      piglit_height * 3 / 4, c2) && pass;
	piglit_present_results();

	if (!pass) {
		printf("%s: failed at size %d x %d x %d\n", TestName,
		       width, height, depth);
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(12);

	glDisable(GL_DITHER);
}

