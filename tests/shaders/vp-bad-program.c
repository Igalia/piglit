/*
 * Copyright © 2008 Intel Corporation
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
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/**
 * @file vp-bad-program.c
 *
 * Tests that the driver reports errors correctly (and doesn't crash) when
 * fed a bad vertex program.
 *
 * Wine likes to do that to us to see how strict we are on the VP language.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	static const char *badprog =
		"!!ARBvp1.0\n"
		"NOTANOPCODE;\n"
		"MOV result.position, vertex.position;\n";
	static const GLfloat vertcoords[4][3] = {
		{ -0.25, -0.25, 0 },
		{  0.25, -0.25, 0 },
		{  0.25,  0.25, 0 },
		{ -0.25,  0.25, 0 }
	};
	int failed = 0;
	GLenum err;

	/* Try the bad vertex program, and make sure we get an error */
	glProgramStringARB(GL_VERTEX_PROGRAM_ARB,
			   GL_PROGRAM_FORMAT_ASCII_ARB,
			   strlen(badprog),
			   (const GLubyte *) badprog);

	err = glGetError();
	if (err != GL_INVALID_OPERATION) {
		printf("Unexpected OpenGL error state %d with bad vertex "
		       "program.\n", err);
		printf("Expected: %d\n", GL_INVALID_OPERATION);
		failed++;

		while (err != 0)
			err = glGetError();
	}

	/* Check that we correctly produce GL_INVALID_OPERATION when rendering
	 * with an invalid/non-existant program.
	 */
        glBindProgramARB(GL_VERTEX_PROGRAM_ARB, 99);
        glEnable(GL_VERTEX_PROGRAM_ARB);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_POLYGON);
	glTexCoord2f(0, 0);  glVertex2f(-0.25, -0.25);
	glTexCoord2f(1, 0);  glVertex2f( 0.25, -0.25);
	glTexCoord2f(1, 1);  glVertex2f( 0.25,  0.25);
	glTexCoord2f(0, 1);  glVertex2f(-0.25,  0.25);
	glEnd();
	err = glGetError();

	if (err != GL_INVALID_OPERATION) {
		printf("Unexpected OpenGL error state %d in glBegin() "
		       "with bad vertex program.\n", err);
		printf("Expected: %d\n", GL_INVALID_OPERATION);
		failed++;

		while (err != 0)
			err = glGetError();
	}

	/* Check that we correctly produce GL_INVALID_OPERATION when doing
	 * glDrawArrays with an invalid/non-existant program.
	 */

	glVertexPointer(3, GL_FLOAT, 0, vertcoords);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_POLYGON, 0, 4);
	err = glGetError();
	glDisableClientState(GL_VERTEX_ARRAY);

	if (err != GL_INVALID_OPERATION) {
		printf("Unexpected OpenGL error state %d in glDrawArrays() "
		       "with bad vertex program.\n", err);
		printf("Expected: %d\n", GL_INVALID_OPERATION);
		failed++;

		while (err != 0)
			err = glGetError();
	}

	return failed ? PIGLIT_FAIL : PIGLIT_PASS;
}

void piglit_init(int argc, char **argv)
{
	piglit_automatic = GL_TRUE;

	piglit_require_vertex_program();
}
