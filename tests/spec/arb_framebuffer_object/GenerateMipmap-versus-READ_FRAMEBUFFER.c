/*
 * Copyright © 2015 Intel Corporation
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

/** \file GenerateMipmap-versus-READ_FRAMEBUFFER.c
 * Verify that GL_READ_FRAMEBUFFER_BINDING and GL_DRAW_FRAMEBUFFER_BINDING are
 * correct after calling glGenerateMipmap.
 *
 * Mesa's meta path for glGenerateMipmap had a problem that it assumed the two
 * bindings were the same.  As a result, one of them was incorrect after
 * returning from _mesa_meta_GenerateMipmap.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const GLuint texels[16] = {
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0
};

void
piglit_init(int argc, char **argv)
{
	GLuint fbos[2];
	GLuint texture;
	GLuint draw_binding;
	GLuint read_binding;
	bool pass = true;

	if (piglit_get_gl_version() >= 30 ||
	    piglit_is_extension_supported("GL_ARB_framebuffer_object")) {
		glGenFramebuffers(2, fbos);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbos[0]);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbos[1]);
	} else if (piglit_is_extension_supported("GL_EXT_framebuffer_object") &&
		   piglit_is_extension_supported("GL_EXT_framebuffer_blit")) {
		glGenFramebuffersEXT(2, fbos);
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, fbos[0]);
		glBindFramebufferEXT(GL_READ_FRAMEBUFFER, fbos[1]);
	} else {
		fprintf(stderr,
			"Either OpenGL 3.0, or GL_ARB_framebuffer_object, or "
			"GL_EXT_framebuffer_object and GL_EXT_framebuffer_blit "
			"is requred.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0,
		     GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, texels);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glGenerateMipmap(GL_TEXTURE_2D);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint *) &draw_binding);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint *) &read_binding);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	if (draw_binding != fbos[0]) {
		fprintf(stderr, "GL_DRAW_FRAMEBUFFER_BINDING munged.  "
			"Got %u, expected %u.\n",
			draw_binding, fbos[0]);
		pass = false;
	}

	if (read_binding != fbos[1]) {
		fprintf(stderr, "GL_READ_FRAMEBUFFER_BINDING munged.  "
			"Got %u, expected %u.\n",
			read_binding, fbos[1]);
		pass = false;
	}

	if (piglit_get_gl_version() >= 30 ||
	    piglit_is_extension_supported("GL_ARB_framebuffer_object")) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glDeleteFramebuffers(2, fbos);
	} else {
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebufferEXT(GL_READ_FRAMEBUFFER, 0);
		glDeleteFramebuffersEXT(2, fbos);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &texture);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display()
{
	/* Unreachable */
	return PIGLIT_FAIL;
}
