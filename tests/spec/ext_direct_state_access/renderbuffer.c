/*
 * Copyright © 2019 Advanced Micro Devices, Inc.
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

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
		PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static enum piglit_result
test_NamedRenderbufferStorageEXT(void* data)
{
	GLuint color_renderbuffer;
	GLuint framebuffer;
	GLint value;

	glGenRenderbuffers(1, &color_renderbuffer);
	glNamedRenderbufferStorageEXT(color_renderbuffer, GL_RGBA8,
				      piglit_width, piglit_height);

	glGenFramebuffers(1, &framebuffer);
	glNamedFramebufferRenderbufferEXT(framebuffer, GL_COLOR_ATTACHMENT0,
					  GL_RENDERBUFFER, color_renderbuffer);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		return PIGLIT_FAIL;
	}
	if (glCheckNamedFramebufferStatusEXT(framebuffer, GL_FRAMEBUFFER) !=
	    GL_FRAMEBUFFER_COMPLETE) {
		return PIGLIT_FAIL;
	}

	glGetNamedRenderbufferParameterivEXT(color_renderbuffer, GL_RENDERBUFFER_WIDTH, &value);
	if (value != piglit_width) {
		return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}

static enum piglit_result
test_GetNamedRenderbufferParameterivEXT(void* data)
{
	GLuint renderbuffer;

	static const GLenum pnames[] = {
		GL_RENDERBUFFER_WIDTH, GL_RENDERBUFFER_HEIGHT,
		GL_RENDERBUFFER_INTERNAL_FORMAT,
		GL_RENDERBUFFER_RED_SIZE, GL_RENDERBUFFER_GREEN_SIZE,
		GL_RENDERBUFFER_BLUE_SIZE, GL_RENDERBUFFER_ALPHA_SIZE,
		GL_RENDERBUFFER_DEPTH_SIZE, GL_RENDERBUFFER_STENCIL_SIZE
	};
	/* Expected values after calling:
	 *     glNamedRenderbufferStorageEXT(renderbuffer, GL_RGB5_A1, 64, 64)
	 */
	static const int expected_values[] = {
		64, 64, GL_RGB5_A1, 5, 5, 5, 1, 0, 0
	};
	int i;

	glGenRenderbuffers(1, &renderbuffer);

	/* Verify default values */
	for (i = 0; i < ARRAY_SIZE(pnames); i++) {
		int value;
		const int expected = (pnames[i] == GL_RENDERBUFFER_INTERNAL_FORMAT) ?
			GL_RGBA : 0;

		glGetNamedRenderbufferParameterivEXT(
			renderbuffer,
			pnames[i],
			&value);

		if (value != expected) {
			piglit_loge("glGetNamedRenderbufferParameterivEXT(..., %s, ...) failed."
				    "Expected %d but got %d\n",
				    piglit_get_gl_enum_name(pnames[i]),
				    expected, value);
			return PIGLIT_FAIL;
		}
	}

	glNamedRenderbufferStorageEXT(renderbuffer, GL_RGB5_A1, 64, 64);

	for (i = 0; i < ARRAY_SIZE(pnames); i++) {
		int value;
		const int expected = expected_values[i];

		glGetNamedRenderbufferParameterivEXT(
			renderbuffer,
			pnames[i],
			&value);

		if (value != expected) {
			piglit_loge("glGetNamedRenderbufferParameterivEXT(..., %s, ...) failed."
				    "Expected %d but got %d\n",
				    piglit_get_gl_enum_name(pnames[i]),
				    expected, value);
			return PIGLIT_FAIL;
		}
	}

	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	static const struct piglit_subtest tests[] = {
		{
			"NamedRenderbufferStorageEXT",
			NULL,
			test_NamedRenderbufferStorageEXT
		},
		{
			"GetNamedRenderbufferParameterivEXT",
			NULL,
			test_GetNamedRenderbufferParameterivEXT
		},
		{
			NULL
		}
	};

	piglit_require_extension("GL_EXT_direct_state_access");

	return piglit_report_result(
		piglit_run_selected_subtests(tests, NULL, 0, PIGLIT_PASS));
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}
