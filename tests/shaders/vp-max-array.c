/*
 * Copyright © 2009 Intel Corporation
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
 * \file vp-max-array.c
 * Validate creation of a single maximally sized parameter array.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util.h"
#include "piglit-framework.h"

int piglit_window_mode = GLUT_RGB;
int piglit_width = 100;
int piglit_height = 100;


static const char vertex_source_template[] =
	"!!ARBvp1.0\n"
	"OPTION	ARB_position_invariant;\n"
	"PARAM	colors[%d] = { program.local[0..%d] };\n"
	"\n"
	"MOV	result.color, colors[0];\n"
	"END\n"
	;


enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}


bool
query_and_require_limit(GLenum pname, GLint *param, const char *name,
			GLint minimum_maximum)
{
	glGetProgramivARB(GL_VERTEX_PROGRAM_ARB, pname, param);
	if (*param < minimum_maximum) {
		fprintf(stderr, "%s: Expected at least %d, got %d\n",
			name, minimum_maximum, *param);
		return false;
	}

	return true;
}

void
piglit_init(int argc, char **argv)
{
	GLint max_parameters;
	GLint max_native_parameters;
	GLint max_local_parameters;
	GLint max_env_parameters;
	char shader_source[1024];
	bool pass = true;

	(void) argc;
	(void) argv;

	piglit_require_vertex_program();

	/* First, query all of the limits.
	 */
	pass = query_and_require_limit(GL_MAX_PROGRAM_PARAMETERS_ARB,
				       & max_parameters,
				       "GL_MAX_PROGRAM_PARAMETERS_ARB",
				       96)
		&& pass;
	pass = query_and_require_limit(GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB,
				       & max_native_parameters,
				       "GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB",
				       96)
		&& pass;
	pass = query_and_require_limit(GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB,
				       & max_local_parameters,
				       "GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB",
				       96)
		&& pass;
	pass = query_and_require_limit(GL_MAX_PROGRAM_ENV_PARAMETERS_ARB,
				       & max_env_parameters,
				       "GL_MAX_PROGRAM_ENV_PARAMETERS_ARB",
				       96)
		&& pass;
	if (!pass) {
		piglit_report_result(PIGLIT_FAIL);
	}

	snprintf(shader_source, sizeof(shader_source),
		 vertex_source_template,
		 max_parameters, max_parameters - 1);

	(void) piglit_compile_program(GL_VERTEX_PROGRAM_ARB, shader_source);
}
