/*
 * Copyright © 2012 Intel Corporation
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
 * \file interpolation.cpp
 *
 * Verify that the implementation interpolates varyings correctly when
 * multisampling is in use, particularly that it properly implements
 * the "centroid" keyword.
 *
 * From the GLSL 1.30 spec, section 4.3.7 (Interpolation):
 *
 *     "This paragraph only applies if interpolation is being done: If
 *     single-sampling, the value is interpolated to the pixel's
 *     center, and the centroid qualifier, if present, is ignored. If
 *     multi-sampling and the variable is not qualified with centroid,
 *     then the value must be interpolated to the pixel's center, or
 *     anywhere within the pixel, or to one of the pixel's samples. If
 *     multi-sampling and the variable is qualified with centroid,
 *     then the value must be interpolated to a point that lies in
 *     both the pixel and in the primitive being rendered, or to one
 *     of the pixel's samples that falls within the primitive. Due to
 *     the less regular location of centroids, their derivatives may
 *     be less accurate than non-centroid interpolated variables."
 *
 *
 * This test accepts two command-line parameters, a value for
 * num_samples, and a test type.  The test types are as follows:
 *
 * - non-centroid-disabled: verify that non-centroid interpolation
 *   produces the same results when applied to a non-multisampled
 *   buffer and a multisampled buffer with GL_MULTISAMPLE disabled.
 *   This effectively verifies that non-centroid varyings are
 *   interpolated at the pixel center when single-sampling.  The test
 *   uses a fragment shader that sets the red, green, and blue
 *   channels to the barycentric coordinates within each triangle.
 *
 * - centroid-disabled: verify that centroid interpolation produces
 *   the same results as non-centroid interpolation when applied to a
 *   multisampled buffer with GL_MULTISAMPLE disabled.  This
 *   effectively verifies that centroid varyings are interpolated at
 *   the pixel center when single-sampling.  This test may also be run
 *   with num_samples=0 to verify that centroid varyings work properly
 *   in non-multisampled buffers.  The test uses a fragment shader
 *   that sets the red, green, and blue channels to the barycentric
 *   coordinates within each triangle.
 *
 * - centroid-edges: verify that centroid interpolation occurs at
 *   points that lie within the extents of the triangle, even for
 *   pixels on triangle edges, where the center of the pixel might lie
 *   outside the extents of the triangle.  The test uses a fragment
 *   shader that sets the blue channel to 1.0 (so that the triangles
 *   can be seen) and the red and green channels to 1.0 if any of the
 *   centroid-interpolated barycentric coordinates is outside the
 *   range [0, 1]; except when num_samples == 0, in which case
 *   it behaves like centroid-disabled.
 *
 * - non-centroid-deriv: verify that the numeric derivative of a
 *   varying using non-centroid interpolation is correct, even at
 *   triangle edges.  This ensures that the implementation properly
 *   handles a subtle corner case: since numeric derivatives are
 *   usually computed using finite differences between adjacent
 *   pixels, it's possible that the value of a varying at a completely
 *   uncovered pixel might be used.  In effect, this tests that the
 *   values of varyings are correct on completely uncovered pixels, if
 *   those values are needed for derivatives.  This test may also be
 *   run with num_samples=0 to verify that non-centroid varyings
 *   exhibit proper derivative behaviour in non-multisampled buffers.
 *   The test uses a fragment shader that sets red=dFdx(interpolated x
 *   coordinate), green=dFdy(interpolated y coordinate), and blue=1.0,
 *   with appropriate scaling applied to the red and green outputs so
 *   that the expected output is red=0.5 and green=0.5.
 *
 * - non-centroid-deriv-disabled: Like non-centroid-deriv, but the
 *   test is done with GL_MULTISAMPLE disabled.
 *
 * - centroid-deriv: verify that the numeric derivative of a vaying
 *   using centroid interpolation is within reasonable bounds.  Any
 *   derivative value between 0 and twice the expected derivative is
 *   considered passing, since this is the expected error bound for a
 *   typical implementation (where derivative is computed via a finite
 *   difference of adjacent pixels, and sample points are within the
 *   pixel boundary). As with non-centroid-deriv, this test may also
 *   be run with num_samples=0 to verify that centroid varyings
 *   exhibit proper derivative behaviour in non-multisampled buffers;
 *   in this case, the error bounds are as in non-centroid-deriv,
 *   since centroid-related derivative errors are not expected.  When
 *   num_samples=0, the fragment shader generates outputs as in
 *   non-centroid-deriv.  Otherwise it sets the blue channel to 1.0
 *   (so that the triangles can be seen) and the red nd green channels
 *   to 1.0 if either derivative is out of tolerance.
 *
 * - centroid-deriv-disabled: like centroid-deriv, but the test is
 *   done with GL_MULTISAMPLE disabled, and the error bounds are as in
 *   non-centroid-deriv.  The fragment shader generates outputs as in
 *   non-centroid-deriv.
 *
 * All test types draw an array of small triangles at various
 * rotations, so that pixels are covered in a wide variety of
 * patterns.  In each case the rendered result is displayed on the
 * left, and the expected result is displayed on the right for
 * comparison.
 */

