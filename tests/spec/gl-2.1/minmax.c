/* Copyright © 2011 Intel Corporation
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

/** @file minmax.c
 *
 * Test for the minimum maximum values in section 6.2 "State Tables"
 * of the GL 2.1 spec.
 */

#include "piglit-util.h"

int piglit_width = 32;
int piglit_height = 32;
int piglit_window_mode = GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA;

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

static bool pass = true;

static void
min_test_i(GLenum token, GLint min, const char *name)
{
	GLint vals[2];

	glGetIntegerv(token, vals);

	if (vals[0] < min) {
		fprintf(stderr, "%-50s %8d %8d (ERROR)\n",
			name, min, vals[0]);
		pass = false;
	} else {
		printf("%-50s %8d %8d\n", name, min, vals[0]);
	}
}

static void
min_test_f(GLenum token, GLfloat min, const char *name)
{
	GLfloat val;

	glGetFloatv(token, &val);

	if (val < min) {
		fprintf(stderr, "%-50s %8f %8f (ERROR)\n",
			name, min, val);
		pass = false;
	} else {
		printf("%-50s %8f %8f\n", name, min, val);
	}
}

/* All the size requirements happen to only require a range covering
 * [1.0, 1.0].
 */
static void
size_range_test(GLenum token, const char *name)
{
	GLfloat vals[2];

	glGetFloatv(token, vals);

	if (vals[0] > 1.0 || vals[1] < 1.0) {
		fprintf(stderr, "%-50s %8s  %.1f-%.1f (ERROR)\n",
			name, "1-1", vals[0], vals[1]);
		pass = false;
	} else {
		printf("%-50s %8s  %.1f-%.1f\n",
		       name, "1-1", vals[0], vals[1]);
	}
}

static void
test_oq_bits()
{
	GLint dims[2];
	GLint minbits, oqbits;

	/* From the GL 2.1 specification:
	 *
	 *     "The number of query counter bits may be zero, in which
	 *      case the counter contains no useful
	 *      information. Otherwise, the minimum number of bits
	 *      allowed is a function of the implementation’s maximum
	 *      viewport dimensions (MAX_VIEWPORT_DIMS). In this case,
	 *      the counter must be able to represent at least two
	 *      overdraws for every pixel in the viewport The formula
	 *      to compute the allowable minimum value (where n is the
	 *      minimum number of bits) is:
	 *
	 *      n = min{32, log2(maxViewportWidth ∗ maxViewportHeight * 2}"
	 */

	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, dims);
	minbits = log2((float)dims[0] * dims[1] * 2);
	if (minbits > 32)
		minbits = 32;

	glGetQueryiv(GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS, &oqbits);
	if (oqbits == 0 || oqbits >= minbits) {
		printf("%-50s   0 / %2d %8d\n",
		       "GL_QUERY_COUNTER_BITS(GL_SAMPLES_PASSED)",
		       minbits, oqbits);
	} else {
		fprintf(stderr,
			"%-50s   0 / %2d %8d\n",
			"GL_QUERY_COUNTER_BITS(GL_SAMPLES_PASSED)",
			minbits, oqbits);
		pass = false;
	}
}

#define MAX_INTEGER_TEST(token, max) max_test_i(token, max, #token)
#define MIN_INTEGER_TEST(token, min) min_test_i(token, min, #token)
#define MIN_FLOAT_TEST(token, min) min_test_f(token, min, #token)
#define TEST_SIZE_RANGE(token) size_range_test(token, #token)

void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(21);

	printf("%-50s %8s %8s\n", "token", "minimum", "value");

	MIN_INTEGER_TEST(GL_MAX_LIGHTS, 8);
	MIN_INTEGER_TEST(GL_MAX_CLIP_PLANES, 6);
	if (piglit_is_extension_supported("GL_ARB_imaging"))
		MIN_INTEGER_TEST(GL_MAX_COLOR_MATRIX_STACK_DEPTH, 2);
	MIN_INTEGER_TEST(GL_MAX_MODELVIEW_STACK_DEPTH, 32);
	MIN_INTEGER_TEST(GL_MAX_PROJECTION_STACK_DEPTH, 2);
	MIN_INTEGER_TEST(GL_MAX_TEXTURE_STACK_DEPTH, 2);
	MIN_INTEGER_TEST(GL_SUBPIXEL_BITS, 4);

	MIN_INTEGER_TEST(GL_MAX_3D_TEXTURE_SIZE, 16);
	MIN_INTEGER_TEST(GL_MAX_TEXTURE_SIZE, 64);
	MIN_FLOAT_TEST(GL_MAX_TEXTURE_LOD_BIAS, 2.0);
	MIN_INTEGER_TEST(GL_MAX_CUBE_MAP_TEXTURE_SIZE, 16);
	MIN_INTEGER_TEST(GL_MAX_PIXEL_MAP_TABLE, 32);
	MIN_INTEGER_TEST(GL_MAX_NAME_STACK_DEPTH, 64);
	MIN_INTEGER_TEST(GL_MAX_LIST_NESTING, 64);
	MIN_INTEGER_TEST(GL_MAX_EVAL_ORDER, 8);

	/* FINISHME:
	 *
	 *     "The maximum viewport dimensions must be greater than
	 *      or equal to the visible dimensions of the display
	 *      being rendered to."
	 *
	 * Surely the screen is at least 1024x768, right?
	 */
	MIN_INTEGER_TEST(GL_MAX_VIEWPORT_DIMS, 1024);

	MIN_INTEGER_TEST(GL_MAX_ATTRIB_STACK_DEPTH, 16);
	MIN_INTEGER_TEST(GL_MAX_CLIENT_ATTRIB_STACK_DEPTH, 16);

	TEST_SIZE_RANGE(GL_ALIASED_POINT_SIZE_RANGE);
	TEST_SIZE_RANGE(GL_SMOOTH_POINT_SIZE_RANGE);
	TEST_SIZE_RANGE(GL_ALIASED_LINE_WIDTH_RANGE);
	TEST_SIZE_RANGE(GL_SMOOTH_LINE_WIDTH_RANGE);

	test_oq_bits();

	MIN_INTEGER_TEST(GL_AUX_BUFFERS, 0);

	if (piglit_is_extension_supported("GL_ARB_imaging")) {
		/* FINISHME: GL_MAX_CONVOLUTION_WIDTH */
		/* FINISHME: GL_MAX_CONVOLUTION_HEIGHT */
	}

	MIN_INTEGER_TEST(GL_SAMPLE_BUFFERS, 0);
	MIN_INTEGER_TEST(GL_SAMPLES, 0);

	MIN_INTEGER_TEST(GL_MAX_TEXTURE_UNITS, 2);
	MIN_INTEGER_TEST(GL_MAX_VERTEX_ATTRIBS, 16);
	MIN_INTEGER_TEST(GL_MAX_VERTEX_UNIFORM_COMPONENTS, 512);
	MIN_INTEGER_TEST(GL_MAX_VARYING_COMPONENTS, 32);
	MIN_INTEGER_TEST(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, 2);
	MIN_INTEGER_TEST(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, 0);
	MIN_INTEGER_TEST(GL_MAX_TEXTURE_IMAGE_UNITS, 2);
	MIN_INTEGER_TEST(GL_MAX_TEXTURE_COORDS, 2);
	MIN_INTEGER_TEST(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, 64);
	MIN_INTEGER_TEST(GL_MAX_DRAW_BUFFERS, 1);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
