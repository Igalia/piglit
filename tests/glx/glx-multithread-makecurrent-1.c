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

/** @file glx-multithread-makecurrent-1.c
 *
 * First test of GLX_MESA_multithread_makecurrent: Bind one context into
 * multiple threads and make sure that synchronized rendering from both
 * threads works correctly.
 */

#include <unistd.h>
#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "pthread.h"

int piglit_width = 70, piglit_height = 30;
static Display *dpy;
static Window win;
static GLXContext ctx;
static pthread_mutex_t mutex;
static XVisualInfo *visinfo;
static int current_step = 1;

static void
get_lock_for_step(int step)
{
	while (true) {
		pthread_mutex_lock(&mutex);
		if (step == current_step) {
			current_step++;
			return;
		}
		pthread_mutex_unlock(&mutex);
		usleep(1);
	}
}

static void *
thread1_func(void *arg)
{
	get_lock_for_step(1);
	glXMakeCurrent(dpy, win, ctx);
	pthread_mutex_unlock(&mutex);

	get_lock_for_step(3);
	glColor4f(0.0, 1.0, 0.0, 1.0);
	piglit_draw_rect(10, 10, 10, 10);
	pthread_mutex_unlock(&mutex);

	get_lock_for_step(5);
	glXMakeCurrent(dpy, None, None);
	pthread_mutex_unlock(&mutex);

	return NULL;
}

static void *
thread2_func(void *arg)
{
	get_lock_for_step(2);
	glXMakeCurrent(dpy, win, ctx);
	pthread_mutex_unlock(&mutex);

	get_lock_for_step(4);
	glColor4f(0.0, 0.0, 1.0, 1.0);
	piglit_draw_rect(30, 10, 10, 10);
	pthread_mutex_unlock(&mutex);

	get_lock_for_step(6);
	glXMakeCurrent(dpy, None, None);
	pthread_mutex_unlock(&mutex);

	return NULL;
}

enum piglit_result
draw(Display *dpy)
{
	GLboolean pass = GL_TRUE;
	float green[] = {0.0, 1.0, 0.0, 1.0};
	float blue[] = {0.0, 0.0, 1.0, 1.0};
	float gray[] = {0.5, 0.5, 0.5, 1.0};
	pthread_t thread1, thread2;
	void *retval;
	int ret;
	int x1 = 10, x2 = 30;

	ctx = piglit_get_glx_context(dpy, visinfo);
	glXMakeCurrent(dpy, win, ctx);
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	piglit_require_glx_extension(dpy, "GLX_MESA_multithread_makecurrent");

	/* Clear background to gray */
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	pthread_mutex_init(&mutex, NULL);

	/* Now, spawn some threads that do some drawing, both with this
	 * context
	 */
	pthread_create(&thread1, NULL, thread1_func, &x1);
	pthread_create(&thread2, NULL, thread2_func, &x2);

	ret = pthread_join(thread1, &retval);
	assert(ret == 0);
	ret = pthread_join(thread2, &retval);
	assert(ret == 0);

	pthread_mutex_destroy(&mutex);

	glColor4f(0.0, 1.0, 0.0, 1.0);
	piglit_draw_rect(50, 10, 10, 10);

	pass &= piglit_probe_rect_rgba( 0, 10, 10, 10, gray);
	pass &= piglit_probe_rect_rgba(10, 10, 10, 10, green);
	pass &= piglit_probe_rect_rgba(20, 10, 10, 10, gray);
	pass &= piglit_probe_rect_rgba(30, 10, 10, 10, blue);
	pass &= piglit_probe_rect_rgba(40, 10, 10, 10, gray);
	pass &= piglit_probe_rect_rgba(50, 10, 10, 10, green);
	pass &= piglit_probe_rect_rgba(60, 10, 10, 10, gray);

	pass &= piglit_probe_rect_rgba(0, 0, piglit_width, 10, gray);
	pass &= piglit_probe_rect_rgba(0, 20, piglit_width, 10, gray);

	glXSwapBuffers(dpy, win);

	glXMakeCurrent(dpy, None, None);
	glXDestroyContext(dpy, ctx);

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

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	visinfo = piglit_get_glx_visual(dpy);
	win = piglit_get_glx_window(dpy, visinfo);

	XMapWindow(dpy, win);

	piglit_glx_event_loop(dpy, draw);
	
	XFree(visinfo);

	return 0;
}
