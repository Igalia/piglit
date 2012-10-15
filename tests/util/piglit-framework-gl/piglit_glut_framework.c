/*
 * Copyright 2012 Intel Corporation
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "piglit_glut_framework.h"
#include "piglit-util-gl-common.h"

struct piglit_glut_framework {
	struct piglit_gl_framework gl_fw;

	enum piglit_result result;
	int window;
};

/**
 * This global variable exists because GLUT's API requires that data be passed
 * to the display function via a global. Ugh, GLUT is such an awful API.
 */
static struct piglit_glut_framework glut_fw;

static void
destroy(struct piglit_gl_framework *gl_fw)
{
	piglit_gl_framework_teardown(gl_fw);

	glut_fw.result = 0;
	glut_fw.window = 0;
}

static void
display(void)
{
	const struct piglit_gl_test_config *test_config = glut_fw.gl_fw.test_config;

	if (test_config->display)
		glut_fw.result = test_config->display();

	if (piglit_automatic) {
		glutDestroyWindow(glut_fw.window);
#ifdef FREEGLUT
		/* Tell GLUT to clean up and exit, so that we can
		 * reasonably valgrind our testcases for memory
		 * leaks by the GL.
		 */
		glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,
			      GLUT_ACTION_GLUTMAINLOOP_RETURNS);
		glutLeaveMainLoop();
#else
		piglit_report_result(glut_fw.result);
#endif
	}
}

static void
default_reshape_func(int w, int h)
{
	if (piglit_automatic &&
	    (w != piglit_width ||
	     h != piglit_height)) {
		printf("Got spurious window resize in automatic run "
		       "(%d,%d to %d,%d)\n", piglit_width, piglit_height, w, h);
		piglit_report_result(PIGLIT_WARN);
	}

	piglit_width = w;
	piglit_height = h;

	glViewport(0, 0, w, h);
}

static void
init_glut(void)
{
	const struct piglit_gl_test_config *test_config = glut_fw.gl_fw.test_config;
	char *argv[] = {"piglit"};
	int argc = 1;

	glutInit(&argc, argv);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(test_config->window_width,
	                   test_config->window_height);
	glutInitDisplayMode(test_config->window_visual);
	glut_fw.window = glutCreateWindow("Piglit");

	glutDisplayFunc(display);
	glutReshapeFunc(default_reshape_func);
	glutKeyboardFunc(piglit_escape_exit_key);

#ifdef PIGLIT_USE_OPENGL
	piglit_dispatch_default_init();
#endif
}

static void
run_test(struct piglit_gl_framework *gl_fw,
         int argc, char *argv[])
{
	const struct piglit_gl_test_config *test_config = glut_fw.gl_fw.test_config;

	if (test_config->init)
		test_config->init(argc, argv);

	glutMainLoop();
	piglit_report_result(glut_fw.result);
}

static void
swap_buffers(struct piglit_gl_framework *gl_fw)
{
	glutSwapBuffers();
}

static void
post_redisplay(struct piglit_gl_framework *gl_fw)
{
	glutPostRedisplay();
}

static void
set_keyboard_func(struct piglit_gl_framework *gl_fw,
                  void (*func)(unsigned char key, int x, int y))
{
	glutKeyboardFunc(func);
}

static void
set_reshape_func(struct piglit_gl_framework *gl_fw,
                 void (*func)(int w, int h))
{
	glutReshapeFunc(func);
}

struct piglit_gl_framework*
piglit_glut_framework_create(const struct piglit_gl_test_config *test_config)
{
	bool ok = true;

	if (!test_config->supports_gl_compat_version) {
		printf("GLUT can create only GL compatibility contexts, "
			"which the test does not support running under.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	ok = piglit_gl_framework_init(&glut_fw.gl_fw, test_config);
	if (!ok)
		return NULL;

	init_glut();

	glut_fw.gl_fw.swap_buffers = swap_buffers;
	glut_fw.gl_fw.run_test = run_test;
	glut_fw.gl_fw.post_redisplay = post_redisplay;
	glut_fw.gl_fw.set_keyboard_func = set_keyboard_func;
	glut_fw.gl_fw.set_reshape_func = set_reshape_func;
	glut_fw.gl_fw.destroy = destroy;

	return &glut_fw.gl_fw;
}
