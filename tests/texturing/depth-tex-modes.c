/*
 * Copyright © 2009 Intel Corporation
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

// author: Ben Holmes

/*
 * Draws depth textures as LUMINANCE, INTENSITY, and ALPHA using both 2d
 * textures and texture rectangles.
 */

#include "piglit-util-gl.h"
#include "depth-tex-modes-common.h"

static const GLenum depth_texture_modes[] = {
	GL_ALPHA, GL_LUMINANCE, GL_INTENSITY
};

#define BOX_SIZE 64
#define TEST_ROWS 4
#define TEST_COLS ARRAY_SIZE(depth_texture_modes)

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 1+(TEST_COLS*(BOX_SIZE+1));
	config.window_height = 1+(TEST_ROWS*(BOX_SIZE+1));
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

GLuint tex[2];

void
piglit_init(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	depth_tex_init();

	if (!piglit_automatic)
		printf(" Left to Right: ALPHA, LUMINANCE, INTENSITY\n"
		       " Lower row: Combined with color\n"
		       " Upper row: combined with alpha\n"
		       " pink: TEXTURE_2D green: TEXTURE_RECTANGLE\n");
}


enum piglit_result
piglit_display(void)
{
	return depth_tex_display(depth_texture_modes, 
				 ARRAY_SIZE(depth_texture_modes),
				 BOX_SIZE);
}
