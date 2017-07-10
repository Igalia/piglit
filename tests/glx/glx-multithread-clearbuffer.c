/*
 * Copyright (c) 2017 Advanced Micro Devices, Inc.
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
 * Create multiple GLX contexts and concurrently create, clear, and destroy
 * buffers and flush the context.
 *
 * This reproduces a deadlock with the radeonsi command submission thread
 * queue.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "pthread.h"

static pthread_mutex_t mutex;
static bool dispatch_ready = false;

static void *
thread_func(void *arg)
{
	Display *dpy;
	XVisualInfo *visinfo;
	Window win;
	unsigned i;

	dpy = piglit_get_glx_display();
	visinfo = piglit_get_glx_visual(dpy);
	win = piglit_get_glx_window(dpy, visinfo);

	GLXContext ctx;

	ctx = piglit_get_glx_context(dpy, visinfo);
	glXMakeCurrent(dpy, win, ctx);

	pthread_mutex_lock(&mutex);
	if (!dispatch_ready) {
		piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);
		dispatch_ready = true;
		piglit_require_gl_version(30);
		piglit_require_extension("GL_ARB_clear_buffer_object");
	}
	pthread_mutex_unlock(&mutex);

	for (i = 0; i < 1000; ++i) {
		GLuint buf;
		glGenBuffers(1, &buf);
		glBindBuffer(GL_ARRAY_BUFFER, buf);
		glBufferData(GL_ARRAY_BUFFER, 512, NULL, GL_STATIC_DRAW);
		glClearBufferSubData(GL_ARRAY_BUFFER, GL_R32UI, 0, 512, GL_RED_INTEGER, GL_UNSIGNED_INT, &buf);
		glDeleteBuffers(1, &buf);
		glFlush();
		piglit_check_gl_error(0);
	}

	glXDestroyContext(dpy, ctx);
	return NULL;
}

int
main(int argc, char **argv)
{
	/* Need at least 16 contexts to congest the thread queue. */
	pthread_t thread[16];

	XInitThreads();

	pthread_mutex_init(&mutex, NULL);

	for (int i = 0; i < ARRAY_SIZE(thread); i++)
		pthread_create(&thread[i], NULL, thread_func, NULL);

	for (int i = 0; i < ARRAY_SIZE(thread); i++)
		pthread_join(thread[i], NULL);

	pthread_mutex_destroy(&mutex);

	piglit_report_result(PIGLIT_PASS);
	return 0;
}
