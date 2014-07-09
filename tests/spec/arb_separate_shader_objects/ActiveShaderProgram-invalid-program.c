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
 * \name ActiveShaderProgram-invalid-program.c
 * Verify glActiveShaderProgram with invalid program parameter
 *
 * There are several cases outlined in the GL 4.4 spec where
 * glActiveShaderProgram should generate specific errors.  In addition,
 * section 2.3.1 (Errors) says:
 *
 *     "Currently, when an error flag is set, results of GL operation are
 *     undefined only if an OUT_OF_MEMORY error has occurred. In other cases,
 *     there are no side effects unless otherwise noted; the command which
 *     generates the error is ignored so that it has no effect on GL state or
 *     framebuffer contents."
 *
 * After calling glActiveShaderProgram with an invalid parameter, verify that
 * the active program state has not been modified.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;
	config.supports_gl_core_version = 31;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_code_template[] =
	"#version %u\n"
	"void main() { gl_Position = vec4(0); }\n"
	;

static const char *const invalid_code =
	"#version 123456789\n"
	"void main() { gl_Position = jambon_banh_mi(); }\n"
	;

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint pipe;
	GLuint vs_prog;
	GLuint active_prog;
	GLuint unlinked_prog;
	GLuint shader;
	unsigned glsl_version;
	bool es;
	int glsl_major;
	int glsl_minor;
	char *source;

	piglit_require_extension("GL_ARB_separate_shader_objects");

	piglit_get_glsl_version(&es, &glsl_major, &glsl_minor);
	glsl_version = ((glsl_major * 100) + glsl_minor) >= 140
		? 140 : ((glsl_major * 100) + glsl_minor);

	glGenProgramPipelines(1, &pipe);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBindProgramPipeline(pipe);

	asprintf(&source, vs_code_template, glsl_version);
	vs_prog = glCreateShaderProgramv(GL_VERTEX_SHADER, 1,
					 (const GLchar *const *) &source);
	piglit_link_check_status(vs_prog);

	/* First, make a valid program active.
	 */
	glActiveShaderProgram(pipe, vs_prog);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* Next, try to make an invalid program active and verify that the
	 * correct error is generated.  Also make sure the old program is
	 * still active.
	 *
	 * Section 7.4 (Program Pipeline Objects) under ActiveShaderProgram of
	 * the OpenGL 4.4 spec says:
	 *
	 *     "An INVALID_VALUE error is generated if program is not zero and
	 *     is not the name of either a program or shader object."
	 */
	glActiveShaderProgram(pipe, ~vs_prog);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	glGetProgramPipelineiv(pipe, GL_ACTIVE_PROGRAM, (GLint *) &active_prog);
	if (active_prog != vs_prog) {
		printf("glActiveShaderProgram with an invalid program name "
		       "changed the active program state.\n");
		pass = false;
	} else {
		glActiveShaderProgram(pipe, vs_prog);
	}

	/* Try the same thing with a valid shader object (that is not part of
	 * a linked program).  Verify that the correct error is generated, and
	 * make sure the old program is still active.
	 *
	 * Section 7.4 (Program Pipeline Objects) under ActiveShaderProgram of
	 * the OpenGL 4.4 spec says:
	 *
	 *     "An INVALID_OPERATION error is generated if program is the name
	 *     of a shader object."
	 */
	shader = piglit_compile_shader_text(GL_VERTEX_SHADER, source);
	glActiveShaderProgram(pipe, shader);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	glGetProgramPipelineiv(pipe, GL_ACTIVE_PROGRAM, (GLint *) &active_prog);
	if (active_prog != vs_prog) {
		printf("glActiveShaderProgram with a shader object "
		       "changed the active program state.\n");
		pass = false;
	} else {
		glActiveShaderProgram(pipe, vs_prog);
	}

	/* Finally, try the same thing with a valid program that is not
	 * linked.  Verify that the correct error is generated, and make sure
	 * the old program is still active.
	 *
	 * Section 7.4 (Program Pipeline Objects) under ActiveShaderProgram of
	 * the OpenGL 4.4 spec says:
	 *
	 *     "An INVALID_OPERATION error is generated if program is not zero
	 *     and has not been linked, or was last linked unsuccessfully."
	 */
	unlinked_prog = glCreateShaderProgramv(GL_VERTEX_SHADER, 1,
					       (const GLchar *const *) &invalid_code);

	glActiveShaderProgram(pipe, unlinked_prog);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	glGetProgramPipelineiv(pipe, GL_ACTIVE_PROGRAM, (GLint *) &active_prog);
	if (active_prog != vs_prog) {
		printf("glActiveShaderProgram with an unlinked program "
		       "changed the active program state.\n");
		pass = false;
	} else {
		glActiveShaderProgram(pipe, vs_prog);
	}


	free(source);
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
