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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * \file user-clip.c
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util-gl.h"

#define BOX_SIZE   32

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const GLfloat clear_color[4] = { 0.5, 0.5, 0.5, 1.0 };


enum piglit_result
piglit_display(void)
{
	static const GLfloat green[4] = { 0.0, 1.0, 0.0, 1.0 };
	static const GLfloat red[4] = { 1.0, 0.0, 0.0, 1.0 };
	enum piglit_result result = PIGLIT_PASS;

	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_TRIANGLES);
	glColor4fv(green);
	glVertex3f(-1.0,  1.0, 0.0);
	glColor4fv(red);
	glVertex3f( 2.0,  0.0, 1.0);
	glColor4fv(green);
	glVertex3f(-1.0, -1.0, 0.0);
	glEnd();

	if (!piglit_probe_pixel_rgb(piglit_width - 2,
				    piglit_height / 2,
				    clear_color)) {
		result = PIGLIT_FAIL;
	}

	piglit_present_results();
	return result;
}


void
piglit_init(int argc, char **argv)
{
	GLdouble clip_plane[4] = {
		0.0, 0.0, -1.0, 0.5
	};

	(void) argc;
	(void) argv;

	glClearColor(clear_color[0],
		     clear_color[1],
		     clear_color[2],
		     clear_color[3]);

	glClipPlane(GL_CLIP_PLANE0, clip_plane);
	glEnable(GL_CLIP_PLANE0);
}
