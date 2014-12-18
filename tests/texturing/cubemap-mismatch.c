/*
 * Copyright (c) 2014 VMware, Inc.
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
 * Test cubemap with mismatched face sizes.
 * It's kind of crazy that OpenGL allows creating cube map textures with
 * mismatched face sizes, but it is what it is.  Do some basic checks
 * that no expected errors are raised and the per-face size queries work.
 */


#include "piglit-util-gl.h"



PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


enum piglit_result
piglit_display(void)
{
	GLuint tex;
	const int w = 64, h = 64;
	GLsizei sizes[6][2] = {
		{ w, h },
		{ w/2, h/2 },
		{ w/3, h/3 },
		{ w/2, h/2 },
		{ w, h },
		{ w/2, h/2 }
	};
	int face;
	bool pass = true;

	glGenTextures(1, &tex);

	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	/* six faces, different sizes */
	for (face = 0; face < 6; face++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA,
			     sizes[face][0], sizes[face][1], 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		pass = false;
	}

	/* test getting sizes */
	for (face = 0; face < 6; face++) {
		GLint tw, th;
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
					 0, GL_TEXTURE_WIDTH, &tw);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
					 0, GL_TEXTURE_HEIGHT, &th);
		if (tw != sizes[face][0] || th != sizes[face][1]) {
			printf("Bad texture size for face %d.\n", face);
			printf("  Expected %d x %d\n",
			       sizes[face][0], sizes[face][1]);
			printf("  Found %d x %d\n", tw, th);
			pass = false;
		}
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_texture_cube_map");
}
