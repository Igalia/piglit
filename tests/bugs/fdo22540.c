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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

// author: Ben Holmes

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 400;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static GLuint vBuffer;

static void
Init(void)
{
	piglit_require_extension("GL_ARB_vertex_buffer_object");
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 400, 0, 300, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

static void
vboInit(void)
{
	static const GLfloat vArray[12] = {
		225, 125, 0,
		225, 175, 0,
		175, 125, 0,
		175, 175, 0
	};
	glGenBuffersARB(1, &vBuffer);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vBuffer);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vArray),
			vArray, GL_STATIC_DRAW_ARB);

}

static GLboolean
vboMap(void)
{
	(void) glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_READ_WRITE_ARB);
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
	return (glGetError() == 0);
}

enum piglit_result
piglit_display(void)
{
	GLfloat gray[3] = {0.5, 0.5, 0.5};
	GLboolean pass;

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vBuffer);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0 ,0);

	glColor3f(0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	pass = vboMap();
	pass = pass && piglit_probe_pixel_rgb(200, 150, gray);

	glFinish();
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char  **argv)
{
	Init();
	vboInit();
}
