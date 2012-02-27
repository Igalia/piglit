/*
 * (C) Copyright IBM Corporation 2004
 * All Rights Reserved.
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
 * \file blendminmax.c
 * 
 * Simple test of GL_EXT_blend_minmax functionality.  Four squares are drawn
 * with different blending modes, but all should be rendered with the same
 * final color.
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */

#include "piglit-util.h"

int piglit_width = 400;
int piglit_height = 200;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;
static const GLfloat Near = 5.0, Far = 25.0;

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int w = (piglit_width - 50) / 4;
	int h = piglit_height - 20;
	int start_x = 10;
	int next_x = 10 + w;
	float expected[4] = {0.5, 0.5, 0.5, 0.5};

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(0.2, 0.2, 0.8, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* This is the "reference" square.
	 */

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ZERO);
	glColor3f(0.5, 0.5, 0.5);
	piglit_draw_rect(start_x + next_x * 0, 10, w, h);

	/* GL_MIN and GL_MAX are supposed to ignore the blend function
	 * setting.  To test that, we set the blend function to
	 * GL_ZERO for both color and alpha each time GL_MIN or GL_MAX
	 * is used.
	 *
	 * Apple ships an extension called
	 * GL_ATI_blend_weighted_minmax (supported on Mac OS X 10.2
	 * and later).  I believe the difference with that extension
	 * is that it uses the blend function.  However, I have no
	 * idea what the enums are for it.  The extension is listed at
	 * Apple's developer site, but there is no documentation.
	 *
	 * http://developer.apple.com/opengl/extensions.html
	 */

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ZERO);
	glColor3f(0.5, 0.5, 0.5);
	piglit_draw_rect(start_x + next_x * 1, 10, w, h);

	glBlendEquation(GL_MAX);
	glBlendFunc(GL_ZERO, GL_ZERO);
	glColor3f(0.2, 0.2, 0.2);
	piglit_draw_rect(start_x + next_x * 1, 10, w, h);

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ZERO);
	glColor3f(0.5, 0.5, 0.5);
	piglit_draw_rect(start_x + next_x * 2, 10, w, h);

	glBlendEquation(GL_MIN);
	glBlendFunc(GL_ZERO, GL_ZERO);
	glColor3f(0.8, 0.8, 0.8);
	piglit_draw_rect(start_x + next_x * 2, 10, w, h);

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ZERO);
	glColor3f(0.8, 0.8, 0.8);
	piglit_draw_rect(start_x + next_x * 3, 10, w, h);

	glBlendEquation(GL_MIN);
	glBlendFunc(GL_ZERO, GL_ZERO);
	glColor3f(0.5, 0.5, 0.5);
	piglit_draw_rect(start_x + next_x * 3, 10, w, h);

	pass = piglit_probe_pixel_rgb(15 + next_x * 0, piglit_height / 2,
				      expected) && pass;
	pass = piglit_probe_pixel_rgb(15 + next_x * 1, piglit_height / 2,
				      expected) && pass;
	pass = piglit_probe_pixel_rgb(15 + next_x * 2, piglit_height / 2,
				      expected) && pass;
	pass = piglit_probe_pixel_rgb(15 + next_x * 3, piglit_height / 2,
				      expected) && pass;

	glutSwapBuffers();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	if (!GLEW_ARB_imaging && !GLEW_EXT_blend_minmax) {
		printf("Sorry, this program requires either GL_ARB_imaging or "
		       "GL_EXT_blend_minmax.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	if (piglit_get_gl_version() < 14) {
		printf("Requires OpenGL 1.4\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	printf("\nAll 4 quads should be the same color.\n");
	glEnable(GL_BLEND);
}
