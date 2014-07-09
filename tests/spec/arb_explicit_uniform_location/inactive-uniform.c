/*
 * Copyright © 2014 Intel Corporation
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
 * \file inactive-uniform.c
 *
 * Tests that glUniform works as specified for inactive uniform with
 * explicit location. This is notably also the only uniform in the shader,
 * which can be special case in the linker.
 *
 * The GL_ARB_explicit_uniform_location spec says:
 *
 *     "No two default-block uniform variables in the program can have
 *     the same location, even if they are unused, otherwise a compiler
 *     or linker error will be generated."
 *
 * note also in the Issues section:
 *
 *     "What happens if Uniform* is called with an explicitly defined
 *     uniform location, but that uniform is deemed inactive by the
 *     linker?
 *
 *     RESOLVED: The call is ignored for inactive uniform variables and
 *     no error is generated."
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 33;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static const char vs_text[] =
	"vec4 vertex;\n"
	"void main() {\n"
		"gl_Position = vertex;\n"
	"}";

static const char fs_text[] =
	"#version 330\n"
	"#extension GL_ARB_explicit_uniform_location: require\n"
	"layout(location = 7) uniform float var;\n"
	"void main() {\n"
		"gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);\n"
	"}";

void
piglit_init(int argc, char **argv)
{
	GLuint prog;

	piglit_require_extension("GL_ARB_explicit_uniform_location");

	prog = piglit_build_simple_program(vs_text, fs_text);

	glUseProgram(prog);

	/* verify that glUniform1f does not generate error */
	glUniform1f(7, 0.1);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glDeleteProgram(prog);
	piglit_report_result(PIGLIT_PASS);
}
