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
 * of the GL 3.0 spec.
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
	piglit_require_gl_version(30);

	piglit_print_minmax_header();

	/* These should be in the section with "Minimum Value" but
	 * appear in the section with "Initial Value".
	 */
	piglit_test_min_int(GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS, 64);
	piglit_test_min_int(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS, 4);
	piglit_test_min_int(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS, 4);

	piglit_test_min_int(GL_MAX_LIGHTS, 8);
	piglit_test_min_int(GL_MAX_CLIP_PLANES, 6);
	if (piglit_is_extension_supported("GL_ARB_imaging"))
		piglit_test_min_int(GL_MAX_COLOR_MATRIX_STACK_DEPTH, 2);
	piglit_test_min_int(GL_MAX_MODELVIEW_STACK_DEPTH, 32);
	piglit_test_min_int(GL_MAX_PROJECTION_STACK_DEPTH, 2);
	piglit_test_min_int(GL_MAX_TEXTURE_STACK_DEPTH, 2);
	piglit_test_min_int(GL_SUBPIXEL_BITS, 4);
	piglit_test_min_int(GL_MAX_3D_TEXTURE_SIZE, 256);
	piglit_test_min_int(GL_MAX_TEXTURE_SIZE, 1024);
	piglit_test_min_float(GL_MAX_TEXTURE_LOD_BIAS, 2.0);
	piglit_test_min_int(GL_MAX_ARRAY_TEXTURE_LAYERS, 256);
	piglit_test_min_int(GL_MAX_CUBE_MAP_TEXTURE_SIZE, 1024);
	piglit_test_min_int(GL_MAX_RENDERBUFFER_SIZE, 1024);
	piglit_test_min_int(GL_MAX_PIXEL_MAP_TABLE, 32);
	piglit_test_min_int(GL_MAX_NAME_STACK_DEPTH, 64);
	piglit_test_min_int(GL_MAX_LIST_NESTING, 64);
	piglit_test_min_int(GL_MAX_EVAL_ORDER, 8);

	piglit_test_min_viewport_dimensions();

	piglit_test_min_int(GL_MAX_ATTRIB_STACK_DEPTH, 16);
	piglit_test_min_int(GL_MAX_CLIENT_ATTRIB_STACK_DEPTH, 16);

	piglit_test_range_float(GL_ALIASED_POINT_SIZE_RANGE, 1, 1);
	piglit_test_range_float(GL_SMOOTH_POINT_SIZE_RANGE, 1, 1);
	piglit_test_range_float(GL_ALIASED_LINE_WIDTH_RANGE, 1, 1);
	piglit_test_range_float(GL_SMOOTH_LINE_WIDTH_RANGE, 1, 1);

	piglit_test_tf_bits(GL_PRIMITIVES_GENERATED);
	piglit_test_tf_bits(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	piglit_test_oq_bits();

	if (piglit_is_extension_supported("GL_ARB_imaging")) {
		/* FINISHME: GL_MAX_CONVOLUTION_WIDTH */
		/* FINISHME: GL_MAX_CONVOLUTION_HEIGHT */
	}

	piglit_test_min_int(GL_MAX_TEXTURE_UNITS, 2);
	piglit_test_min_int(GL_MAX_VERTEX_ATTRIBS, 16);
	piglit_test_min_int(GL_MAX_VERTEX_UNIFORM_COMPONENTS, 1024);
	piglit_test_min_int(GL_MAX_VARYING_COMPONENTS, 64);
	piglit_test_min_int(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, 16);
	piglit_test_min_int(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, 16);
	piglit_test_min_int(GL_MAX_TEXTURE_IMAGE_UNITS, 16);
	piglit_test_min_int(GL_MAX_TEXTURE_COORDS, 8);
	piglit_test_min_int(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, 1024);

	piglit_test_max_int(GL_MIN_PROGRAM_TEXEL_OFFSET, -8);
	piglit_test_min_int(GL_MAX_PROGRAM_TEXEL_OFFSET, 7);

	piglit_test_min_int(GL_AUX_BUFFERS, 0);
	piglit_test_min_int(GL_MAX_DRAW_BUFFERS, 8);

	piglit_test_min_int(GL_SAMPLE_BUFFERS, 0);
	piglit_test_min_int(GL_SAMPLES, 0);
	piglit_test_min_int(GL_MAX_COLOR_ATTACHMENTS, 8);
	piglit_test_min_int(GL_MAX_SAMPLES, 4);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(piglit_minmax_pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
