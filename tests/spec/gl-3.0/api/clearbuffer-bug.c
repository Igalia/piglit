/* Copyright © 2020 Intel Corporation
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
 * \file clearbuffer-bug.c
 * Verify clear buffer correctness. Based on test case in bug:
 * https://gitlab.freedesktop.org/mesa/mesa/-/issues/3783
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 30;
PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

const char *v_str =
	"attribute vec4 piglit_vertex;\n"
	"void main() {\n"
		"gl_Position = piglit_vertex;\n"
	"}";

const char* f_str =
	"#version 110\n"
	"void main() {\n"
		"gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"gl_FragDepth = 0.3;\n"
	"}";

void piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(30);
	piglit_require_GLSL_version(110);

	bool pass = true;

	GLint program = piglit_build_simple_program(v_str, f_str);

	if (!program)
		piglit_report_result(PIGLIT_FAIL);

	glUseProgram(program);

	/* Create depth/stencil texture. */
	GLuint ds_tex;
	glGenTextures(1, &ds_tex);
	glBindTexture(GL_TEXTURE_2D, ds_tex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH32F_STENCIL8, 4, 4);

	/* Create color texture. */
	GLuint c_tex;
	glGenTextures(1, &c_tex);
	glBindTexture(GL_TEXTURE_2D, c_tex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 4, 4);

	/* Create fbo with attachments. */
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, c_tex, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, ds_tex, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	GLint error = glGetError();

	if (status != GL_FRAMEBUFFER_COMPLETE)
		piglit_report_result(PIGLIT_SKIP);

	if (error != 0) {
		pass = false;
		goto fail;
	}

	glClearBufferfi(GL_DEPTH_STENCIL, 0, 0.3, 3);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_EQUAL);

	piglit_draw_rect(-1.0, -1.0, 2.0, 2.0);

	float expect[] = { 1, 0, 0, 1 };
	if (!piglit_probe_rect_rgba(0, 0, 4, 4, expect))
		pass = false;

fail:
	glDeleteTextures(1, &c_tex);
	glDeleteTextures(1, &ds_tex);
	glDeleteFramebuffers(1, &fbo);
	glDeleteProgram(program);
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
