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
 *
 * Authors:
 *    Ben Holmes <shranzel@hotmail.com>
 */

/*
* draws two triangles using different colors for each vert(1st-red, 2nd-green,
* 3rd-blue). first tri drawn using glProvokingVertexEXT set to
* GL_FIRST_VERTEX_CONVENTION_EXT.
* Second tri using GL_LAST_VERTEX_CONVENTION_EXT.
*/

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 400;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{

	piglit_require_extension("GL_EXT_provoking_vertex");
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glShadeModel(GL_FLAT);
	glClearColor(0.2, 0.2, 0.2, 1.0);

}

enum piglit_result
piglit_display(void)
{
	static const float red[3] = {0.9, 0.0, 0.0};
	static const float blue[3] = {0.0, 0.0, 0.9};
	static const float green[3] = {0.0, 1.0, 0.0};
	GLboolean pass = GL_TRUE;

	glClear(GL_COLOR_BUFFER_BIT);
	glProvokingVertexEXT(GL_FIRST_VERTEX_CONVENTION_EXT);
        /* lower triangle: counter-clockwise */
	glBegin(GL_TRIANGLES);
		glColor3fv(red);
		glVertex3i(125, 85, 0);
		glColor3fv(green);
		glVertex3i(175, 85, 0);
		glColor3fv(blue);
		glVertex3i(150, 110, 0);
	glEnd();

        /* upper triangle: clockwise */
	glBegin(GL_TRIANGLES);
		glColor3fv(red);
		glVertex3i(125, 165, 0);
		glColor3fv(blue);
		glVertex3i(150, 190, 0);
		glColor3fv(green);
		glVertex3i(175, 165, 0);
	glEnd();

	glProvokingVertexEXT(GL_LAST_VERTEX_CONVENTION_EXT);
        /* lower triangle: counter-clockwise */
	glBegin(GL_TRIANGLES);
		glColor3fv(red);
		glVertex3i(200, 85, 0);
		glColor3fv(green);
		glVertex3i(250, 85, 0);
		glColor3fv(blue);
		glVertex3i(225, 110, 0);
	glEnd();

        /* upper triangle: clockwise */
	glBegin(GL_TRIANGLES);
		glColor3fv(green);
		glVertex3i(250, 165, 0);
		glColor3fv(red);
		glVertex3i(200, 165, 0);
		glColor3fv(blue);
		glVertex3i(225, 190, 0);
	glEnd();

	pass = pass && piglit_probe_pixel_rgb(150, 90, red);
	pass = pass && piglit_probe_pixel_rgb(150, 170, red);

	pass = pass && piglit_probe_pixel_rgb(225, 90, blue);
	pass = pass && piglit_probe_pixel_rgb(225, 170, blue);

	glFinish();
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
