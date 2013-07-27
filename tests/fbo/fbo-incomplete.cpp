/*
 * Copyright © 2013 Intel Corporation
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
 * \file fbo-incomplete.c
 * Collection of negative framebuffer completeness tests.
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const float green[] = { 0.f, 1.f, 0.f, 1.f };

class incomplete_fbo_test {
public:

	incomplete_fbo_test(const char *_name, GLenum _target)
		: name(_name), target(_target), tex(0), rb(0), fbo(0),
		  _pass(true)
	{
		if (target == GL_RENDERBUFFER) {
			glGenRenderbuffers(1, &rb);
			glBindRenderbuffer(target, rb);
		} else {
			glGenTextures(1, &tex);
			glBindTexture(target, tex);
			glTexParameteri(target,
					GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	}


	~incomplete_fbo_test()
	{
		if (target == GL_RENDERBUFFER)
			glBindRenderbuffer(target, 0);
		else
			glBindTexture(target, 0);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		glDeleteTextures(1, &tex);
		glDeleteRenderbuffers(1, &rb);
		glDeleteFramebuffers(1, &fbo);

		piglit_report_subtest_result(_pass ? PIGLIT_PASS : PIGLIT_FAIL,
					     name);
	}

	bool check_fbo_status(GLenum expect)
	{
		GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		if (status != expect) {
			fprintf(stderr,
				"status was %s (0x%04x), "
				"expected %s (0x%04x).\n",
				piglit_get_gl_enum_name(status),
				status,
				piglit_get_gl_enum_name(expect),
				expect);
			return false;
		}

		return true;
	}

	bool pass()
	{
		_pass = true;
		return true;
	}

	bool fail()
	{
		_pass = false;
		return false;
	}

	const char *name;
	GLenum target;
	GLuint tex;
	GLuint rb;
	GLuint fbo;

private:
	bool _pass;
};

/**
 * Verify that attaching a 0x0 texture results in incompleteness.
 */
bool
incomplete_0_by_0_texture(void)
{
	incomplete_fbo_test t("0x0 texture", GL_TEXTURE_2D);

	/* Attach a 0x0 texture to the framebuffer.  That should make it
	 * incomplete.
	 */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, t.tex, 0);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return t.fail();

	if (!t.check_fbo_status(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT))
		return t.fail();

	/* Allocate some storage for the texture and verify that the FBO is
	 * now complete.
	 */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, NULL);

	if (!t.check_fbo_status(GL_FRAMEBUFFER_COMPLETE))
		return t.fail();

	/* Verify that simple rendering can occur to the FBO.
	 */
	glClearColor(0.f, 1.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, t.fbo);
	if (!piglit_probe_rect_rgba(0, 0, 4, 4, green))
		return t.fail();

	return t.pass();
}

/**
 * Verify that attaching a 0x0 renderbuffer results in incompleteness.
 */
bool
incomplete_0_by_0_renderbuffer(void)
{
	incomplete_fbo_test t("0x0 renderbuffer", GL_RENDERBUFFER);

	/* Attach a 0x0 renderbuffer to the framebuffer.  That should make it
	 * incomplete.
	 */
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 0, 0);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, t.rb);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return t.fail();

	if (!t.check_fbo_status(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT))
		return t.fail();

	/* Allocate some storage for the renderbuffer and verify that
	 * the FBO is now complete.
	 */
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 4, 4);

	if (!t.check_fbo_status(GL_FRAMEBUFFER_COMPLETE))
		return t.fail();

	/* Verify that simple rendering can occur to the FBO.
	 */
	glClearColor(0.f, 1.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, t.fbo);
	if (!piglit_probe_rect_rgba(0, 0, 4, 4, green))
		return t.fail();

	return t.pass();
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_ARB_framebuffer_object");

	pass = incomplete_0_by_0_texture() && pass;
	pass = incomplete_0_by_0_renderbuffer() && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
