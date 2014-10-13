/*
 * Copyright © 2014 Intel Corporation
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
 *    Neil Roberts <neil@linux.intel.com>
 *
 */

/** @file glx-context-flush-control.c
 *
 * Tests the GLX_ARB_context_flush_control extension. It takes the
 * following steps using two threads. The threads are only used so it
 * can operate on another context without having to rebind it. The
 * threads are run lock-step so that each step is run sequentially.
 *
 * Thread 1: Make a flushless context A
 * Thread 1: Make a flushy context B, shared with A
 * Thread 1: Make a flushy context C, shared with A
 * Thread 1: Bind context A
 * Thread 2: Bind context C
 * Thread 1: Make a renderbuffer.
 * Thread 1: glClear() it to green.
 * Thread 1: glFinish()
 * Thread 1: glClear() it to red.
 * Thread 2: Do a glReadPixels()
 *
 * (At this point the GL implementation is allowed to have finished
 * the clear to red but it probably won't have. If the read pixels
 * returns green here then it's not a failure but the test won't work
 * so it will report PIGLIT_SKIP)
 *
 * Thread 1: Bind context C
 * Thread 1: sleep(.5)
 * Thread 2: Make sure glReadPixels() is still green, otherwise fail.
 *
 * All of the steps are then run again but this time context A is made
 * flushy and the last step ensures that the pixel becomes red instead
 * of green. If it did become red then the GL successfully made a
 * flush when context A was released.
 *
 * The test also verifies that calling glGetIntegerv with
 * GL_CONTEXT_RELEASE_BEHAVIOR returns the expected value when setting
 * the attribute to none and flush and also when the attribute is left
 * out entirely.
 */

#include <unistd.h>

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "pthread.h"

#ifndef GLX_ARB_context_flush_control
#define GLX_CONTEXT_RELEASE_BEHAVIOR_ARB 0x2097
#define GLX_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB 0
#define GLX_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB 0x2098
#endif

#ifndef GL_CONTEXT_RELEASE_BEHAVIOR
#define GL_CONTEXT_RELEASE_BEHAVIOR 0x82FB
#define GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH 0x82FC
#endif

static PFNGLXCREATECONTEXTATTRIBSARBPROC CreateContextAttribs = NULL;

enum release_behavior {
	RB_NONE,
	RB_FLUSH,
	RB_NOT_SPECIFIED
};

struct window {
	GLXFBConfig config;
	XVisualInfo *xvi;
	Window window;
	GLXWindow glx_window;
};

struct thread_data {
	Display *display;
	struct window *window;
	GLXContext context;

	pthread_mutex_t mutex;
	pthread_cond_t cond;

	bool quit;
	void (* running_func)(struct thread_data *data);

	GLuint fbo;

	enum piglit_result result;
};

static float red[] = { 1.0f, 0.0f, 0.0f };
static float green[] = { 0.0f, 1.0f, 0.0f };

GLXContext
create_context(Display *display,
	       struct window *window,
	       GLXContext share_ctx,
	       enum release_behavior release_behavior)
{
	GLint actual_release_behavior;
	GLXContext ctx;
	int ctx_attribs[7] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 1,
		GLX_CONTEXT_MINOR_VERSION_ARB, 5,
		GLX_CONTEXT_RELEASE_BEHAVIOR_ARB,
		release_behavior,
		0
	};

	switch (release_behavior) {
	case RB_NONE:
		ctx_attribs[5] = GLX_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB;
		break;
	case RB_FLUSH:
		ctx_attribs[5] = GLX_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB;
		break;
	case RB_NOT_SPECIFIED:
		ctx_attribs[4] = 0;
		break;
	}

	ctx = CreateContextAttribs(display, window->config,
				   share_ctx, True,
				   ctx_attribs);

	assert(ctx);

	glXMakeCurrent(display, window->glx_window, ctx);

	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	piglit_require_extension("GL_KHR_context_flush_control");

	glGetIntegerv(GL_CONTEXT_RELEASE_BEHAVIOR, &actual_release_behavior);

	switch (release_behavior) {
	case RB_FLUSH:
	case RB_NOT_SPECIFIED:
		assert(actual_release_behavior ==
		       GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH);
		       break;
	case RB_NONE:
		assert(actual_release_behavior == GL_NONE);
		       break;
	default:
		assert(0);
	}

	return ctx;
}

