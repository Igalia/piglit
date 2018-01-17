/*
 * Copyright (C) 2018 VMware, Inc.
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
 * Test glMaterial calls in a display list.
 */

#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END


static const GLfloat red[] = {1.0, 0.0, 0.0, 1.0};
static const GLfloat green[] = {0.0, 1.0, 0.0, 1.0};
static const GLfloat black[] = {0.0, 0.0, 0.0, 1.0};
static const GLfloat white[] = {1.0, 1.0, 1.0, 1.0};


/**
 * Build display list to draw two quads with a triangle strip
 * using glMaterial calls to set vertex colors.
 * \param set_all  If true, set the material attribs for all vertices.
 *                 Otherwise, just set the material attribs for two
 *                 provoking vertices.
 *
 * Note: the set_all parameter controls whether Mesa hits the "loopback" code.
 */
static GLuint
make_list(GLenum mat, bool set_all)
{
	GLuint list;

	list = glGenLists(1);
	glNewList(list, GL_COMPILE);

	/*
	 * Draw tri strip drawing two quads - left=red, right=green.
	 */
	glShadeModel(GL_FLAT);
	glBegin(GL_TRIANGLE_STRIP);
	glNormal3f(0, 0, 1);

	/* v0 */
	if (set_all)
		glMaterialfv(GL_FRONT_AND_BACK, mat, red);
	glVertex2f(-1, -1);

	/* v1 */
	if (set_all)
		glMaterialfv(GL_FRONT_AND_BACK, mat, red);
	glVertex2f(-1, 1);

	/* v2 */
	glMaterialfv(GL_FRONT_AND_BACK, mat, red);
	glVertex2f( 0, -1);

	/* v3 */
	if (set_all)
		glMaterialfv(GL_FRONT_AND_BACK, mat, red);
	glVertex2f( 0, 1);

	/* v4 */
	glMaterialfv(GL_FRONT_AND_BACK, mat, green);
	glVertex2f( 1, -1);

	/* v5 */
	if (set_all)
		glMaterialfv(GL_FRONT_AND_BACK, mat, green);
	glVertex2f( 1, 1);

	glEnd();

	glEndList();

	return list;
}


static bool
test_material(GLenum mat, bool set_all)
{
	bool pass = true;
	GLint w = piglit_width;
	GLint h = piglit_height;

	glClear(GL_COLOR_BUFFER_BIT);

	/* init all material coefficients to black */
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, black);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);

	GLuint list = make_list(mat, set_all);
	glCallList(list);
	glDeleteLists(list, 1);

	pass = piglit_probe_pixel_rgb(w/2-2, h/2, red) && pass;
	pass = piglit_probe_pixel_rgb(w/2+2, h/2, green) && pass;

	piglit_present_results();

	if (!pass) {
		printf("Failed testing %s material (set_all = %u)\n",
		       piglit_get_gl_enum_name(mat), set_all);
	}

	return pass;
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;

	pass = test_material(GL_AMBIENT, false) && pass;
	pass = test_material(GL_DIFFUSE, false) && pass;
	pass = test_material(GL_SPECULAR, false) && pass;
	pass = test_material(GL_EMISSION, false) && pass;

	pass = test_material(GL_AMBIENT, true) && pass;
	pass = test_material(GL_DIFFUSE, true) && pass;
	pass = test_material(GL_SPECULAR, true) && pass;
	pass = test_material(GL_EMISSION, true) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, white);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);
}
