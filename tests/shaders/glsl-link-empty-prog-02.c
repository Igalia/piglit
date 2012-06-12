/*
 * Copyright © 2010 Intel Corporation
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

/** @file glsl-link-empty-prog-02.c
 *
 * Verify behavior when a program object with no shaders attached is used.
 * Neither the OpenGL spec nor the GLSL spec are explicit about what happens
 * in this case.  However, the correct behavior can be infered from a few bits
 * in the spec.  Section 2.15 of the GL 2.1 spec says:
 *
 *     "When the program object currently in use includes a vertex shader, its
 *     vertex shader is considered active and is used to process vertices. If
 *     the program object has no vertex shader, or no program object is
 *     currently in use, the fixed-function method for processing vertices is
 *     used instead."
 *
 * Section 3.11 of the OpenGL 2.1 spec says:
 *
 *     "When the program object currently in use includes a fragment shader,
 *     its fragment shader is considered active, and is used to process
 *     fragments. If the program object has no fragment shader, or no program
 *     object is currently in use, the fixed-function fragment processing
 *     operations described in previous sections are used."
 *
 * If there is no vertex shader in the program, fixed-function vertex state is
 * used.  If there is no fragment shader in the program, fixed-function
 * fragment state is used.  If there is no vertex shader and no fragment
 * shader in the program, fixed-function vertex and fragment state are used.
 *
 * This test configures some simple fixed-function vertex and fragment state.
 * It verifies that this state is used when an "empty" program is active.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util.h"

PIGLIT_GL_TEST_MAIN(
    32 /*window_width*/,
    32 /*window_height*/,
    GLUT_RGB | GLUT_DOUBLE)

static GLuint prog = 0;
static GLuint tex = 0;

static const float black[4] = { 0.0, 0.0, 0.0, 1.0 };
static const float white[4] = { 1.0, 1.0, 1.0, 1.0 };
static const float green[4] = { 0.0, 1.0, 0.0, 1.0 };

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;

	glColor4fv(green);
	piglit_draw_rect_tex(0.0, 0.0,
			     (float) piglit_width, (float) piglit_height,
			     0.0, 0.0, 1.0, 1.0);

	pass &= piglit_probe_pixel_rgb(0, 0, black);
	pass &= piglit_probe_pixel_rgb(piglit_width - 1, 0, green);
	pass &= piglit_probe_pixel_rgb(0, piglit_height - 1, green);
	pass &= piglit_probe_pixel_rgb(piglit_width - 1, piglit_height - 1,
				       black);

	glutSwapBuffers();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	if (piglit_get_gl_version() < 20) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	prog = glCreateProgram();

	glLinkProgram(prog);
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);

	glUseProgram(prog);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glEnable(GL_TEXTURE_2D);
	piglit_checkerboard_texture(tex, 0, 16, 16, 2, 2, black, white);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
