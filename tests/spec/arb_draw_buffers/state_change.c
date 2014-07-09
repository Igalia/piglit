/*
 * Copyright © 2011 Henri Verbeet <hverbeet@gmail.com>
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
 *
 */

/* Test switching between various draw buffers. In particular, this tests that
 * glDrawBuffersARB() enables the correct buffers when only the buffer count
 * changes. This is for a bug in _mesa_drawbuffers() where it would fail to
 * set the remaining buffers to NONE when only the first buffer was updated.
 * It would then fail to enable the second buffer again because it was already
 * pointing to the new buffer. */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 128;
	config.window_height = 128;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static void
check_fbo_status(void)
{
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf("FBO incomplete, status %#x.\n", status);
		piglit_report_result(PIGLIT_FAIL);
	}
}

enum piglit_result
piglit_display(void)
{
	static GLenum buffers[] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT};
	static const float red[] = {1.0f, 0.0f, 0.0f};
	static const float green[] = {0.0f, 1.0f, 0.0f};
	static const float blue[] = {0.0f, 0.0f, 1.0f};
	static const struct {
		GLsizei buffer_count;
		const float *clear_color;
		const float *expected_0;
		const float *expected_1;
	} tests[] = {
		{2, red, red, red},
		{1, green, green, red},
		{2, blue, blue, blue},
	};
	int w = piglit_width;
	int h = piglit_height;
	GLuint fbo, tex[2];
	unsigned i;

	glGenTextures(2, tex);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h,
		     0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h,
		     0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D, tex[0], 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT,
				  GL_TEXTURE_2D, tex[1], 0);
	check_fbo_status();
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	for (i = 0; i < sizeof(tests) / sizeof(*tests); ++i) {
		GLint buffer, expected_buffer;

		glDrawBuffersARB(tests[i].buffer_count, buffers);
		check_fbo_status();

		glGetIntegerv(GL_DRAW_BUFFER1_ARB, &buffer);
		expected_buffer = tests[i].buffer_count < 2 ? GL_NONE : GL_COLOR_ATTACHMENT1_EXT;
		if (buffer != expected_buffer) {
			printf("Unexpected buffer %#x for DRAW_BUFFER1_ARB in test %u, expected %#x.\n",
			       buffer, i, expected_buffer);
			piglit_report_result(PIGLIT_FAIL);
		}

		glClearColor(tests[i].clear_color[0], tests[i].clear_color[1],
			     tests[i].clear_color[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
		if (!piglit_probe_pixel_rgb(w / 2, h / 2, tests[i].expected_0)) {
			printf("Probe failed for test %u, attachment 0.\n", i);
			piglit_report_result(PIGLIT_FAIL);
		}

		glReadBuffer(GL_COLOR_ATTACHMENT1_EXT);
		if (!piglit_probe_pixel_rgb(w / 2, h / 2, tests[i].expected_1)) {
			printf("Probe failed for test %u, attachment 1.\n", i);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	GLint i;

	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_ARB_draw_buffers");
	glGetIntegerv(GL_MAX_DRAW_BUFFERS_ARB, &i);
	if (i < 2) {
		printf("2 draw buffers required, %d reported.\n", i);
		piglit_report_result(PIGLIT_SKIP);
	}
}
