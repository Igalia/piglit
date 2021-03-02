/*
 * Copyright (c) 2021 Advanced Micro Devices, Inc.
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

/* Simple test to reproduce this buffer reference counting issue:
 *    https://gitlab.freedesktop.org/mesa/mesa/-/issues/4259
 */

#include <unistd.h>

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_width = 50, piglit_height = 50;

int
main(int argc, char **argv)
{
	piglit_automatic = 1;

	XInitThreads();
	Display *dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	XVisualInfo *visinfo = piglit_get_glx_visual(dpy);
	Window win = piglit_get_glx_window(dpy, visinfo);

	GLXContext ctx1 = piglit_get_glx_context_share(dpy, visinfo, NULL);
	GLXContext ctx2 = piglit_get_glx_context_share(dpy, visinfo, ctx1);

	glXMakeCurrent(dpy, win, ctx1);
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	piglit_require_extension("GL_ARB_uniform_buffer_object");

	GLuint buf[2];
	glCreateBuffers(2, buf);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, buf[0]);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, buf[1]);

	glXMakeCurrent(dpy, win, ctx2);
	glDeleteBuffers(1, &buf[1]);

	glXMakeCurrent(dpy, win, ctx1);
	glDeleteBuffers(1, &buf[0]); /* This shouldn't crash. */

	glXDestroyContext(dpy, ctx1);
	glXDestroyContext(dpy, ctx2);

	piglit_report_result(PIGLIT_PASS);
	return 0;
}
