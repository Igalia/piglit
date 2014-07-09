/* Copyright © 2011-2012 Intel Corporation
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

/** @file minmax-test.c
 *
 * Helpers for performing minimuTest for the minimum maximum values in section 6.2 "State Tables"
 * of the GL 3.0 spec.
 */

#include <inttypes.h>
#include <string.h>

#include "piglit-util-gl.h"
#include "minmax-test.h"

bool piglit_minmax_pass = true;

void piglit_print_minmax_header(void)
{
	printf("%-50s %8s %8s\n", "token", "minimum", "value");
}

static void
piglit_report_int(const char *name, GLint limit, GLint val, bool pass)
{
	if (pass) {
		printf("%-50s %8d %8d\n", name, limit, val);
	} else {
		fprintf(stderr, "%-50s %8d %8d (ERROR)\n",
			name, limit, val);
		piglit_minmax_pass = false;
	}
}

static void
piglit_report_uint(const char *name, GLuint limit, GLuint val, bool pass)
{

	printf("%-50s %8u %8u", name, limit, val);
	if (!pass) {
		printf(" (ERROR)");
		piglit_minmax_pass = false;
	}
	printf("\n");
}

static void
piglit_report_int64(const char *name, GLint64 limit, GLint64 val, bool pass)
{
	printf("%-50s %8"PRId64" %8"PRId64 , name, limit, val);
	if (!pass) {
		printf(" (ERROR)");
		piglit_minmax_pass = false;
	}
	printf("\n");
}

static void
piglit_report_uint64(const char *name, GLuint64 limit, GLuint64 val, bool pass)
{
	printf("%-50s %8"PRIu64" %8"PRIu64 , name, limit, val);
	if (!pass) {
		printf(" (ERROR)");
		piglit_minmax_pass = false;
	}
	printf("\n");
}

static void
piglit_report_float(const char *name, GLfloat limit, GLfloat val, bool pass)
{
	if (pass) {
		printf("%-50s %8.1f %8.1f\n", name, limit, val);
	} else {
		fprintf(stderr, "%-50s %8f %8f (ERROR)\n",
			name, limit, val);
		piglit_minmax_pass = false;
	}
}

#define SENTINEL  0xDEADBEEF

static void
piglit_test_int(GLenum token, GLint limit, bool max)
{
	const char *name = piglit_get_gl_enum_name(token);
	GLint val = SENTINEL;
	bool pass;

	glGetIntegerv(token, &val);

	pass = piglit_check_gl_error(GL_NO_ERROR);

	piglit_report_int(name, limit, val,
			  pass &&
			  val != (GLint)SENTINEL &&
			  ((max && val <= limit) ||
			   (!max && val >= limit)));
}

static void
piglit_test_int_v(GLenum token, GLuint index, GLint limit, bool max)
{
	char *name;
	GLint val = 9999;
	asprintf(&name, "%s[%d]", piglit_get_gl_enum_name(token), index);

	glGetIntegeri_v(token, index, &val);

	piglit_report_int(name, limit, val,
			  (max && val <= limit) ||
			  (!max && val >= limit));
}

static void
piglit_test_uint(GLenum token, GLuint limit, bool max)
{
	const char *name = piglit_get_gl_enum_name(token);
	GLuint val = SENTINEL;
	bool pass;

	glGetIntegerv(token, (GLint*) &val);

	pass = piglit_check_gl_error(GL_NO_ERROR);

	piglit_report_uint(name, limit, val,
			   pass &&
			   val != SENTINEL &&
			   ((max && val <= limit) ||
			    (!max && val >= limit)));
}

static void
piglit_test_int64(GLenum token, GLint64 limit, bool max)
{
	const char *name = piglit_get_gl_enum_name(token);
	GLint64 val = SENTINEL;
	bool pass;

	glGetInteger64v(token, &val);

	pass = piglit_check_gl_error(GL_NO_ERROR);

	piglit_report_int64(name, limit, val,
			    pass &&
			    val != SENTINEL &&
			    ((max && val <= limit) ||
			     (!max && val >= limit)));
}

static void
piglit_test_uint64(GLenum token, GLuint64 limit, bool max)
{
	const char *name = piglit_get_gl_enum_name(token);
	GLuint64 val = SENTINEL;
	bool pass;

	/* To obtain GLuint64 values, we must use glGetInteger64v.
	 * Justification is found in the GL_ARB_sync spec:
	 *
	 *   30) What is the type of the timeout interval?
	 *
	 *       RESOLVED: GLuint64. [...] Consequently the type of <timeout>
	 *       has been changed to 'GLuint64' and a corresponding
	 *       'GetInteger64v' query taking 'GLint64' added (by symmetry
	 *       with GetInteger, where unsigned quantities are queries with
	 *       a function taking a pointer to a signed integer - the pointer
	 *       conversion is harmless).
	 */

	glGetInteger64v(token, (GLint64*) &val);

	pass = piglit_check_gl_error(GL_NO_ERROR);

	piglit_report_uint64(name, limit, val,
			     pass &&
			     val != SENTINEL &&
			     ((max && val <= limit) ||
			      (!max && val >= limit)));
}

void piglit_test_min_int_v(GLenum token, GLuint index, GLint min)
{
	piglit_test_int_v(token, index, min, false);
}

