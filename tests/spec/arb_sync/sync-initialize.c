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
 * Test that a sync is initialized correctly with FenceSync
 *
 * Section 5.2(Sync Objects and Fences) of OpenGL 3.2 Core says:
 * 	"Table 5.1: Initial properties of a sync object created with FenceSync."
 *
 * 	 Property Name	Property Value
 * 	--------------------------------------
 * 	 OBJECT_TYPE		SYNC_FENCE
 * 	 SYNC_CONDITION	<condition>
 * 	 SYNC_STATUS		UNSIGNALED
 * 	 SYNC_FLAGS		<flags>
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
	GLsizei length = -5;
	GLint value;
	GLsync sync;

	if (piglit_get_gl_version() < 32) {
		piglit_require_extension("GL_ARB_sync");
	}

	/* Create a new fence sync */
	sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	/* Test initialized as fence type */
	glGetSynciv(sync, GL_OBJECT_TYPE, 1, &length, &value);
	if (length != 1) {
		printf("length should be 1 but incorrectly returned: %d\n",
			length);
		pass = false;
	}
	if (value != GL_SYNC_FENCE) {
		printf("Expected GL_SYNC_FENCE but returned: %s\n",
			piglit_get_gl_enum_name(value));
		pass = false;
	}

	/* Test initialized to given condition */
	length = -5;
	glGetSynciv(sync, GL_SYNC_CONDITION, 1, &length, &value);
	if (length != 1) {
		printf("length should be 1 but incorrectly returned: %d\n",
			length);
		pass = false;
	}
	if (value != GL_SYNC_GPU_COMMANDS_COMPLETE) {
		printf("Expected GL_SYNC_GPU_COMMANDS_COMPLETE but returned: %s\n",
			piglit_get_gl_enum_name(value));
		pass = false;
	}

	/* Test initialized to unsignaled */
	length = -5;
	glGetSynciv(sync, GL_SYNC_STATUS, 1, &length, &value);
	if (length != 1) {
		printf("length should be 1 but incorrectly returned: %d\n",
			length);
		pass = false;
	}
	/* We can't test for just GL_UNSIGNALED here, since the driver
	 * may have actually completed any previous rendering (or, in
	 * our case, no rendering at all) already.
	 */
	if (value != GL_UNSIGNALED && value != GL_SIGNALED) {
		printf("Expected GL_UNSIGNALED or GL_SIGNALED but returned: %s\n",
			piglit_get_gl_enum_name(value));
		pass = false;
	}

	/* Test initialized with given flag */
	length = -5;
	glGetSynciv(sync, GL_SYNC_FLAGS, 1, &length, &value);
	if (length != 1) {
		printf("length should be 1 but incorrectly returned: %d\n",
			length);
		pass = false;
	}
	if (value != 0) {
		printf("Expected GL_SYNC_FLAGS == 0 but returned: %d\n",
			value);
		pass = false;
	}

	glDeleteSync(sync);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
