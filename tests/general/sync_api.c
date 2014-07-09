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
 *
 * Authors:
 *    Ian Romanick <ian.d.romanick@intel.com>
 */

/**
 * \file sync_api.c
 * Simple test of the API for GL_ARB_sync.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 400;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

#define FAIL_ON_ERROR(string)						\
	do {								\
		const GLenum err = glGetError();			\
		if (err != GL_NO_ERROR) {				\
			fprintf(stderr, "%s generated error 0x%04x\n", 	\
				string, err);				\
			pass = GL_FALSE;				\
			goto done;					\
		}							\
	} while (0)

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_sync");

	glClearColor(0.1, 0.1, 0.3, 0.0);
	piglit_gen_ortho_projection(-1.0, 1.0, -1.0, 1.0, -0.5, 1000.0, GL_FALSE);
}

GLboolean
test_GetSynciv(GLsync sync, GLenum pname, GLint expect)
{
	GLboolean pass = GL_TRUE;
	GLint val;
	GLsizei len;

	glGetSynciv(sync, pname, 1, & len, & val);
	FAIL_ON_ERROR("glGetSynciv");
	if (len != 1) {
		fprintf(stderr, "glGetSynciv length of 0x%04x was %d\n",
			pname, len);
		pass = GL_FALSE;
	} else if (val != expect) {
		fprintf(stderr, "glGetSynciv of 0x%04x expected 0x%08x, "
			"got 0x%08x\n", pname, expect, val);
		pass = GL_FALSE;
	}

done:
	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLenum wait_val;
	GLsync sync;

	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_TRIANGLES);
	glColor3f(.8,0,0);
	glVertex3f(-0.9, -0.9, -30.0);
	glColor3f(0,.9,0);
	glVertex3f( 0.9, -0.9, -30.0);
	glColor3f(0,0,.7);
	glVertex3f( 0.0,  0.9, -30.0);
	glEnd();

	glGetError();

	sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	FAIL_ON_ERROR("glFenceSync");

	if (!glIsSync(sync)) {
		fprintf(stderr, "IsSync(%p) failed\n", sync);
		pass = GL_FALSE;
		goto done;
	}
	FAIL_ON_ERROR("glIsSync");

	if (! test_GetSynciv(sync, GL_SYNC_CONDITION,
			     GL_SYNC_GPU_COMMANDS_COMPLETE)) {
		pass = GL_FALSE;
		goto done;
	}

	if (! test_GetSynciv(sync, GL_SYNC_FLAGS, 0)) {
		pass = GL_FALSE;
		goto done;
	}

	glFinish();

	/* After the glFinish, the sync *must* be signaled!
	 */
	if (! test_GetSynciv(sync, GL_SYNC_STATUS, GL_SIGNALED)) {
		pass = GL_FALSE;
		goto done;
	}


	/* Since the sync has already been signaled, the wait should return
	 * GL_ALREADY_SIGNALED.
	 */
	wait_val = glClientWaitSync(sync, 0, 1);
	FAIL_ON_ERROR("glClientWaitSync");

	if (wait_val != GL_ALREADY_SIGNALED) {
		fprintf(stderr, "glClientWaitSync expected 0x%08x, "
			"got 0x%08x\n", GL_ALREADY_SIGNALED, wait_val);
		pass = GL_FALSE;
	}

	glDeleteSync(sync);
	FAIL_ON_ERROR("glDeleteSync");

done:
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

