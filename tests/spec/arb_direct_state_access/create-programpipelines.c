/*
 * Copyright 2015 Intel Corporation
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

/** @file create-programpipelines.c
 *
 * Tests glCreateProgramPipelines to see if it behaves in the expected way,
 * throwing the correct errors, etc.
 *
 * From OpenGL 4.5, section 7.4 "Program Pipeline Objects", page 116:
 *
 * "void CreateProgramPipelines( sizei n, uint *pipelines );
 *
 * CreateProgramPipelinesreturns n previously unused program pipeline names
 * in pipelines, each representing a new program pipeline object which is a
 * state vector comprising all the state and with the same initial values
 * listed in table 23.31.
 *
 * Errors
 * An INVALID_VALUE error is generated if n is negative."
 */

#include "piglit-util-gl.h"
#include "dsa-utils.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
		PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_direct_state_access");
	piglit_require_extension("GL_ARB_separate_shader_objects");
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	GLchar label[11];
	GLsizei length;
	GLuint ids[10];
	GLint param;

	/* Throw some invalid inputs at glCreateProgramPipelines */

	/* n is negative */
	glCreateProgramPipelines(-1, ids);
	SUBTEST(GL_INVALID_VALUE, pass, "n < 0");

	/* Throw some valid inputs at glCreateProgramPipelines. */

	/* n is zero */
	glCreateProgramPipelines(0, NULL);
	SUBTEST(GL_NO_ERROR, pass, "n == 0");

	/* n is more than 1 */
	glCreateProgramPipelines(10, ids);
	SUBTEST(GL_NO_ERROR, pass, "n > 1");

	/* test the default state of dsa-created program pipeline objects */
	SUBTESTCONDITION(glIsProgramPipeline(ids[2]), pass,
			 "IsProgramPipeline()");

	glGetProgramPipelineiv(ids[2], GL_ACTIVE_PROGRAM, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == 0, pass,
			 "default active program(%d) == 0", param);

	glGetProgramPipelineiv(ids[2], GL_VERTEX_SHADER, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == 0, pass,
			 "default vertex shader program(%d) == 0", param);

	glGetProgramPipelineiv(ids[2], GL_GEOMETRY_SHADER, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == 0, pass,
			 "default geometry shader program(%d) == 0", param);

	glGetProgramPipelineiv(ids[2], GL_FRAGMENT_SHADER, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == 0, pass,
			 "default fragment shader program(%d) == 0", param);

	if (piglit_is_extension_supported("GL_ARB_compute_shader")) {
		glGetProgramPipelineiv(ids[2], GL_COMPUTE_SHADER, &param);
		piglit_check_gl_error(GL_NO_ERROR);
		SUBTESTCONDITION(param == 0, pass,
				 "default compute shader program(%d) == 0",
				 param);
	} else {
		piglit_report_subtest_result(PIGLIT_SKIP,
					 "default compute shader program == 0");
	}

	if (piglit_is_extension_supported("GL_ARB_tessellation_shader")) {
		glGetProgramPipelineiv(ids[2], GL_TESS_CONTROL_SHADER, &param);
		piglit_check_gl_error(GL_NO_ERROR);
		SUBTESTCONDITION(param == 0, pass,
				 "default TCS(%d) == 0", param);

		glGetProgramPipelineiv(ids[2], GL_TESS_EVALUATION_SHADER,
				       &param);
		piglit_check_gl_error(GL_NO_ERROR);
		SUBTESTCONDITION(param == 0, pass,
				 "default TES(%d) == 0", param);
	} else {
		piglit_report_subtest_result(PIGLIT_SKIP,
					 "default TCS == 0");

		piglit_report_subtest_result(PIGLIT_SKIP,
					 "default TES == 0");
	}

	glGetProgramPipelineiv(ids[2], GL_VALIDATE_STATUS, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == GL_FALSE, pass,
			 "default validate status(%d) == FALSE", param);

	glGetProgramPipelineiv(ids[2], GL_INFO_LOG_LENGTH, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == 0, pass,
			 "startup log length(%d) == 0", param);

	glGetObjectLabel(GL_PROGRAM_PIPELINE, ids[2], 11, &length, label);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(length == 0, pass,
			 "default label size(%d) == 0", length);

	glDeleteProgramPipelines(10, ids);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
