/**
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
 * \file
 * Test ClientWaitSync() returns correct error messages for invalid input
 *
 *
 * Section 5.2.1(Waiting for Sync Objects) of OpenGL 3.2 Core says:
 *
 *     "If <sync> is not the name of a sync object, an INVALID_VALUE error
 *      is generated. If <flags> contains any bits other than
 *      SYNC_FLUSH_COMMANDS_BIT, an INVALID_VALUE error is generated."
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLsync a = (GLsync)0xDEADBEEF;
	GLenum status;
	int i;

	if (piglit_get_gl_version() < 32) {
		piglit_require_extension("GL_ARB_sync");
	}

	/* sync not set up yet so this should fail with both GL error and
	 * respond GL_WAIT_FAILED
	 */
	status = glClientWaitSync(a, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
	if (status != GL_WAIT_FAILED) {
		printf("Expected GL_WAIT_FAILED but returned: %s\n",
			piglit_get_gl_enum_name(status));
		pass = false;
	}

	a = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	/* test that valid sync results in NO_ERROR */
	status = glClientWaitSync(a, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* test that invalid flag value results in INVALID_VALUE */
	for (i = 0; i < sizeof(GLbitfield) * 8; i++) {
		GLbitfield mask = 1 << i;
		/* Skip over the valid bit */
		if (mask == GL_SYNC_FLUSH_COMMANDS_BIT) {
			continue;
		}
		status = glClientWaitSync(a, mask, 0);
		pass = (status == GL_WAIT_FAILED) && pass;
		pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
	}

	glDeleteSync(a);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
