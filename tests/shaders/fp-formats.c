/*
 * Copyright © 2009 Intel Corporation
 * Copyright © 2010 Red Hat, Inc.
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
 * \file fp-formats.c
 * Test fragment programs sampling from different texture formats.
 * (currently just alpha-only textures)
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/**
 * Modulate the primary color with the alpha channel of the texture
 */
static const char alpha_source[] =
	"!!ARBfp1.0\n"
        "TEMP texel0;\n"
        "TEX texel0,fragment.texcoord[0],texture[0],2D;\n"
        "MOV result.color, texel0.abgr;\n"
	"END"
	;

static GLint progs[1];
static GLuint textures[1];

enum piglit_result
piglit_display(void)
{
	enum piglit_result result = PIGLIT_PASS;

	const GLfloat expected[4] = { 0.5, 0.0, 0.0, 0.0 };

	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, progs[0]);

	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	piglit_draw_rect(0, 0, piglit_width, piglit_height);

	if (!piglit_probe_pixel_rgb(piglit_width / 2,
				    piglit_height / 2,
				    expected))
	  result = PIGLIT_FAIL;

	piglit_present_results();
	return result;
}


void
piglit_init(int argc, char **argv)
{
	const GLfloat alpha_data[] = { 0.5 };

	(void) argc;
	(void) argv;

	piglit_require_fragment_program();
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	progs[0] = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB,
					  alpha_source);

	glGenTextures(1, textures);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 1, 1, 0, GL_ALPHA, GL_FLOAT, alpha_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glClearColor(1.0, 1.0, 1.0, 1.0);
}
