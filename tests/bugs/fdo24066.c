/*
 * Copyright (c) 2009 Nicolai Hähnle
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
 * Authors:
 *    Nicolai Hähnle <nhaehnle@gmail.com>
 *
 */

/**
 * \file
 * Test for the crash reported in bugs.freedesktop.org bug #24066.
 * This occured when the native limits of a vertex program are queried
 * before a fragment program has been setup.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result piglit_display(void)
{
	return PIGLIT_PASS;
}


static const char program_text[] =
	"!!ARBvp1.0\n"
	"MOV result.position, vertex.position;\n"
	"END";

void piglit_init(int argc, char ** argv)
{
	GLuint program_object;
	GLint result;

	(void) argc;
	(void) argv;

	piglit_require_vertex_program();

	program_object = piglit_compile_program(GL_VERTEX_PROGRAM_ARB, program_text);

	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, program_object);

	printf("Testing whether the following call crashes...\n");
	glGetProgramivARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &result);

	piglit_report_result(PIGLIT_PASS);
}
