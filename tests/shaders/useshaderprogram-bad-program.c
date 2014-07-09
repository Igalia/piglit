/*
 * Copyright © 2010 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file useshaderprogram-bad-program.c
 * Call glUseShaderProgramEXT with a bad program, verify the error generated
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char bad_vs_text[] =
	"vec4 my_ftransform(void);\n"
	"void main() { gl_Position = my_ftransform(); }";

static const char good_vs_text[] =
	"void main() { gl_Position = gl_Vertex; }";

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	enum piglit_result result = PIGLIT_PASS;
	GLenum err;
	GLint ok;
	GLuint prog;
	GLuint vs;

	piglit_require_gl_version(20);

	piglit_require_extension("GL_EXT_separate_shader_objects");

	printf("Trying shader with unresolved external symbol...\n");
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, bad_vs_text);
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glLinkProgram(prog);
	glDeleteShader(vs);

	ok = piglit_link_check_status_quiet(prog);
	if (ok) {
		fprintf(stderr,
			"Linking with unresolved symbol succeeded when it "
			"should have failed.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* There shouldn't be any GL errors, but clear them all just to be
	 * sure.
	 */
	while (glGetError() != 0)
		/* empty */ ;

	/* The specified program is not linked.  This should generate
	 * the error GL_INVALID_OPERATION.
	 */
	glUseShaderProgramEXT(GL_VERTEX_SHADER, prog);

	err = glGetError();
	if (err != GL_INVALID_OPERATION) {
		printf("Unexpected OpenGL error state 0x%04x for "
		       "glUseShaderProgramEXT called with\n"
		       "an unlinked shader program (expected 0x%04x).\n",
		       err, GL_INVALID_OPERATION);
		result = PIGLIT_FAIL;
	}

	glDeleteProgram(prog);
	glUseProgram(0);

	/* Try again with a shader program that could be linked but wasn't.
	 */
	printf("Trying unlinked, valid shader...\n");
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, good_vs_text);
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glDeleteShader(vs);

	/* There shouldn't be any GL errors, but clear them all just to be
	 * sure.
	 */
	while (glGetError() != 0)
		/* empty */ ;

	/* The specified program is not linked.  This should generate
	 * the error GL_INVALID_OPERATION.
	 */
	glUseShaderProgramEXT(GL_VERTEX_SHADER, prog);

	err = glGetError();
	if (err != GL_INVALID_OPERATION) {
		printf("Unexpected OpenGL error state 0x%04x for "
		       "glUseShaderProgramEXT called with\n"
		       "an unlinked shader program (expected 0x%04x).\n",
		       err, GL_INVALID_OPERATION);
		result = PIGLIT_FAIL;
	}

	glDeleteProgram(prog);
	glUseProgram(0);

	piglit_report_result(result);
}
