/*
 * Copyright © 2009-2011 Intel Corporation
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

/** @file glx-multi-context-ib.c
 *
 * Tests that multiple contexts drawing using index buffers work correctly.
 *
 * Catches a bug in the i965 driver in which index buffer state was
 * not reemitted across batchbuffer boundaries, if the first draw
 * after the batch didn't use the IB.
 */

#include <unistd.h>
#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_width = 50, piglit_height = 50;
static Display *dpy;
static Window win;
static GLXContext ctx0, ctx1;

GLuint vb_c0, vb_c1, ib_c0, ib_c1;
static float green[] = {0, 1, 0, 0};
static float red[]   = {1, 0, 0, 0};

/* c0 sets up an IB that will mess up c1's drawing, by only indexing
 * outside of c0's VBO.
 */
static void
context0_init()
{
	float vb_data[] = {
		0, 0,
		0, 0,
		0, 0,
		0, 0,
		-1, -1,
		 1, -1,
		 1,  1,
		-1,  1,
	};
	uint32_t ib_data[] = {
		4, 5, 6, 7,
	};

	glGenBuffersARB(1, &vb_c0);
	glGenBuffersARB(1, &ib_c0);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vb_c0);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ib_c0);

	glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vb_data), vb_data,
			GL_STATIC_DRAW);
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(ib_data), ib_data,
			GL_STATIC_DRAW);
}

static void
context1_init()
{
	float vb_data[] = {
		-1, -1,
		 1, -1,
		 1,  1,
		-1,  1,
	};
	uint32_t ib_data[] = {
		0, 1, 2, 3,
	};

	glGenBuffersARB(1, &vb_c1);
	glGenBuffersARB(1, &ib_c1);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vb_c1);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ib_c1);

	glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vb_data), vb_data,
			GL_STATIC_DRAW);
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(ib_data), ib_data,
			GL_STATIC_DRAW);
}

static void
context0_frame(void)
{
	glXMakeCurrent(dpy, win, ctx0);

	glColor4fv(red);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vb_c0);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ib_c0);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, (void *)0);

	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, (void *)0);
}

static void
context1_frame(int iter)
{
	glXMakeCurrent(dpy, win, ctx1);

	/* This is the drawing without an IB that triggered the driver
	 * not reemitting IB state in the next draw call.
	 *
	 * The other context just exists to ensure that the race for
	 * the IB getting smashed is lost, and is also the thing that
	 * produces the glFlush()-based batchbuffer emits we rely on.
	 */
	glColor4fv(red);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	/* Failing draw call. */
	if (iter == 1)
		glColor4fv(green);
	else
		glColor4fv(red);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vb_c0);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ib_c0);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, (void *)0);

	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, (void *)0);
}

enum piglit_result
draw(Display *dpy)
{
	bool pass;

	context0_frame();
	context1_frame(0);
	/* The issue was that on the second frame, failure occurred. */
	context0_frame();
	context1_frame(1);

	pass = piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height, green);

	glXSwapBuffers(dpy, win);

	glXMakeCurrent(dpy, None, None);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

int
main(int argc, char **argv)
{
	XVisualInfo *visinfo;
	int i;

	for(i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-auto"))
			piglit_automatic = 1;
		else
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
	}

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	visinfo = piglit_get_glx_visual(dpy);
	win = piglit_get_glx_window(dpy, visinfo);

	XMapWindow(dpy, win);

	ctx0 = piglit_get_glx_context(dpy, visinfo);
	ctx1 = piglit_get_glx_context(dpy, visinfo);

	glXMakeCurrent(dpy, win, ctx0);
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);
	piglit_require_extension("GL_ARB_vertex_buffer_object");
	context0_init();
	glXMakeCurrent(dpy, win, ctx1);
	context1_init();

	piglit_glx_event_loop(dpy, draw);

	XFree(visinfo);
	glXDestroyContext(dpy, ctx0);
	glXDestroyContext(dpy, ctx1);

	return 0;
}
