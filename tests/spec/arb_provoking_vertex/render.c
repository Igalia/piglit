/**
 * Copyright 2015 VMware, Inc.
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
 * Test provoking vertex control with rendering.
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
PIGLIT_GL_TEST_CONFIG_END


static const float red[3] = {1, 0, 0},
	green[3] = {0, 1, 0},
	blue[3] = {0, 0, 1},
	yellow[3] = {1, 1, 0},
	black[3] = {0, 0, 0};


/* Do GL_QUADS, GL_QUAD_STRIP obey the provoking vertex control? */
static GLboolean quads_pv;


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_provoking_vertex");
}


static bool
test_mode(GLenum prim, GLenum pv_mode)
{
	bool pass = true;
	const float *expected1, *expected2;
	int x1 = piglit_width / 4;
	int x2 = piglit_width * 3 / 4;
	int y = piglit_height / 2;
	int dy;
	int num_black = 0;
	float dummy[4];

	glClear(GL_COLOR_BUFFER_BIT);

	glProvokingVertex(pv_mode);

	switch (prim) {
	case GL_LINES:
		glBegin(GL_LINES);
		/* first line */
		glColor3fv(red);
		glVertex2f(-1, 0);
		glColor3fv(green);
		glVertex2f( -0.1, 0);
		/* second line */
		glColor3fv(blue);
		glVertex2f(0.1, 0);
		glColor3fv(yellow);
		glVertex2f(1, 0);
		glEnd();
		if (pv_mode == GL_FIRST_VERTEX_CONVENTION) {
			expected1 = red;
			expected2 = blue;
		}
		else {
			expected1 = green;
			expected2 = yellow;
		}
		break;

	case GL_LINE_STRIP:
		glBegin(GL_LINE_STRIP);
		glColor3fv(red);
		glVertex2f(-1, 0);
		glColor3fv(green);
		glVertex2f(0, 0);
		glColor3fv(blue);
		glVertex2f(1, 0);
		glEnd();
		if (pv_mode == GL_FIRST_VERTEX_CONVENTION) {
			expected1 = red;
			expected2 = green;
		}
		else {
			expected1 = green;
			expected2 = blue;
		}
		break;

	case GL_LINE_LOOP:
		glBegin(GL_LINE_STRIP);
		glColor3fv(red);
		glVertex2f(-1, 0);
		glColor3fv(green);
		glVertex2f(0, 0);
		glColor3fv(blue);
		glVertex2f(1, 0);
		glColor3fv(yellow);
		glVertex2f(0, 1);
		glEnd();
		if (pv_mode == GL_FIRST_VERTEX_CONVENTION) {
			expected1 = red;
			expected2 = green;
		}
		else {
			expected1 = green;
			expected2 = blue;
		}
		break;

	case GL_TRIANGLES:
		glBegin(GL_TRIANGLES);
		/* first tri */
		glColor3fv(red);
		glVertex2f(-1, -1);
		glColor3fv(green);
		glVertex2f( 0, -1);
		glColor3fv(blue);
		glVertex2f(-0.5, 1);
		/* second tri */
		glColor3fv(green);
		glVertex2f(0, -1);
		glColor3fv(blue);
		glVertex2f( 1, -1);
		glColor3fv(red);
		glVertex2f(0.5, 1);
		glEnd();
		if (pv_mode == GL_FIRST_VERTEX_CONVENTION) {
			expected1 = red;
			expected2 = green;
		}
		else {
			expected1 = blue;
			expected2 = red;
		}
		break;

	case GL_TRIANGLE_STRIP:
		glBegin(GL_TRIANGLE_STRIP);
		/* first tri */
		glColor3fv(red);
		glVertex2f(-1, -1);
		glColor3fv(green);
		glVertex2f(-0.5, 1);
		glColor3fv(blue);
		glVertex2f(0.5, -1);
		glColor3fv(yellow);
		glVertex2f(1, 1);
		glEnd();
		if (pv_mode == GL_FIRST_VERTEX_CONVENTION) {
			expected1 = red;
			expected2 = green;
		}
		else {
			expected1 = blue;
			expected2 = yellow;
		}
		break;

	case GL_TRIANGLE_FAN:
		glBegin(GL_TRIANGLE_FAN);
		glColor3fv(red);
		glVertex2f(1, -1);
		glColor3fv(green);
		glVertex2f(-1, -1);
		glColor3fv(blue);
		glVertex2f(-1, 1);
		glColor3fv(yellow);
		glVertex2f(1, 1);
		glEnd();
		if (pv_mode == GL_FIRST_VERTEX_CONVENTION) {
			expected1 = green;
			expected2 = blue;
		}
		else {
			expected1 = blue;
			expected2 = yellow;
		}
		break;

        case GL_QUADS:
		glBegin(GL_QUADS);
		/* first quad */
		glColor3fv(red);
		glVertex2f(-1, -1);
		glColor3fv(green);
		glVertex2f(-1, 1);
		glColor3fv(blue);
		glVertex2f(-0.1, 1);
		glColor3fv(yellow);
		glVertex2f(-0.1, -1);
		/* second quad */
		glColor3fv(green);
		glVertex2f(0.1, -1);
		glColor3fv(blue);
		glVertex2f(0.1, 1);
		glColor3fv(yellow);
		glVertex2f(1, 1);
		glColor3fv(red);
		glVertex2f(1, -1);
		glEnd();
		if (quads_pv && pv_mode == GL_FIRST_VERTEX_CONVENTION) {
			expected1 = red;
			expected2 = green;
		}
		else {
			expected1 = yellow;
			expected2 = red;
		}
		break;

        case GL_QUAD_STRIP:
		glBegin(GL_QUAD_STRIP);
		glColor3fv(red);
		glVertex2f(-1, -1);
		glColor3fv(green);
		glVertex2f(-1, 1);
		glColor3fv(blue);
		glVertex2f(0, -1);
		glColor3fv(yellow);
		glVertex2f(0, 1);
		glColor3fv(green);
		glVertex2f(1, -1);
		glColor3fv(red);
		glVertex2f(1, 1);
		glEnd();
		if (quads_pv && pv_mode == GL_FIRST_VERTEX_CONVENTION) {
			expected1 = red;
			expected2 = blue;
		}
		else {
			expected1 = yellow;
			expected2 = red;
		}
		break;

	case GL_POLYGON:
		glBegin(GL_POLYGON);
		glColor3fv(red);
		glVertex2f(1, -1);
		glColor3fv(green);
		glVertex2f(-1, -1);
		glColor3fv(blue);
		glVertex2f(-1, 1);
		glColor3fv(yellow);
		glVertex2f(1, 1);
		glEnd();
		expected1 = red;
		expected2 = red;
		break;

	default:
		assert(!"Bad prim mode");
		return false;
	}

	/* try probing 3 scan lines to make sure we hit GL_LINES, etc. */
	for (dy = -1; dy <= 1 && pass; dy++) {
		if (piglit_probe_pixel_rgb_silent(x1, y+dy, black, dummy)) {
			/* try next Y pos */
			num_black++;
			continue;
		}
		if (!piglit_probe_pixel_rgb(x1, y+dy, expected1)) {
			pass = false;
		}
		if (!piglit_probe_pixel_rgb(x2, y+dy, expected2)) {
			pass = false;
		}
	}

	if (num_black == 3) {
		/* nothing drawn */
		pass = false;
	}

	if (!pass) {
		printf("Failure for %s, %s\n",
		       piglit_get_prim_name(prim),
		       piglit_get_gl_enum_name(pv_mode));
	}

	piglit_present_results();

	return pass;
}


enum piglit_result
piglit_display(void)
{
	static const GLenum modes[] = {
		GL_LINES,
		GL_LINE_STRIP,
		GL_LINE_LOOP,
		GL_TRIANGLES,
		GL_TRIANGLE_STRIP,
		GL_TRIANGLE_FAN,
		GL_QUADS,
		GL_QUAD_STRIP,
		GL_POLYGON

	};
	int i;
	bool pass = true;

	glViewport(0, 0, piglit_width, piglit_height);
	glShadeModel(GL_FLAT);

	glGetBooleanv(GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION, &quads_pv);
	printf("GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION = %u\n", quads_pv);

	for (i = 0; i < ARRAY_SIZE(modes); i++) {
		pass = test_mode(modes[i], GL_FIRST_VERTEX_CONVENTION) && pass;
		pass = test_mode(modes[i], GL_LAST_VERTEX_CONVENTION) && pass;
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);

	return PIGLIT_FAIL;
}
