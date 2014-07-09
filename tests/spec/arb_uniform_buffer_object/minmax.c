/* Copyright © 2012 Intel Corporation
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
 * of the GL 3.1 spec.
 */

#include "piglit-util-gl.h"
#include "minmax-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	int vuniforms = 0, vblocks = 0;
	int guniforms = 0, gblocks = 0;
	int funiforms = 0, fblocks = 0;
	int blocksize = 0;
	bool gs = (piglit_get_gl_version() >= 32) || piglit_is_extension_supported("GL_ARB_geometry_shader4");

	piglit_require_extension("GL_ARB_uniform_buffer_object");

	piglit_print_minmax_header();

	piglit_test_min_int(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, 1024);

	piglit_test_min_int(GL_MAX_VERTEX_UNIFORM_BLOCKS, 12);
	piglit_test_min_int(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, 12);
	if (gs)
		piglit_test_min_int(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, 12);

	piglit_test_min_int(GL_MAX_COMBINED_UNIFORM_BLOCKS, gs ? 36 : 24);
	piglit_test_min_int(GL_MAX_UNIFORM_BUFFER_BINDINGS, gs ? 36 : 24);
	piglit_test_min_int(GL_MAX_UNIFORM_BLOCK_SIZE, 16384);

	/* Minimum value for OpenGL 3.1 is
	 * (MAX_<stage>_UNIFORM_BLOCKS * MAX_UNIFORM_BLOCK_SIZE) +
	 * MAX_<stage>_UNIFORM_COMPONENTS. Minimum value prior to
	 * OpenGL 3.1 is MAX_<stage>_UNIFORM_COMPONENTS.
	 */
	if (piglit_get_gl_version() >= 31) {
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &vblocks);
		glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &fblocks);
	}
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &vuniforms);
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &funiforms);
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &blocksize);
	piglit_test_min_int(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS,
			 vblocks * blocksize / 4 + vuniforms);
	piglit_test_min_int(GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS,
			 fblocks * blocksize / 4 + funiforms);
	if (gs) {
		if (piglit_get_gl_version() >= 31) {
			glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &gblocks);
		}
		glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS, &guniforms);

		piglit_test_min_int(GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS,
				    gblocks * blocksize / 4 + guniforms);
	}

	piglit_test_min_int(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, 1);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(piglit_minmax_pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
