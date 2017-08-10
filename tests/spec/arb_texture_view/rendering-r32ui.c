/*
 * Copyright (c) 2017 Józef Kucia <joseph.kucia@gmail.com>
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
 * \file rendering-r32ui.c
 * Exercises a Radeonsi bug.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

config.supports_gl_compat_version = 30;
config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char *vs =
	"#version 130\n"
	"void main() { \n"
	"	gl_Position = gl_Vertex;\n"
	"}\n";

static const char *ps =
	"#version 130\n"
	"out uvec4 color;\n"
	"void main() {\n"
	"	color = uvec4(0xff, 0, 0, 0);\n"
	"}\n";

#define TEX_SIZE 64

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint tex, view, framebuffer, prog;
	GLuint data[TEX_SIZE * TEX_SIZE];
	bool pass = true;

	piglit_require_gl_version(30);
	piglit_require_extension("GL_ARB_texture_view");

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, TEX_SIZE, TEX_SIZE);

	glGenTextures(1, &view);
	glTextureView(view, GL_TEXTURE_2D, tex, GL_R32UI, 0, 1, 0, 1);

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, view, 0);

	prog = piglit_build_simple_program(vs, ps);
	glUseProgram(prog);

	piglit_draw_rect(-1, -1, 2, 2);

	glBindTexture(GL_TEXTURE_2D, view);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT,
		      data);
	if (data[0] != 0xff) {
		printf("Got value %#x\n", data[0]);
		pass = false;
	}

	glDeleteTextures(1, &view);
	glDeleteTextures(1, &tex);
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteProgram(prog);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