void piglit_test_max_int_v(GLenum token, GLuint index, GLint max)
{
	piglit_test_int_v(token, index, max, true);
}

void piglit_test_min_int(GLenum token, GLint min)
{
	piglit_test_int(token, min, false);
}

void piglit_test_max_int(GLenum token, GLint max)
{
	piglit_test_int(token, max, true);
}

void piglit_test_min_uint(GLenum token, GLuint min)
{
	piglit_test_uint(token, min, false);
}

void piglit_test_max_uint(GLenum token, GLuint max)
{
	piglit_test_uint(token, max, true);
}

void piglit_test_min_int64(GLenum token, GLint64 min)
{
	piglit_test_int64(token, min, false);
}

void piglit_test_max_int64(GLenum token, GLint64 max)
{
	piglit_test_int64(token, max, true);
}

void piglit_test_min_uint64(GLenum token, GLuint64 min)
{
	piglit_test_uint64(token, min, false);
}

void piglit_test_max_uint64(GLenum token, GLuint64 max)
{
	piglit_test_uint64(token, max, true);
}

static void
piglit_test_float(GLenum token, GLfloat limit, bool max)
{
	const char *name = piglit_get_gl_enum_name(token);
	GLfloat val = SENTINEL;
	bool pass;

	glGetFloatv(token, &val);

	pass = piglit_check_gl_error(GL_NO_ERROR);

	piglit_report_float(name, limit, val,
			    pass &&
			    val != (GLfloat)SENTINEL &&
			    ((max && val <= limit) ||
			     (!max && val >= limit)));
}

void piglit_test_min_float(GLenum token, GLfloat min)
{
	piglit_test_float(token, min, false);
}

void piglit_test_max_float(GLenum token, GLfloat max)
{
	piglit_test_float(token, max, true);
}

/** Tests tha the range referneced by the token covers at least low-high. */
void piglit_test_range_float(GLenum token, GLfloat low, GLfloat high)
{
	const char *name = piglit_get_gl_enum_name(token);
	char *temp;
	GLfloat vals[2] = {SENTINEL, SENTINEL};
	bool pass;

	glGetFloatv(token, vals);

	pass = piglit_check_gl_error(GL_NO_ERROR);

	asprintf(&temp, "%s[0]", name);
	piglit_report_float(temp, low, vals[0], pass && vals[0] <= low);
	free(temp);

	asprintf(&temp, "%s[1]", name);
	piglit_report_float(temp, high, vals[1], pass && vals[1] >= high);
	free(temp);
}

void piglit_test_min_viewport_dimensions(void)
{
	int min_w, min_h;
	GLint dims[2] = {9999, 9999};

	if (piglit_get_gl_version() < 30) {
		/* FINISHME:
		 *
		 *     "The maximum viewport dimensions must be
		 *      greater than or equal to the larger of the
		 *      visible dimensions of the display being
		 *      rendered to (if a display exists), and the
		 *      largest renderbuffer image which can be
		 *      successfully created and attached to a
		 *      framebuffer object (see chapter 4).  INVALID
		 *      VALUE is generated if either w or h is
		 *      negative."
		 *
		 * We're only looking at RB limits here.
		 */
		int rb_size = 9999;
		glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &rb_size);

		min_w = rb_size;
		min_h = rb_size;
	} else {
		/* FINISHME:
		 *
		 *     "The maximum viewport dimensions must be
		 *      greater than or equal to the visible
		 *      dimensions of the display being rendered to."
		 *
		 * Surely the screen is at least 1024x768, right?
		 */
		min_w = 1024;
		min_h = 768;
	}

	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, dims);

	piglit_report_int("GL_MAX_VIEWPORT_DIMS[0]", min_w, dims[0],
			  dims[0] >= min_w);
	piglit_report_int("GL_MAX_VIEWPORT_DIMS[1]", min_h, dims[1],
			  dims[1] >= min_h);
}

void
piglit_test_oq_bits()
{
	GLint dims[2] = {9999, 9999};
	GLint minbits, oqbits = 9999;

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
		piglit_minmax_pass = false;
	}
}

void
piglit_test_tf_bits(GLenum target)
{
	GLint bits = 9999;
	const char *name;

	if (target == GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN)
		name = "GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN bits";
	else
		name = "GL_PRIMITIVES_GENERATED bits";

	/* From the GL 3.0 specification, page 329:
	 *
	 *     "If pname is QUERY_COUNTER_BITS, the
	 *      implementation-dependent number of query counter bits
	 *      may be zero, in which case the counter contains no
	 *      useful information.
	 *
	 *      For primitive queries (PRIMITIVES GENERATED and
	 *      TRANSFORM FEEDBACK PRIMITIVES WRITTEN) if the number
	 *      of bits is non-zero, the minimum number of bits
	 *      allowed is 32."
	 */

	glGetQueryiv(target, GL_QUERY_COUNTER_BITS, &bits);
	if (bits == 0 || bits >= 32) {
		printf("%-50s %8s %8d\n", name, "0 / 32", bits);
	} else {
		fprintf(stderr, "%-50s %8s %8d (ERROR)\n",
			name, "0 / 32", bits);
		piglit_minmax_pass = false;
	}
}
