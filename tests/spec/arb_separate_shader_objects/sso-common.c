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
 * \file sso-common.c
 * Utility functions used by multiple separate shader objects tests.
 */
#include "piglit-util-gl.h"
#include "sso-common.h"

/**
 * Pick a GLSL version that will work with explicit location layout qualifiers
 *
 * Some NVIDIA drivers have issues with layout qualifiers, 'in' keywords, and
 * 'out' keywords in "lower" GLSL versions.  If the driver supports GLSL >=
 * 1.40, use 1.40.  Otherwise, pick the highest version that the driver
 * supports.
 *
 * 1.40 is selected as the maximum version because core-profile contexts
 * aren't required to support versions earlier than 1.40.  Otherwise, 1.30
 * would also work.
 */
unsigned
pick_a_glsl_version(void)
{
	unsigned glsl_version;
	bool es;
	int glsl_major;
	int glsl_minor;

	piglit_get_glsl_version(&es, &glsl_major, &glsl_minor);
	glsl_version = ((glsl_major * 100) + glsl_minor) >= 140
		? 140 : ((glsl_major * 100) + glsl_minor);

	return glsl_version;
}

GLuint
format_and_link_program(GLenum type, const char* code, unsigned glsl_version)
{
	char *source;
	GLuint prog;

	asprintf(&source, code, glsl_version);
	prog = glCreateShaderProgramv(type, 1,
			(const GLchar *const *) &source);

	piglit_link_check_status(prog);
	free(source);

	return prog;
}

/**
 * Create a transform feedback object and some storage for the data
 *
 * \note
 * The XFB object will be bound on exit.  The buffer object for the XFB data
 * will be bound to the XFB object and the \c GL_TRANSFORM_FEEDBACK_BUFFER
 * binding on exit.
 */
void
configure_transform_feedback_object(GLuint *xfb, GLuint *buf)
{
	glGenBuffers(1, buf);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, *buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, 1024, NULL, GL_STREAM_READ);

	glGenTransformFeedbacks(1, xfb);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, *xfb);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, *buf);
}

/**
 * Create a separable vertex shader program with transform feedback output
 *
 * A vertex shader must be created using the "traditional" API because
 * \c glTransformFeedbackVaryings must be called before linking.  There is no
 * way to do that with \c glCreateShaderProgramv.
 */
bool
CreateShaderProgram_with_xfb(const char *source,
			     const char **varyings,
			     unsigned num_varyings,
			     GLuint *vs_prog)
{
	GLuint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, source);

	*vs_prog = glCreateProgram();
	glAttachShader(*vs_prog, vs);

	glProgramParameteri(*vs_prog, GL_PROGRAM_SEPARABLE, GL_TRUE);
	glTransformFeedbackVaryings(*vs_prog, num_varyings, varyings,
				    GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(*vs_prog);
	glDeleteShader(vs);

	return piglit_link_check_status(*vs_prog);
}
