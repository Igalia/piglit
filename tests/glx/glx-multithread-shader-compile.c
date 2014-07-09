/*
 * Copyright (c) 2012 Mathias Fröhlich
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
 * @file glx-multithread-shader-compile.c
 * @author Mathias Fröhlich
 *
 * Create two GLX contexts and concurrently compile shaders.
 * Exercises a race conditon with the r600 llvm compiler.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "pthread.h"

static const char *vert_shader_text =
   "void main() \n"
   "{ \n"
   "   gl_Position = ftransform(); \n"
   "   gl_FrontColor = gl_Color; \n"
   "} \n";

static const char *frag_shader_text =
   "void main() \n"
   "{ \n"
   "   gl_FragColor = vec4(1.0) - gl_Color; \n"
   "} \n";

static pthread_mutex_t mutex;

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

	for (i = 0; i < 100; ++i) {
		GLXContext ctx;
		GLuint vert_shader, frag_shader;
		GLuint program;

		ctx = piglit_get_glx_context(dpy, visinfo);
		glXMakeCurrent(dpy, win, ctx);

		/* Ok, not nice but should be safe due to all threads working
		 * on the same type of context.
                 */
		pthread_mutex_lock(&mutex);
		piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);
		pthread_mutex_unlock(&mutex);

		vert_shader = piglit_compile_shader_text(GL_VERTEX_SHADER, vert_shader_text);
		piglit_check_gl_error(GL_NO_ERROR);

		frag_shader = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag_shader_text);
		piglit_check_gl_error(GL_NO_ERROR);

		program = piglit_link_simple_program(vert_shader, frag_shader);
		piglit_check_gl_error(GL_NO_ERROR);

		glUseProgram(program);
		piglit_check_gl_error(GL_NO_ERROR);

		glXDestroyContext(dpy, ctx);
	}

	return NULL;
}

int
main(int argc, char **argv)
{
	int ret;
	pthread_t thread1, thread2;

	XInitThreads();

	pthread_mutex_init(&mutex, NULL);

	/* Now, spawn some threads that compile simple shaders.
	 */
	pthread_create(&thread1, NULL, thread_func, NULL);
	pthread_create(&thread2, NULL, thread_func, NULL);

	ret = pthread_join(thread1, NULL);
	assert(ret == 0);
	ret = pthread_join(thread2, NULL);
	assert(ret == 0);

	pthread_mutex_destroy(&mutex);

	piglit_report_result(PIGLIT_PASS);
	return 0;
}
