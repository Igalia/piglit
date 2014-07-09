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
 *     "If <sync> is signaled at the time ClientWaitSync is called
 *      then ClientWaitSync returns immediately. If <sync> is
 *      unsignaled at the time ClientWaitSync is called then
 *      ClientWaitSync will block and will wait up to <timeout>
 *      nanoseconds for <sync> to become signaled.
 *
 *      ...
 *
 *      ALREADY_SIGNALED will always be returned if <sync> was
 *      signaled, even if the value of <timeout> is zero."
 *
 * There was concern that the implementation of the kernel API on i965
 * might violate this for the specific case of back-to-back
 * ClientWaitSyncs, but Mesa core doesn't end up calling into the
 * driver on a later ClientWaitSync.
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

	piglit_require_extension("GL_ARB_sync");

	glClear(GL_COLOR_BUFFER_BIT);

	sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	ret1 = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, ONE_SECOND);
	ret2 = glClientWaitSync(sync, 0, ONE_SECOND);

	if (ret1 == GL_TIMEOUT_EXPIRED) {
		printf("timeout expired on the first wait\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	if (ret2 != GL_ALREADY_SIGNALED) {
		fprintf(stderr,
			"Expected GL_ALREADY_SIGNALED on second wait, got %s",
			piglit_get_gl_enum_name(ret2));
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}
