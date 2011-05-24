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

#include "piglit-util.h"
#include "piglit-framework.h"

int piglit_window_mode = GLUT_RGB;
int piglit_width = 250, piglit_height = 250;

#define CHECK_GL_INVALID_VALUE if (glGetError() != GL_INVALID_VALUE) return PIGLIT_FAIL;

enum piglit_result piglit_display(void)
{
	int attribCount;

	GLfloat floatv[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat quad[] = { -1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0 };

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &attribCount);

	glVertexAttrib1f(attribCount, floatv[0]);
	CHECK_GL_INVALID_VALUE;

	glVertexAttrib2f(attribCount, floatv[0], floatv[1]);
	CHECK_GL_INVALID_VALUE;

	glVertexAttrib3f(attribCount, floatv[0], floatv[1], floatv[2]);
	CHECK_GL_INVALID_VALUE;

	glVertexAttrib4f(attribCount, floatv[0], floatv[1], floatv[2],
			 floatv[3]);
	CHECK_GL_INVALID_VALUE;

	glVertexAttrib1fv(attribCount, floatv);
	CHECK_GL_INVALID_VALUE;

	glVertexAttrib2fv(attribCount, floatv);
	CHECK_GL_INVALID_VALUE;

	glVertexAttrib3fv(attribCount, floatv);
	CHECK_GL_INVALID_VALUE;

	glVertexAttrib4fv(attribCount, floatv);
	CHECK_GL_INVALID_VALUE;

	glVertexAttribPointer(attribCount, 2, GL_FLOAT, GL_FALSE, 0, quad);
	CHECK_GL_INVALID_VALUE;

	return PIGLIT_PASS;
}

void piglit_init(int argc, char **argv)
{
	if (!GLEW_VERSION_2_0) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}
}
