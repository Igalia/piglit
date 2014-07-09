/*
 * Copyright © 2013 Gregory Hainaut <gregory.hainaut@gmail.com>
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
 * \name IsProgramPipeline.c
 * Verify correct behavior of glIsProgramPipeline relative to when a pipeline
 * actually starts to exist.
 *
 * Also verify that glGenProgramPipelines and glDeleteProgramPipelines with
 * negative counts function correctly.
 *
 * Section 2.11.4 (Program Pipeline Objects) of the OpenGL 4.1 spec says:
 *
 *     "The command
 *
 *         void GenProgramPipelines( sizei n, uint *pipelines );
 *
 *     returns n previously unused program pipeline object names in
 *     pipelines. These names are marked as used, for the purposes of
 *     GenProgramPipelines only, but they acquire state only when they are
 *     first bound."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;
	config.supports_gl_core_version = 31;

PIGLIT_GL_TEST_CONFIG_END

static bool pass;

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

static void
IsProgramPipeline(GLuint pipe, GLboolean expected)
{
	const GLboolean status = glIsProgramPipeline(pipe);

	if (status != expected) {
		pass = false;

		fprintf(stderr,
			"Pipeline %d has wrong IsProgramPipeline. "
			"Expected %d, got %d\n",
			pipe, expected, status);
	}
}

void
piglit_init(int argc, char **argv)
{
	GLuint id[4];
	int i;
	GLint dummy;

	piglit_require_extension("GL_ARB_separate_shader_objects");

	pass = true;

	assert(glGetError() == GL_NO_ERROR);

	if (!piglit_automatic)
		printf("glGenProgramPipelines with negative n value\n");

	glGenProgramPipelines(-1, id);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	if (!piglit_automatic)
		printf("glGenProgramPipelines with correct n value\n");

	glGenProgramPipelines(4, id);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	for (i = 0; i < 4 ; i++) {
		IsProgramPipeline(id[i], GL_FALSE);
	}

	glBindProgramPipeline(id[0]);
	glUseProgramStages(id[1], GL_ALL_SHADER_BITS, 0);
	glActiveShaderProgram(id[2], 0);
	glGetProgramPipelineiv(id[3], GL_VERTEX_SHADER, &dummy);

	/* Flush any errors.  The goal is to check that objects acquire a
	 * state.
	 */
	piglit_reset_gl_error();

	for (i = 0; i < 4 ; i++) {
		IsProgramPipeline(id[i], GL_TRUE);
	}

	if (!piglit_automatic)
		printf("glDeleteProgramPipelines with negative n value\n");

	glDeleteProgramPipelines(-1, id);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	if (!piglit_automatic)
		printf("glDeleteProgramPipelines with correct n value\n");

	glDeleteProgramPipelines(4, id);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
