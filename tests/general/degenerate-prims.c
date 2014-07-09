/*
 * Copyright © 2013 VMware, Inc.
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
 */


/**
 * Test drawing primitives with too few vertices.  In particular,
 * GL_QUADS and GL_QUAD_STRIP with 3 verts seems to regress every
 * once in a while in Mesa.
 *
 * Brian Paul
 * Feb 2013
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END


/**
 * Test a specific degenerate primitive.
 * The expected outcome is that nothing will be drawn.
 */
static bool
test_prim(GLenum prim, unsigned numVerts, const void *verts)
{
	static const float black[] = {0, 0, 0, 0};
	bool pass;

	glClear(GL_COLOR_BUFFER_BIT);

	glVertexPointer(2, GL_FLOAT, 0, verts);
	glEnable(GL_VERTEX_ARRAY);
	glDrawArrays(prim, 0, numVerts);

	/* Nothing should have been drawn / look for all black */
	pass = piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height, black);

	piglit_present_results();

	if (!pass) {
		piglit_report_subtest_result(PIGLIT_FAIL, "Primitive: %s",
					     piglit_get_prim_name(prim));
	}

	return pass;
}


enum piglit_result
piglit_display(void)
{
	static const float
		verts2[2][2] = { {-1, -1}, {1, 1} },
		verts3[3][2] = { {-1, -1}, {1, -1}, {0, 1} },
		verts4[4][2] = { {-1, -1}, {1, -1}, {1, 1}, {-1, 1} };
	static bool pass = true;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);

	glColor3f(1, 1, 1);

	pass = test_prim(GL_POINTS, 0, verts2) && pass;
	pass = test_prim(GL_LINES, 1, verts2) && pass;
	pass = test_prim(GL_LINE_STRIP, 1, verts2) && pass;
	pass = test_prim(GL_LINE_LOOP, 1, verts2) && pass;
	pass = test_prim(GL_TRIANGLES, 2, verts3) && pass;
	pass = test_prim(GL_TRIANGLE_STRIP, 2, verts3) && pass;
	pass = test_prim(GL_TRIANGLE_FAN, 2, verts3) && pass;
	pass = test_prim(GL_QUADS, 3, verts4) && pass;
	pass = test_prim(GL_QUAD_STRIP, 3, verts4) && pass;
	pass = test_prim(GL_POLYGON, 2, verts4) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	/* nop */
}
