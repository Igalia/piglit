/*
 * Copyright 2015 VMware, Inc.
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

/**
 * Test glGetTextureSubImage with cube maps.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


static void
memset_series(unsigned *buffer, unsigned baseValue, unsigned size)
{
	unsigned i;
	for (i = 0; i < size; i++)
		buffer[i] = baseValue + i;
}


static bool
compare_series(const unsigned *buffer, unsigned baseValue, unsigned size)
{
	unsigned i;
	for (i = 0; i < size; i++) {
		if (buffer[i] != baseValue + i) {
			printf("Expected 0x%08x found 0x%08x\n",
			       baseValue + i, buffer[i]);
			return false;
		}

	}
	return true;
}


static bool
test_cubemap(void)
{
	GLuint tex;
	GLuint buffer[8*8];
	GLuint results[6*8*8];
	int level, face, imgStart;

	piglit_require_extension("GL_ARB_get_texture_sub_image");

	/* setup 8x8 mipmapped cube texture */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 4, GL_RGBA8, 8, 8);

	for (level = 0; level < 4; level++) {
		for (face = 0; face < 6; face++) {
			memset_series(buffer, face*10000+level*100,
				      sizeof(buffer)/4);
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
					level, 0, 0, 8 >> level, 8 >> level,
					GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
					buffer);
		}
	}

	/* test getting all six faces */
	for (level = 0; level < 4; level++) {
		/* get all six faces */
		memset(results, 0, sizeof(results));
		glGetTextureSubImage(tex, level,
				     0, 0, 0,  /* offset */
				     8 >> level, 8 >> level, 6, /* size */
				     GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
				     sizeof(results), results);

		/* check results */
		imgStart = 0;
		for (face = 0; face < 6; face++) {
			GLuint expected = face * 10000 + level * 100;
			int numTexels = (8 >> level) * (8 >> level);
			if (!compare_series(results + imgStart, expected,
					    numTexels)) {
				printf("Incorrect cubemap texel at "
				       "level %u, face %u\n",
				       level, face);
				return false;
			}
			imgStart += numTexels;
		}
	}

	/* Test getting face sub images (skip last 1x1 mipmap level)
	 * using four glGetTextureSubImage calls, one per quadrant.
	 * Note that each call retrieves the quadrant for all six faces
	 * at once.
	 */
	for (level = 0; level < 3; level++) {
		const int w = 4 >> level, h = 4 >> level;
		int x, y;

		memset(results, 0, sizeof(results));

		glPixelStorei(GL_PACK_ROW_LENGTH, w * 2);
		glPixelStorei(GL_PACK_IMAGE_HEIGHT, h * 2);

		/* lower-left */
		x = y = 0;
		glPixelStorei(GL_PACK_SKIP_PIXELS, x);
		glPixelStorei(GL_PACK_SKIP_ROWS, y);
		glGetTextureSubImage(tex, level,
				     x, y, 0,
				     w, h, 6,
				     GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
				     sizeof(results), results);

		/* lower-right */
		x = w;
		y = 0;
		glPixelStorei(GL_PACK_SKIP_PIXELS, x);
		glPixelStorei(GL_PACK_SKIP_ROWS, y);
		glGetTextureSubImage(tex, level,
				     x, y, 0,
				     w, h, 6,
				     GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
				     sizeof(results), results);

		/* upper-left */
		x = 0;
		y = h;
		glPixelStorei(GL_PACK_SKIP_PIXELS, x);
		glPixelStorei(GL_PACK_SKIP_ROWS, y);
		glGetTextureSubImage(tex, level,
				     x, y, 0,
				     w, h, 6,
				     GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
				     sizeof(results), results);

		/* upper-right */
		x = w;
		y = h;
		glPixelStorei(GL_PACK_SKIP_PIXELS, x);
		glPixelStorei(GL_PACK_SKIP_ROWS, y);
		glGetTextureSubImage(tex, level,
				     x, y, 0,
				     w, h, 6,
				     GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
				     sizeof(results), results);

		/* check results */
		imgStart = 0;
		for (face = 0; face < 6; face++) {
			GLuint expected = face * 10000 + level * 100;
			int numTexels = (8 >> level) * (8 >> level);
			if (!compare_series(results + imgStart, expected,
					    numTexels)) {
				printf("Incorrect cubemap texel at "
				       "level %u, face %u\n",
				       level, face);
				return false;
			}
			imgStart += numTexels;
		}
	}

	return true;
}


void
piglit_init(int argc, char **argv)
{
	bool pass = test_cubemap();
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	/* never called */
	return PIGLIT_PASS;
}
