/*
 * Copyright © 2012, 2015 Intel Corporation
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
 * @file blit-mismatched-formats.cpp
 *
 * This test verifies that calling glBlitFramebuffer to blit between
 * two multisampled framebuffers works even if they have different
 * formats. Note that originally the GL spec required that blitting
 * between differing formats should report a GL_INVALID_OPERATION
 * error. However in practice most drivers allowed it anyway and in
 * the GL 4.4 spec the restriction was removed. I think it can be
 * considered a mistake in the spec that this was not the case
 * originally so this test assumes that it should be possible in any
 * version.
 *
 * We initialize two FBOs with minimum supported sample count and
 * different buffer formats, do blitting operation between them and
 * verify the expected results.
 *
 * Authors: Anuj Phogat <anuj.phogat@gmail.com>
 *	    Neil Roberts <neil@linux.intel.com>
 */

#include "piglit-test-pattern.h"
#include "piglit-fbo.h"
using namespace piglit_util_fbo;
using namespace piglit_util_test_pattern;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 256;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

const int pattern_width = 256; const int pattern_height = 256;
Fbo src_fbo, dst_fbo, ss_fbo;
ColorGradientSunburst *test_pattern;
GLfloat *reference_image;

enum component {
	COMPONENT_RED = (1 << 0),
	COMPONENT_GREEN = (1 << 1),
	COMPONENT_BLUE = (1 << 2),
	COMPONENT_ALPHA = (1 << 3),
};

struct color_format {
	GLenum name;
	GLbitfield components;
};

static struct color_format
color_formats[] = {
	{ GL_ALPHA, COMPONENT_ALPHA },
	{ GL_RED, COMPONENT_RED },
	{ GL_RG, COMPONENT_RED | COMPONENT_GREEN },
	{ GL_RGB, COMPONENT_RED | COMPONENT_GREEN | COMPONENT_BLUE }
};

static GLfloat *
generate_expected_image(const GLfloat *ref_image,
			int pattern_width, int pattern_height,
			GLbitfield components)
{
	GLfloat *expected_image = (GLfloat *) malloc(pattern_width *
						     pattern_height * 4 *
						     sizeof(GLfloat));
	GLfloat *dst = expected_image;
	const GLfloat *src = ref_image;
	int i;

	for (i = 0; i < pattern_width * pattern_height; i++) {
		dst[0] = (components & COMPONENT_RED) ? src[0] : 0.0f;
		dst[1] = (components & COMPONENT_GREEN) ? src[1] : 0.0f;
		dst[2] = (components & COMPONENT_BLUE) ? src[2] : 0.0f;
		dst[3] = (components & COMPONENT_ALPHA) ? src[3] : 1.0f;
		src += 4;
		dst += 4;
	}

	return expected_image;
}

enum piglit_result
piglit_display()
{
	bool pass = true;
	GLfloat *expected_image;
	GLuint i;

	FboConfig config_ms(1 , pattern_width, pattern_height);

	for(i = 0; i < ARRAY_SIZE(color_formats); i++) {
		expected_image =
			generate_expected_image(reference_image,
						pattern_width, pattern_height,
						color_formats[i].components);

		config_ms.color_internalformat = color_formats[i].name;
		src_fbo.setup(config_ms);

		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			printf("Error setting up renderbuffer color format\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, src_fbo.handle);
		test_pattern->draw(TestPattern::no_projection);

		/* Blit multisample-to-multisample with non-matching formats */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo.handle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_fbo.handle);

		glClear(GL_COLOR_BUFFER_BIT);

		glBlitFramebuffer(0, 0, pattern_width, pattern_height,
				  0, 0, pattern_width, pattern_height,
				  GL_COLOR_BUFFER_BIT, GL_NEAREST);

		/* Blitting between different formats shouldn't
		 * generate an error.
		 */
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		/* Downsample the blitted buffer so we can read the
		 * results
		 */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, dst_fbo.handle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ss_fbo.handle);
		glClear(GL_COLOR_BUFFER_BIT);
		glBlitFramebuffer(0, 0, pattern_width, pattern_height,
				  0, 0, pattern_width, pattern_height,
				  GL_COLOR_BUFFER_BIT, GL_NEAREST);

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		glBindFramebuffer(GL_FRAMEBUFFER, ss_fbo.handle);
		pass = piglit_probe_image_rgba(0, 0,
					       pattern_width, pattern_height,
					       expected_image) && pass;

		/* Also try a downsample blit with mismatched formats
		 */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo.handle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ss_fbo.handle);
		glClear(GL_COLOR_BUFFER_BIT);
		glBlitFramebuffer(0, 0, pattern_width, pattern_height,
				  0, 0, pattern_width, pattern_height,
				  GL_COLOR_BUFFER_BIT, GL_NEAREST);

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		glBindFramebuffer(GL_FRAMEBUFFER, ss_fbo.handle);
		pass = piglit_probe_image_rgba(0, 0,
					       pattern_width, pattern_height,
					       expected_image) && pass;

		/* Blit the result to the window system buffer so that
		 * something will be displayed in a non-automatic
		 * test.
		 */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, ss_fbo.handle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glBlitFramebuffer(0, 0, pattern_width, pattern_height,
				  0, 0, pattern_width, pattern_height,
				  GL_COLOR_BUFFER_BIT, GL_NEAREST);

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		piglit_present_results();

		free(expected_image);
	}

	return (pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(21);
	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_vertex_array_object");

	/* Passing sample count = 1 will create the FBOs with minimum supported
	 * sample count. Both FBOs are created with GL_RGBA format by default.
	 */
	src_fbo.setup(FboConfig(1 /* sample count */,
				pattern_width,
				pattern_height));

	dst_fbo.setup(FboConfig(1 /* sample count */,
				pattern_width,
				pattern_height));

	/* Single-sampled FBO used so that we can call glReadPixels to
	 * examine the results.
	 */
	ss_fbo.setup(FboConfig(0 /* sample count */,
			       pattern_width,
			       pattern_height));

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Error setting up frame buffer objects\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	test_pattern = new ColorGradientSunburst(GL_FLOAT);
	test_pattern->compile();

	glBindFramebuffer(GL_FRAMEBUFFER, src_fbo.handle);
	test_pattern->draw(TestPattern::no_projection);

	/* Generate a reference image by downsampling between matching
	 * formats.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ss_fbo.handle);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	reference_image = (GLfloat *) malloc(pattern_width *
					     pattern_height * 4 *
					     sizeof(GLfloat));

	glBindFramebuffer(GL_FRAMEBUFFER, ss_fbo.handle);
	glReadPixels(0, 0, pattern_width, pattern_height,
		     GL_RGBA, GL_FLOAT, reference_image);
}
