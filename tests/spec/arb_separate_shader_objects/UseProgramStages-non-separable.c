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
 * \file UseProgramStages-non-separable.c
 * Verify that a program w/o PROGRAM_SEPARABLE cannot be used with SSO
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *vs_code =
	"#version 110\n"
	"void main() { gl_Position = gl_Vertex; }\n"
	;

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLuint prog;
	GLuint pipeline;
	GLint param;
	bool pass = true;

	piglit_require_vertex_shader();
	piglit_require_extension("GL_ARB_separate_shader_objects");

	prog = piglit_build_simple_program(vs_code, NULL);

	/* Sanity check that GL_PROGRAM_SEPARABLE didn't magically get
	 * set for us.
	 */
	param = 0xDEADBEEF;
	glGetProgramiv(prog, GL_PROGRAM_SEPARABLE, &param);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	if (param == 0xDEADBEEF) {
		fprintf(stderr,
			"glGetProgramiv(GL_PROGRAM_SEPARABLE) didn't write "
			"a value.\n");
		pass = false;
	} else if (param != GL_FALSE) {
		fprintf(stderr,	"GL_PROGRAM_SEPARABLE is %d, should be 0.\n",
			param);
		pass = false;
	}

	/* Section 2.11.4 (Program Pipeline Objects) of the OpenGL 4.1
	 * spec says:
	 *
	 *     "If the program object named by program was linked without the
	 *     PROGRAM_SEPARABLE parameter set, or was not linked
	 *     successfully, the error INVALID_OPERATION is generated and the
	 *     corresponding shader stages in the pipeline program pipeline
	 *     object are not modified."
	 */
	glGenProgramPipelines(1, &pipeline);
	glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, prog);

	/* Verify that the error is generated...
	 */
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	/* ...and that the old binding is not modified.
	 */
	param = 0xDEADBEEF;
	glGetProgramPipelineiv(pipeline, GL_VERTEX_SHADER, &param);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	if (param == 0xDEADBEEF) {
		fprintf(stderr,
			"glGetProgramPipelineiv(GL_VERTEX_SHADER) didn't "
			"write a value.\n");
		pass = false;
	} else if (param != 0) {
		fprintf(stderr,	"GL_VERTEX_SHADER is %d, should be 0.\n",
			param);
		pass = false;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
