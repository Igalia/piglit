/*
 * Copyright © 2014 Red Hat
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
 * \file double_in_bool_uniform.c
 *
 * Tests that glUniform works as specified for trying to using the double
 * interfaces with a bool uniform.
 *
 * The GL_ARB_gpu_shader_fp64 specification says in the Issues section:
 * "(15) Can the 64-bit uniform APIs be used to load values for uniforms of
 *       type "bool", "bvec2", "bvec3", or "bvec4"?
 *    RESOLVED:  No.  OpenGL 2.0 and beyond did allow "bool" variable to be
 *    set with Uniform*i* and Uniform*f APIs, and OpenGL 3.0 extended that
 *    support to Uniform*ui* for orthogonality.  But it seems pointless to
 *    extended this capability forward to 64-bit Uniform APIs as well."
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
	"uniform bool var;\n"
	"void main() {\n"
		"gl_FragColor = vec4(float(var), 0.0, 1.0, 1.0);\n"
	"}";

void
piglit_init(int argc, char **argv)
{
	GLuint prog;
	GLint loc;
	piglit_require_extension("GL_ARB_gpu_shader_fp64");

	prog = piglit_build_simple_program(vs_text, fs_text);

	glUseProgram(prog);

	loc = glGetUniformLocation(prog, "var");

	/* verify that glUniform1d generates an error */
	glUniform1d(loc, 0.1);

	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);

	glDeleteProgram(prog);
	piglit_report_result(PIGLIT_PASS);
}
