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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

/**
 * \file glsl-max-vertex-attrib.c
 * Test setting vertex attrib value of GL_MAX_VERTEX_ATTRIBS attrib
 *
 * Queries the value for GL_MAX_VERTEX_ATTRIBS and uses that as index
 * to set a value. GL specification states that GL_INVALID_VALUE should
 * occur if index >= GL_MAX_VERTEX_ATTRIBS.
 *
 * \author Sun Yi <yi.sun@intel.com>
 * \author Tapani Pälli <tapani.palli@intel.com>
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 250;
	config.window_height = 250;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static int test = 0;

#define CHECK_GL_INVALID_VALUE \
	if (glGetError() != GL_INVALID_VALUE) return PIGLIT_FAIL; \
	else printf("glsl-max-vertex-attrib test %d passed\n", ++test);

static const char *vShaderString =
	"attribute float pos;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = pos;\n"
	"}\n";

static const char *fShaderString =
	"void main()\n"
	"{\n"
	"	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
	"}\n";

enum piglit_result piglit_display(void)
{
	GLvoid *datap;
	GLint intv[] = { 1, 1, 1, 1 };
	GLfloat floatv[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat quad[] = { -1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0 };

	GLsizei length;
	GLint size;
	GLenum type;
	GLchar buffer[64];

	GLuint program, vShader, fShader;
	int maxAttribCount;

	/* --- valid program needed for some of the functions --- */

	fShader = glCreateShader(GL_FRAGMENT_SHADER);
	vShader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(fShader, 1, &fShaderString, NULL);
	glShaderSource(vShader, 1, &vShaderString, NULL);

	glCompileShader(vShader);
	glCompileShader(fShader);

	program = glCreateProgram();

	glAttachShader(program, vShader);
	glAttachShader(program, fShader);

	glLinkProgram(program);

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribCount);

	/* --- tests begin here --- */

	glVertexAttrib1f(maxAttribCount, floatv[0]);
	CHECK_GL_INVALID_VALUE;

	glVertexAttrib2f(maxAttribCount, floatv[0], floatv[1]);
	CHECK_GL_INVALID_VALUE;

	glVertexAttrib3f(maxAttribCount, floatv[0], floatv[1], floatv[2]);
	CHECK_GL_INVALID_VALUE;

	glVertexAttrib4f(maxAttribCount, floatv[0], floatv[1], floatv[2],
			 floatv[3]);
	CHECK_GL_INVALID_VALUE;

	glVertexAttrib1fv(maxAttribCount, floatv);
	CHECK_GL_INVALID_VALUE;

	glVertexAttrib2fv(maxAttribCount, floatv);
	CHECK_GL_INVALID_VALUE;

	glVertexAttrib3fv(maxAttribCount, floatv);
	CHECK_GL_INVALID_VALUE;

	glVertexAttrib4fv(maxAttribCount, floatv);
	CHECK_GL_INVALID_VALUE;

	glVertexAttribPointer(maxAttribCount, 2, GL_FLOAT, GL_FALSE, 0, quad);
	CHECK_GL_INVALID_VALUE;

	glBindAttribLocation(program, maxAttribCount, "pos");
	CHECK_GL_INVALID_VALUE;

	glEnableVertexAttribArray(maxAttribCount);
	CHECK_GL_INVALID_VALUE;

	glDisableVertexAttribArray(maxAttribCount);
	CHECK_GL_INVALID_VALUE;

	glGetVertexAttribfv(maxAttribCount, GL_CURRENT_VERTEX_ATTRIB, floatv);
	CHECK_GL_INVALID_VALUE;

	glGetVertexAttribiv(maxAttribCount, GL_CURRENT_VERTEX_ATTRIB, intv);
	CHECK_GL_INVALID_VALUE;

	glGetVertexAttribPointerv(maxAttribCount, GL_VERTEX_ATTRIB_ARRAY_POINTER, &datap);
	CHECK_GL_INVALID_VALUE;

	glGetActiveAttrib(program, maxAttribCount, 64, &length, &size, &type, buffer);
	CHECK_GL_INVALID_VALUE;

	return PIGLIT_PASS;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(20);
}
