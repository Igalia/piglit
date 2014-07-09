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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file glx-multithread.c
 *
 * Test that rendering two plain colored rectangles in two different threads
 * to the same GLX window works correctly.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "pthread.h"

int piglit_width = 50, piglit_height = 50;
static Display *dpy;
static Window win;
static pthread_mutex_t mutex;
static XVisualInfo *visinfo;

static void *
thread_func(void *arg)
{
	GLXContext ctx;
	int *x = arg;
	Bool ret;

	pthread_mutex_lock(&mutex);

	ctx = piglit_get_glx_context(dpy, visinfo);
	ret = glXMakeCurrent(dpy, win, ctx);
	assert(ret);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	glColor4f(0.0, 1.0, 0.0, 0.0);
	piglit_draw_rect(*x, 10, 10, 10);

	glFinish();
	glXDestroyContext(dpy, ctx);

	pthread_mutex_unlock(&mutex);

	return NULL;
}

enum piglit_result
draw(Display *dpy)
{
	GLboolean pass = GL_TRUE;
	float green[] = {0.0, 1.0, 0.0, 0.0};
	pthread_t thread1, thread2;
	void *retval;
	int ret;
	int x1 = 10, x2 = 30;
	GLXContext ctx;

	ctx = piglit_get_glx_context(dpy, visinfo);
	glXMakeCurrent(dpy, win, ctx);
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	/* Clear background to gray */
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glFinish();

	pthread_mutex_init(&mutex, NULL);
	/* Now, spawn some threads that do some drawing, both with this
	 * context
	 */
	pthread_create(&thread1, NULL, thread_func, &x1);
	pthread_create(&thread2, NULL, thread_func, &x2);

	ret = pthread_join(thread1, &retval);
	assert(ret == 0);
	ret = pthread_join(thread2, &retval);
	assert(ret == 0);

	pthread_mutex_destroy(&mutex);

	pass &= piglit_probe_pixel_rgb(15, 15, green);
	pass &= piglit_probe_pixel_rgb(35, 15, green);

	glXSwapBuffers(dpy, win);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

int
main(int argc, char **argv)
{
	int i;

	for(i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-auto"))
			piglit_automatic = 1;
		else
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
	}

	XInitThreads();
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	visinfo = piglit_get_glx_visual(dpy);
	win = piglit_get_glx_window(dpy, visinfo);

	XMapWindow(dpy, win);

	piglit_glx_event_loop(dpy, draw);

	return 0;
}
