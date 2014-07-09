/* Copyright © 2011 Intel Corporation
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

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "common.h"

int main(int argc, char **argv)
{
	bool pass = true;
	pid_t child;
	int status;

	GLX_EXT_import_context_setup();
	get_context_IDs();

	child = fork();
	if (child == 0) {
		GLXContext ctx;
		Window win;

		GLX_EXT_import_context_setup_for_child();

		win = piglit_get_glx_window(dpy, visinfo);

		ctx = glXImportContextEXT(dpy, indirectID);
		glXMakeCurrent(dpy, win, ctx);

		XSync(dpy, 0);
		pass = validate_glx_error_code(Success, -1);

		exit(pass ? 0 : 1);
	}

	/* The test passes if the child exited normally with a return value of
	 * zero.
	 */
	waitpid(child, &status, 0);
	pass = WIFEXITED(status) && (WEXITSTATUS(status) == 0);

	GLX_EXT_import_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