#include "piglit-test-pattern.h"
#include "piglit-fbo.h"
using namespace piglit_util_fbo;
using namespace piglit_util_test_pattern;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 512;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

namespace {

const int pattern_width = 256; const int pattern_height = 256;

Fbo singlesampled_fbo;
Fbo multisampled_fbo;


/**
 * Test pattern that we'll use to draw the test image.
 */
TestPattern *test_pattern;


/**
 * Test pattern that we'll use to draw the reference image.
 */
TestPattern *ref_pattern;


/**
 * If true, we will disable GL_MULTISAMPLE while drawing the test
 * image, and we will draw the reference image into a single-sample
 * buffer.
 */
bool disable_msaa_during_test_image = false;


/**
 * Fragment shader source that sets the red, green, and blue channels
 * to the non-centroid-interpolated barycentric coordinates within
 * each triangle.
 */
const char *frag_non_centroid_barycentric =
	"#version 120\n"
	"varying vec3 barycentric_coords;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = vec4(barycentric_coords, 1.0);\n"
	"}\n";


/**
 * Fragment shader source that sets the red, green, and blue channels
 * to the centroid-interpolated barycentric coordinates within each
 * triangle.
 */
const char *frag_centroid_barycentric =
	"#version 120\n"
	"centroid varying vec3 barycentric_coords_centroid;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = vec4(barycentric_coords_centroid, 1.0);\n"
	"}\n";


/**
 * Fragment shader source that sets the blue channel to 1.0, and the
 * red and green channels to 1.0 if any of the centroid-interpolated
 * barycentric coordinates is outside the range [0, 1].
 */
const char *frag_centroid_range_check =
	"#version 120\n"
	"centroid varying vec3 barycentric_coords_centroid;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  if (any(greaterThan(barycentric_coords_centroid, vec3(1.0))) ||\n"
	"      any(lessThan(barycentric_coords_centroid, vec3(0.0))))\n"
	"    gl_FragColor = vec4(1.0);\n"
	"  else\n"
	"    gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);\n"
	"}\n";


/**
 * Fragment shader source that sets red=dFdx(interpolated x
 * coordinate), green=dFdy(interpolated y coordinate), and blue=1.0,
 * with appropriate scaling applied to the red and green outputs so
 * that the expected output is red=0.5 and green=0.5.  The coordinates
 * are non-centroid interpolated.
 */
const char *frag_non_centroid_deriv =
	"#version 120\n"
	"varying vec2 pixel_pos;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = vec4(0.5*dFdx(pixel_pos.x),\n"
	"                      0.5*dFdy(pixel_pos.y),\n"
	"                      1.0, 1.0);\n"
	"}\n";


/**
 * Fragment shader source that sets red=dFdx(interpolated x
 * coordinate), green=dFdy(interpolated y coordinate), and blue=1.0,
 * with appropriate scaling applied to the red and green outputs so
 * that the expected output is red=0.5 and green=0.5.  The coordinates
 * are non-centroid interpolated.
 */
const char *frag_centroid_deriv =
	"#version 120\n"
	"centroid varying vec2 pixel_pos_centroid;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = vec4(0.5*dFdx(pixel_pos_centroid.x),\n"
        "                      0.5*dFdy(pixel_pos_centroid.y),\n"
	"                      1.0, 1.0);\n"
	"}\n";


/**
 * Fragment shader source that sets the blue channel to 1.0, and the
 * red and green channels to 1.0 if either derivative is out of
 * tolerance.
 */
const char *frag_centroid_deriv_range_check =
	"#version 120\n"
	"centroid varying vec2 pixel_pos_centroid;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  if (distance(1.0, dFdx(pixel_pos_centroid.x)) > 1.0 ||\n"
	"      distance(1.0, dFdy(pixel_pos_centroid.y)) > 1.0)\n"
	"    gl_FragColor = vec4(1.0);\n"
	"  else\n"
	"    gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);\n"
	"}\n";


/**
 * Fragment shader source that outputs blue (the expected output of
 * frag_centroid_range_check and frag_centroid_deriv_range_check).
 */
const char *frag_blue =
	"#version 120\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);\n"
	"}\n";


/**
 * Fragment shader source that sets red=0.5, green=0.5, and blue=1.0
 * (the expected output of frag_non_centroid_deriv and
 * frag_centroid_deriv).
 */
const char *frag_rg_0_5 =
	"#version 120\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = vec4(0.5, 0.5, 1.0, 1.0);\n"
	"}\n";


void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples> <test_type>\n"
	       "  where <test_type> is one of:\n"
	       "    non-centroid-disabled: non-centroid varying, MSAA off\n"
	       "    centroid-disabled: centroid varying, MSAA off\n"
	       "    centroid-edges: centroid behaviour at trinagle edges\n"
	       "    non-centroid-deriv: dFdx/dFdy on non-centroid varying\n"
	       "    non-centroid-deriv-disabled: As above, with MSAA off\n"
	       "    centroid-deriv: dFdx/dFdy on centroid varying\n"
	       "    centroid-deriv-disabled: As above, with MSAA off\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

extern "C" void
piglit_init(int argc, char **argv)
{
	if (argc != 3)
		print_usage_and_exit(argv[0]);

	/* 1st arg: num_samples */
	char *endptr = NULL;
	int num_samples = strtol(argv[1], &endptr, 0);
	if (endptr != argv[1] + strlen(argv[1]))
		print_usage_and_exit(argv[0]);

	/* 2nd arg: test_type */
	const char *frag; /* Fragment shader for the test image */
	const char *ref_frag; /* Fragment shader for the reference image */
	if (strcmp(argv[2], "non-centroid-disabled") == 0) {
		frag = frag_non_centroid_barycentric;
		ref_frag = frag_non_centroid_barycentric;
		disable_msaa_during_test_image = true;
	} else if (strcmp(argv[2], "centroid-disabled") == 0) {
		frag = frag_centroid_barycentric;
		ref_frag = frag_non_centroid_barycentric;
		disable_msaa_during_test_image = true;
	} else if (strcmp(argv[2], "centroid-edges") == 0) {
		if (num_samples == 0) {
			frag = frag_centroid_barycentric;
			ref_frag = frag_non_centroid_barycentric;
		} else {
			frag = frag_centroid_range_check;
			ref_frag = frag_blue;
		}
	} else if (strcmp(argv[2], "non-centroid-deriv") == 0) {
		frag = frag_non_centroid_deriv;
	        ref_frag = frag_rg_0_5;
	} else if (strcmp(argv[2], "non-centroid-deriv-disabled") == 0) {
		frag = frag_non_centroid_deriv;
	        ref_frag = frag_rg_0_5;
		disable_msaa_during_test_image = true;
	} else if (strcmp(argv[2], "centroid-deriv") == 0) {
		if (num_samples == 0) {
			frag = frag_centroid_deriv;
			ref_frag = frag_rg_0_5;
		} else {
			frag = frag_centroid_deriv_range_check;
			ref_frag = frag_blue;
		}
	} else if (strcmp(argv[2], "centroid-deriv-disabled") == 0) {
		frag = frag_centroid_deriv;
		ref_frag = frag_rg_0_5;
		disable_msaa_during_test_image = true;
	} else {
		print_usage_and_exit(argv[0]);
	}

	piglit_require_gl_version(21);
	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_vertex_array_object");
	piglit_require_extension("GL_EXT_framebuffer_multisample");

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	singlesampled_fbo.setup(FboConfig(0, pattern_width,
					  pattern_height));
	multisampled_fbo.setup(FboConfig(num_samples, pattern_width,
					 pattern_height));
	test_pattern = new InterpolationTestPattern(frag);
	test_pattern->compile();
	ref_pattern = new InterpolationTestPattern(ref_frag);
	ref_pattern->compile();

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}

extern "C" enum piglit_result
piglit_display()
{
	bool pass = true;

	/* Draw the test pattern into the multisampled buffer,
	 * disabling MSAA if appropriate.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampled_fbo.handle);
	multisampled_fbo.set_viewport();
	if (disable_msaa_during_test_image)
		glDisable(GL_MULTISAMPLE);
	test_pattern->draw(TestPattern::no_projection);
	if (disable_msaa_during_test_image)
		glEnable(GL_MULTISAMPLE);

	/* Blit the test pattern to the single-sampled buffer to force
	 * a resolve.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampled_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, singlesampled_fbo.handle);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Blit the test pattern to the left half of the piglit window. */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, singlesampled_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Draw the reference pattern.  If we disabled GL_MULTISAMPLE
	 * while drawing the test pattern, then draw the reference
	 * pattern into a single-sampled buffer so that multisampling
	 * won't take place; otherwise draw the reference pattern into
	 * the multisampled buffer.
	 */
	Fbo *draw_fbo = disable_msaa_during_test_image ?
		&singlesampled_fbo : &multisampled_fbo;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_fbo->handle);
	draw_fbo->set_viewport();
	ref_pattern->draw(TestPattern::no_projection);

	/* If we drew the reference pattern into the multisampled
	 * buffer, blit to the single-sampled buffer to force a
	 * resolve.
	 */
	if (!disable_msaa_during_test_image) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER,
				  multisampled_fbo.handle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
				  singlesampled_fbo.handle);
		glBlitFramebuffer(0, 0, pattern_width, pattern_height,
				  0, 0, pattern_width, pattern_height,
				  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	/* Blit the reference image to the right half of the piglit window. */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, singlesampled_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  pattern_width, 0, 2*pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Compare the test pattern to the reference image. */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	pass = piglit_probe_rect_halves_equal_rgba(0, 0, 2*pattern_width,
						   pattern_height) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

} /* Anonymous namespace */
