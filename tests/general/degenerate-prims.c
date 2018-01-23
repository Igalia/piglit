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
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

struct test_data {
	GLenum prim;
	unsigned numVerts;
	const void *verts;
};

/**
 * Test a specific degenerate primitive.
 * The expected outcome is that nothing will be drawn.
 */
static enum piglit_result
test_prim(void *_data)
{
	struct test_data *data = _data;
	static const float black[] = {0, 0, 0, 0};
	bool pass;

	glClear(GL_COLOR_BUFFER_BIT);

	glVertexPointer(2, GL_FLOAT, 0, data->verts);
	glEnable(GL_VERTEX_ARRAY);
	glDrawArrays(data->prim, 0, data->numVerts);

	/* Nothing should have been drawn / look for all black */
	pass = piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height, black);

	piglit_present_results();

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
			             "Primitive: %s", piglit_get_prim_name(data->prim));

	return pass;
}


enum piglit_result
piglit_display(void)
{
	static const float
		verts2[2][2] = { {-1, -1}, {1, 1} },
		verts3[3][2] = { {-1, -1}, {1, -1}, {0, 1} },
		verts4[4][2] = { {-1, -1}, {1, -1}, {1, 1}, {-1, 1} };
	enum piglit_result result = PIGLIT_PASS;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);

	glColor3f(1, 1, 1);

	struct test_data data[] = {
		{ GL_POINTS, 0, verts2 },
		{ GL_LINES, 1, verts2 },
		{ GL_LINE_STRIP, 1, verts2 },
		{ GL_LINE_LOOP, 1, verts2 },
		{ GL_TRIANGLES, 2, verts3 },
		{ GL_TRIANGLE_STRIP, 2, verts3 },
		{ GL_TRIANGLE_FAN, 2, verts3 },
		{ GL_QUADS, 3, verts4 },
		{ GL_QUAD_STRIP, 3, verts4 },
		{ GL_POLYGON, 2, verts4 },
	};

	struct piglit_subtest tests[ARRAY_SIZE(data) + 1];
	for (int i = 0; i < ARRAY_SIZE(data); ++i) {
		tests[i].name = piglit_get_prim_name(data[i].prim);
		tests[i].option = "";
		tests[i].subtest_func = test_prim;
		tests[i].data = &data[i];
	}
	tests[ARRAY_SIZE(data)].name = NULL;

	result  = piglit_run_selected_subtests(tests, NULL, 0, result);

	return result;
}


void
piglit_init(int argc, char **argv)
{
	/* nop */
}