static void
create_window(Display *display,
	      struct window *window)
{
	window->xvi = piglit_get_glx_visual(display);
	window->config =
		piglit_glx_get_fbconfig_for_visinfo(display, window->xvi);

	window->window = piglit_get_glx_window(display, window->xvi);
	window->glx_window = glXCreateWindow(display, window->config,
					     window->window, NULL);
}

static void *
thread_func(void *user_data)
{
	struct thread_data *data = user_data;
	bool quit;
	void (* running_func)(struct thread_data *data);

	while (true) {
		/* Wait for something to do */
		pthread_mutex_lock(&data->mutex);
		while (!data->quit && data->running_func == NULL)
			pthread_cond_wait(&data->cond, &data->mutex);
		quit = data->quit;
		running_func = data->running_func;
		pthread_mutex_unlock(&data->mutex);

		if (quit)
			break;

		running_func(data);

		pthread_mutex_lock(&data->mutex);
		data->running_func = NULL;
		pthread_cond_signal(&data->cond);
		pthread_mutex_unlock(&data->mutex);
	}

	return NULL;
}

static void
run_in_thread(struct thread_data *data,
	      void (* func)(struct thread_data *data))
{
	pthread_mutex_lock(&data->mutex);

	/* Tell the thread about the function */
	data->running_func = func;
	pthread_cond_signal(&data->cond);

	/* Wait for it to complete */
	do
		pthread_cond_wait(&data->cond, &data->mutex);
	while (data->running_func);

	pthread_mutex_unlock(&data->mutex);

	if (data->result != PIGLIT_PASS)
		piglit_report_result(data->result);
}

static void
bind_context(struct thread_data *data)
{
	glXMakeCurrent(data->display, data->window->glx_window, data->context);
}

static void
unbind_context(struct thread_data *data)
{
	glXMakeCurrent(data->display, None, NULL);
}

