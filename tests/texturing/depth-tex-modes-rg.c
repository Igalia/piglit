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

/**
 * \file depth-tex-modes-rg.c
 * Draws depth textures as RED using both 2d textures and texture rectangles.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util-gl.h"
#include "depth-tex-modes-common.h"

static const GLenum depth_texture_modes[] = {
	GL_RED
};

#define BOX_SIZE 64
#define TEST_ROWS 4
#define TEST_COLS ARRAY_SIZE(depth_texture_modes)

#define TEST_WIDTH  (1+(TEST_COLS*(BOX_SIZE+1)))
#define TEST_HEIGHT (1+(TEST_ROWS*(BOX_SIZE+1)))


PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = MAX2(TEST_WIDTH, config.window_width);
	config.window_height = MAX2(TEST_HEIGHT, config.window_height);
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

GLuint tex[2];

void
piglit_init(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	piglit_require_extension("GL_ARB_texture_rg");
	depth_tex_init();

	if (!piglit_automatic)
		printf(" Lower row: Combined with color\n"
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
