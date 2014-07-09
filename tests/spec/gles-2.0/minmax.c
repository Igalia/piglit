/* Copyright © 2013 Linaro
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

#include "piglit-util-gl.h"
#include "minmax-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 20;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	/* Taken from es_full_spec_2.0.25, Chapture 6.2
	 * If the value's type is listed as Z in a spec table, then consider
	 * its type to be a signed int (that is, GLint). If the
	 * value's type is listed as Z^+, then consider its type to be
	 * unsigned (that is, GLuint).
	 */

	piglit_print_minmax_header();

	/* Table 6.18 */
	piglit_test_min_uint(GL_SUBPIXEL_BITS, 4);
	piglit_test_min_uint(GL_MAX_TEXTURE_SIZE, 64);
	piglit_test_min_uint(GL_MAX_CUBE_MAP_TEXTURE_SIZE, 16);
	piglit_test_min_viewport_dimensions();
	piglit_test_range_float(GL_ALIASED_POINT_SIZE_RANGE, 1, 1);
	piglit_test_range_float(GL_ALIASED_LINE_WIDTH_RANGE, 1, 1);
	piglit_test_min_uint(GL_SAMPLE_BUFFERS, 0);
	piglit_test_min_uint(GL_SAMPLES, 0);
	piglit_test_min_int(GL_NUM_COMPRESSED_TEXTURE_FORMATS, 0);

	/* Table 6.19 */
	piglit_test_min_int(GL_NUM_SHADER_BINARY_FORMATS, 0);

	/* Table 6.20 */
	piglit_test_min_uint(GL_MAX_VERTEX_ATTRIBS, 8);
	piglit_test_min_uint(GL_MAX_VERTEX_UNIFORM_VECTORS, 128);
	piglit_test_min_uint(GL_MAX_VARYING_VECTORS, 8);
	piglit_test_min_uint(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, 8);
	piglit_test_min_uint(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, 0);
	piglit_test_min_uint(GL_MAX_TEXTURE_IMAGE_UNITS, 8);
	piglit_test_min_uint(GL_MAX_FRAGMENT_UNIFORM_VECTORS, 16);
	piglit_test_min_uint(GL_MAX_RENDERBUFFER_SIZE, 1);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(piglit_minmax_pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