static void
check_green(struct thread_data *data)
{
	glBindFramebuffer(GL_FRAMEBUFFER, data->fbo);

	/* At this point the main thread has flushed a clear to green
	 * and queued a clear to red without flushing. It would be
	 * valid for the framebuffer to be red here but in that case
	 * the test won't work so we will skip the test */
	if (!piglit_probe_pixel_rgb_silent(0, 0, green, NULL)) {
		printf("Either the clear to green command was not completed "
		       "or the clear to red command was flushed too early so "
		       "the test will be skipped\n");
		data->result = PIGLIT_SKIP;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void
check_still_green(struct thread_data *data)
{
	float probe[4];

	glBindFramebuffer(GL_FRAMEBUFFER, data->fbo);

	/* The pixel should still be green even though the main thread
	 * has released the original context because it shouldn't
	 * cause a flush */
	if (!piglit_probe_pixel_rgb_silent(0, 0, green, NULL)) {
		if (piglit_probe_pixel_rgb_silent(0, 0, red, probe)) {
			printf("The renderbuffer contains a red pixel which "
			       "means that releasing the first context has "
			       "caused a flush.\n");
		} else {
			printf("Expected green\n"
			       "Observed: %f %f %f\n",
			       probe[0], probe[1], probe[2]);
		}

		data->result = PIGLIT_FAIL;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void
check_changed_to_red(struct thread_data *data)
{
	float probe[4];

	glBindFramebuffer(GL_FRAMEBUFFER, data->fbo);

	/* Releasing the original context should have caused a flush
	 * so the framebuffer should have become red */
	if (!piglit_probe_pixel_rgb_silent(0, 0, red, NULL)) {
		if (piglit_probe_pixel_rgb_silent(0, 0, green, probe)) {
			printf("The renderbuffer contains a green pixel which "
			       "means that releasing the first context has not "
			       "caused a flush.\n");
		} else {
			printf("Expected red\n"
			       "Observed: %f %f %f\n",
			       probe[0], probe[1], probe[2]);
		}

		data->result = PIGLIT_FAIL;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void
do_test_flush(Display *display,
	      struct window *window,
	      GLXContext context_b,
	      struct thread_data *thread_data,
	      enum release_behavior release_behavior)
{
	GLenum status;
	GLuint fbo, rb;

	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, 1, 1);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,
				  GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER,
				  rb);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("failed to create a 1x1 GL_RGB renderbuffer\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	thread_data->fbo = fbo;

	/* Clear the framebuffer to green */
	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	/* Make sure the color actually hits the framebuffer */
	glFinish();

	/* Post a command to clear it to red without flushing */
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	run_in_thread(thread_data, check_green);

	/* Switch to the other context. This shouldn't cause a flush
	 * if the release behavior is RB_NONE */
	glXMakeCurrent(display, window->glx_window, context_b);

	/* Give the GPU some time to finish rendering */
	usleep(500000);

	if (release_behavior == RB_NONE) {
		/* Verify that it didn't cause a flush */
		run_in_thread(thread_data, check_still_green);
	} else {
		/* Make sure it did cause a flush */
		run_in_thread(thread_data, check_changed_to_red);
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteRenderbuffers(1, &rb);
	glDeleteFramebuffers(1, &fbo);
}

static void
test_flush(Display *display,
	   struct window *window,
	   enum release_behavior release_behavior)
{
	GLXContext context_a, context_b, context_c;
	struct thread_data thread_data;
	pthread_t thread;

	/* Create three contexts with each of the three possible
	 * values for the release behavior. This also verifies that
	 * the GL extension returns the right value for each attribute
	 * value from glGetIntegerv. We only need one context without
	 * flushing and the value for the others doesn't really
	 * matter */
	context_a = create_context(display, window, NULL, release_behavior);
	piglit_require_extension("GL_EXT_framebuffer_object");
	context_b = create_context(display, window, context_a, RB_FLUSH);
	piglit_require_extension("GL_EXT_framebuffer_object");
	context_c = create_context(display, window,
				   context_a, RB_NOT_SPECIFIED);
	piglit_require_extension("GL_EXT_framebuffer_object");

	thread_data.display = display;
	thread_data.window = window;
	thread_data.context = context_c;

	pthread_mutex_init(&thread_data.mutex, NULL);
	pthread_cond_init(&thread_data.cond, NULL);

	thread_data.quit = false;
	thread_data.running_func = NULL;
	thread_data.result = PIGLIT_PASS;

	pthread_create(&thread, NULL, thread_func, &thread_data);

	glXMakeCurrent(display, window->glx_window, context_a);

	run_in_thread(&thread_data, bind_context);

	do_test_flush(display, window, context_b,
		      &thread_data, release_behavior);

	run_in_thread(&thread_data, unbind_context);

	pthread_mutex_lock(&thread_data.mutex);
	thread_data.quit = true;
	pthread_cond_signal(&thread_data.cond);
	pthread_mutex_unlock(&thread_data.mutex);

	pthread_join(thread, NULL);

	pthread_cond_destroy(&thread_data.cond);
	pthread_mutex_destroy(&thread_data.mutex);

	glXDestroyContext(display, context_c);
	glXDestroyContext(display, context_b);
	glXDestroyContext(display, context_a);
}

static void
destroy_window(Display *display,
	       struct window *window)
{
	glXDestroyWindow(display, window->glx_window);
	XDestroyWindow(display, window->window);
}

int
main(int argc, char **argv)
{
	Display *display;
	struct window window;
	bool pass = true;

	display = piglit_get_glx_display();

	piglit_require_glx_extension(display, "GLX_ARB_get_proc_address");
	piglit_require_glx_extension(display, "GLX_ARB_create_context");
	piglit_require_glx_extension(display, "GLX_ARB_context_flush_control");

	CreateContextAttribs = (PFNGLXCREATECONTEXTATTRIBSARBPROC)
		glXGetProcAddressARB((GLubyte *) "glXCreateContextAttribsARB");

	create_window(display, &window);

	test_flush(display, &window, RB_NONE);
	test_flush(display, &window, RB_FLUSH);

	destroy_window(display, &window);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);

	return 0;
}
