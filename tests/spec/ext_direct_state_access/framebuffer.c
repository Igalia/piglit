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
test_FramebufferDrawBufferEXT(void* d)
{
	int max_color_attachments, max_draw_buffers;
	GLuint fbs[3];
	int i, j, got;
	GLenum* attachments;

	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_color_attachments);
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);

	attachments = (GLenum*) malloc((max_color_attachments + 1) * sizeof(GLenum));
	for (i = 0; i < max_color_attachments; i++) {
		attachments[i] = GL_COLOR_ATTACHMENT0 + i;
	}
	attachments[max_color_attachments] = GL_NONE;
	max_color_attachments += 1;

	glGenFramebuffers(ARRAY_SIZE(fbs), fbs);

	glBindFramebuffer(GL_FRAMEBUFFER, fbs[0]);
	glFramebufferDrawBufferEXT(fbs[1], attachments[1]);

	for (i = 0; i < max_color_attachments; i++) {
		const int buffer_count = MIN2(max_draw_buffers, max_color_attachments - i);

		glFramebufferDrawBufferEXT(fbs[1], attachments[i]);
		glFramebufferDrawBuffersEXT(fbs[2],
					    buffer_count,
					    &attachments[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, fbs[1]);
		glGetIntegerv(GL_DRAW_BUFFER, &got);

		if (got != attachments[i]) {
			piglit_loge("glFramebufferDrawBufferEXT(..., %s) failed. Got %s\n",
				    piglit_get_gl_enum_name(attachments[i]),
				    piglit_get_gl_enum_name(got));
			return PIGLIT_FAIL;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, fbs[2]);
		for (j = 0; j < buffer_count; j++) {
			glGetIntegerv(GL_DRAW_BUFFER0 + j, &got);
			if (got != attachments[i + j]) {
				piglit_loge("glFramebuffersDrawBufferEXT(..., %d, ...) failed.\n"
					    "Buffer %d: expected %s but got %s\n",
					    buffer_count,
					    j,
					    piglit_get_gl_enum_name(attachments[i + j]),
					    piglit_get_gl_enum_name(got));
				return PIGLIT_FAIL;
			}
		}
	}

	glDeleteFramebuffers(ARRAY_SIZE(fbs), fbs);

	return piglit_check_gl_error(GL_NO_ERROR) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_FramebufferReadDrawBufferEXTDefault(void* d)
{
	GLuint fb;
	int i, got;
	static const GLenum attachments[] = {
		GL_NONE,
		GL_FRONT, GL_BACK,
		GL_LEFT, GL_FRONT_AND_BACK,
		GL_FRONT_LEFT, GL_BACK_LEFT
	};
	glGenFramebuffers(1, &fb);
	glFramebufferDrawBufferEXT(0, attachments[1]);
	glFramebufferReadBufferEXT(0, attachments[1]);

	for (i = 0; i < ARRAY_SIZE(attachments); i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, fb);
		glFramebufferDrawBufferEXT(0, attachments[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glGetIntegerv(GL_DRAW_BUFFER, &got);

		if (!piglit_check_gl_error(GL_NO_ERROR) || got != attachments[i]) {
			piglit_loge("glFramebufferDrawBufferEXT(0, %s) failed. Got %s\n",
				    piglit_get_gl_enum_name(attachments[i]),
				    piglit_get_gl_enum_name(got));
			return PIGLIT_FAIL;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, fb);
	}

	for (i = 0; i < ARRAY_SIZE(attachments); i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, fb);
		glFramebufferReadBufferEXT(0, attachments[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glGetIntegerv(GL_READ_BUFFER, &got);

		if (!piglit_check_gl_error(GL_NO_ERROR) || got != attachments[i]) {
			piglit_loge("glFramebufferReadBufferEXT(0, %s) failed. Got %s\n",
				    piglit_get_gl_enum_name(attachments[i]),
				    piglit_get_gl_enum_name(got));
			return PIGLIT_FAIL;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, fb);
	}

	glDeleteFramebuffers(1, &fb);

	return piglit_check_gl_error(GL_NO_ERROR) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_FramebufferReadBufferEXT(void* d)
{
	int max_color_attachments;
	GLuint fb;
	int i, got;
	GLenum* attachments;

	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_color_attachments);

	attachments = (GLenum*) malloc((max_color_attachments + 1) * sizeof(GLenum));
	for (i = 0; i < max_color_attachments; i++) {
		attachments[i] = GL_COLOR_ATTACHMENT0 + i;
	}
	attachments[max_color_attachments] = GL_NONE;
	max_color_attachments += 1;

	glGenFramebuffers(1, &fb);
	glFramebufferReadBufferEXT(fb, attachments[1]);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	for (i = 0; i < max_color_attachments; i++) {
		glFramebufferReadBufferEXT(fb, attachments[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, fb);
		glGetIntegerv(GL_READ_BUFFER, &got);

		if (got != attachments[i]) {
			piglit_loge("glFramebufferReadBufferEXT(..., %s) failed. Got %s\n",
				    piglit_get_gl_enum_name(attachments[i]),
				    piglit_get_gl_enum_name(got));
			return PIGLIT_FAIL;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	glDeleteFramebuffers(1, &fb);

	return piglit_check_gl_error(GL_NO_ERROR) ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
test_GetFramebufferParameterivEXT(void* data)
{
	int i;
	GLuint fb;
	int got, expected;
	int max_draw_buffers;
	static GLenum pnames[2 + 16];
	pnames[0] = GL_DRAW_BUFFER;
	pnames[1] = GL_READ_BUFFER;

	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);
	for (i = 0; i < max_draw_buffers; i++) {
		pnames[2 + i] = GL_DRAW_BUFFER0 + i;
	}

	/* The GL_EXT_direct_state_access says:
	 *
	 * The query returns the same value in param that GetIntegerv would
	 * return if called with pname and param as if the framebuffer specified
	 * by the framebuffer parameter had been bound with BindFramebuffer
	 */
	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	for (i = 0; i < 2 + max_draw_buffers; i++) {
		glGetFramebufferParameterivEXT(fb, pnames[i], &got);

		glBindFramebuffer(GL_FRAMEBUFFER, fb);
		glGetIntegerv(pnames[i], &expected);

		if (got != expected) {
			piglit_loge("glGetFramebufferParameterivEXT(..., %s, ...) failed.\n"
				    "Expected %s but got %s\n",
				    piglit_get_gl_enum_name(pnames[i]),
				    piglit_get_gl_enum_name(expected),
				    piglit_get_gl_enum_name(got));
			return PIGLIT_FAIL;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	glDeleteFramebuffers(1, &fb);
	return piglit_check_gl_error(GL_NO_ERROR) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static GLenum
dimension_to_target(int n)
{
	assert(n == 1 || n == 2 || n == 3);
	switch (n) {
		case 1: return GL_TEXTURE_1D;
		case 2: return GL_TEXTURE_2D;
		case 3:
		default:
			return GL_TEXTURE_3D;
	}
}

static enum piglit_result
test_NamedFramebufferTextureNDEXT(void* data)
{
	const int n = (int)(intptr_t) data;
	const GLenum target = dimension_to_target(n);
	GLuint color_texture;
	GLuint framebuffer;
	int got;

	glGenTextures(1, &color_texture);
	glBindTexture(target, color_texture);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	if (target == GL_TEXTURE_1D) {
		glTexImage1D(target, 0, GL_RGBA, piglit_width,
			     0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	} else if (target == GL_TEXTURE_2D) {
		glTexImage2D(target, 0, GL_RGBA, piglit_width, piglit_height,
			     0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	} else {
		glTexImage3D(target, 0, GL_RGBA, piglit_width, piglit_height, 1,
			     0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}
	glBindTexture(target, 0);

	glGenFramebuffers(1, &framebuffer);

	if (target == GL_TEXTURE_1D) {
		glNamedFramebufferTexture1DEXT(framebuffer, GL_COLOR_ATTACHMENT0,
					       target, color_texture, 0);
	} else if (target == GL_TEXTURE_2D) {
		glNamedFramebufferTexture2DEXT(framebuffer, GL_COLOR_ATTACHMENT0,
					       target, color_texture, 0);
	} else if (target == GL_TEXTURE_3D) {
		glNamedFramebufferTexture3DEXT(framebuffer, GL_COLOR_ATTACHMENT0,
					       target, color_texture, 0, 0);
	}

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		return PIGLIT_FAIL;
	}
	if (glCheckNamedFramebufferStatusEXT(framebuffer, GL_FRAMEBUFFER)
	    != GL_FRAMEBUFFER_COMPLETE) {
		return PIGLIT_FAIL;
	}

	glGetNamedFramebufferAttachmentParameteriv(framebuffer,
						   GL_COLOR_ATTACHMENT0,
						   GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
						   &got);
	if (got != GL_TEXTURE) {
		return PIGLIT_FAIL;
	}

	glGetNamedFramebufferAttachmentParameteriv(framebuffer,
						   GL_COLOR_ATTACHMENT0,
						   GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
						   &got);
	if (got != color_texture) {
		return PIGLIT_FAIL;
	}

	glDeleteFramebuffers(1, &framebuffer);

	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_direct_state_access");
	struct piglit_subtest tests[] = {
		{
			"FramebufferDrawBufferEXT",
			NULL,
			test_FramebufferDrawBufferEXT
		},
		{
			"FramebufferReadDrawBufferEXT (default framebuffer)",
			NULL,
			test_FramebufferReadDrawBufferEXTDefault
		},
		{
			"FramebufferReadBufferEXT",
			NULL,
			test_FramebufferReadBufferEXT,
		},
		{
			"GetFramebufferParameterivEXT",
			NULL,
			test_GetFramebufferParameterivEXT,
		},
		{
			"NamedFramebufferTexture1DEXT",
			NULL,
			test_NamedFramebufferTextureNDEXT,
			(void*) 1
		},
		{
			"NamedFramebufferTexture2DEXT",
			NULL,
			test_NamedFramebufferTextureNDEXT,
			(void*) 2
		},
		{
			"NamedFramebufferTexture3DEXT",
			NULL,
			test_NamedFramebufferTextureNDEXT,
			(void*) 3
		},
		{
			NULL
		}
	};

	piglit_report_result(piglit_run_selected_subtests(tests, NULL, 0, PIGLIT_PASS));
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}

