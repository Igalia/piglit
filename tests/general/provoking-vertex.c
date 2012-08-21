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

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_MAIN(
    400 /*window_width*/,
    300 /*window_height*/,
    GLUT_RGB | GLUT_DOUBLE)

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
	float red[3] = {1.0, 0.0, 0.0};
	float blue[3] = {0.0, 0.0, 1.0};
	GLboolean pass = GL_TRUE;

	glClear(GL_COLOR_BUFFER_BIT);
	glProvokingVertexEXT(GL_FIRST_VERTEX_CONVENTION_EXT);
	glBegin(GL_TRIANGLES);
		glColor3f(1.0, 0.0, 0.0);
		glVertex3i(125, 125, 0);
		glColor3f(0.0, 1.0, 0.0);
		glVertex3i(175, 125, 0);
		glColor3f(0.0, 0.0, 1.0);
		glVertex3i(150, 150, 0);
	glEnd();

	glProvokingVertexEXT(GL_LAST_VERTEX_CONVENTION_EXT);
	glBegin(GL_TRIANGLES);
		glColor3f(1.0, 0.0, 0.0);
		glVertex3i(200, 125, 0);
		glColor3f(0.0, 1.0, 0.0);
		glVertex3i(250, 125, 0);
		glColor3f(0.0, 0.0, 1.0);
		glVertex3i(225, 150, 0);
	glEnd();

	pass = pass && piglit_probe_pixel_rgb(150, 130, red);
	pass = pass && piglit_probe_pixel_rgb(225, 130, blue);

	glFinish();
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
