/* Copyright © 2012 Intel Corporation
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

/** @file repeat-wait.c
 *
 * From the GL_ARB_sync spec:
 *
 *     "ALREADY_SIGNALED will always be returned if <sync> was
 *      signaled, even if the value of <timeout> is zero
 *
 *      ...
 *
 *      If the value of <timeout> is zero, then ClientWaitSync does
 *      not block, but simply tests the current state of
 *      <sync>. TIMEOUT_EXPIRED will be returned in this case if
 *      <sync> is not signaled, even though no actual wait was
 *      performed."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 10;
	config.window_height = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define ONE_SECOND 1000000

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLsync sync;
	GLenum ret1, ret2;
	bool pass = true;


	piglit_require_extension("GL_ARB_sync");

	glClear(GL_COLOR_BUFFER_BIT);
	sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	ret1 = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
	glFinish();
	ret2 = glClientWaitSync(sync, 0, 0);

	glDeleteSync(sync);

	if (ret1 != GL_TIMEOUT_EXPIRED &&
	    ret1 != GL_ALREADY_SIGNALED) {
		fprintf(stderr,
			"On first wait:\n"
			"  Expected GL_ALREADY_SIGNALED or GL_TIMEOUT_EXPIRED\n"
			"  Got %s\n",
			piglit_get_gl_enum_name(ret1));
		pass = false;
	}

	if (ret2 != GL_ALREADY_SIGNALED) {
		fprintf(stderr,
			"On repeated wait:\n"
			"  Expected GL_ALREADY_SIGNALED\n"
			"  Got %s\n",
			piglit_get_gl_enum_name(ret2));
		pass = false;
	}

	glClear(GL_COLOR_BUFFER_BIT);
	sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	glFinish();
	ret1 = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);

	if (ret1 != GL_ALREADY_SIGNALED) {
		fprintf(stderr,
			"On wait after a finish:\n"
			"  Expected GL_ALREADY_SIGNALED\n"
			"  Got %s\n",
			piglit_get_gl_enum_name(ret1));
		pass = false;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
