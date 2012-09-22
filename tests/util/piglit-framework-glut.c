/*
 * Copyright 2009-2012 Intel Corporation
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "piglit-util-gl-common.h"
#include "piglit-framework-gl.h"
#include "piglit-framework-glut.h"

#ifdef PIGLIT_USE_GLX
#include "piglit-glx-util.h"
#endif

/**
 * \brief Set by piglit_framework_glut_init().
 *
 * This global variable exists because GLUT's API requires that data be passed
 * to the display function via a global. Ugh, what an awful API.
 */
static const struct piglit_gl_test_info *test_info;

static int piglit_window;
static enum piglit_result result;


static void
display(void)
{
	result = test_info->display();

	if (piglit_automatic) {
		glutDestroyWindow(piglit_window);
#ifdef FREEGLUT
		/* Tell GLUT to clean up and exit, so that we can
		 * reasonably valgrind our testcases for memory
		 * leaks by the GL.
		 */
		glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,
			      GLUT_ACTION_GLUTMAINLOOP_RETURNS);
		glutLeaveMainLoop();
#else
		piglit_report_result(result);
#endif
	}
}

static void
reshape(int w, int h)
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

/* Swapbuffers the results to the window in non-auto mode. */
void
piglit_present_results()
{
	if (!piglit_automatic && !piglit_use_fbo)
		glutSwapBuffers();
}

void
piglit_framework_glut_init(int argc, char *argv[],
			   const struct piglit_gl_test_info *info)
{
	if (test_info != NULL)
		assert(!"already init");

	test_info = info;
	glutInit(&argc, argv);

#	if defined(PIGLIT_USE_WAFFLE)
#		if defined(PIGLIT_USE_OPENGL)
			glutInitAPIMask(GLUT_OPENGL_BIT);
#		elif defined(PIGLIT_USE_OPENGL_ES1)
			glutInitAPIMask(GLUT_OPENGL_ES1_BIT);
#		elif defined(PIGLIT_USE_OPENGL_ES2)
			glutInitAPIMask(GLUT_OPENGL_ES2_BIT);
#		else
#			error
#		endif
#	endif

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(info->window_width,
			info->window_height);
	glutInitDisplayMode(info->window_visual);
	piglit_window = glutCreateWindow(argv[0]);

#if defined(PIGLIT_USE_GLX) && !defined(PIGLIT_USE_WAFFLE)
	/* If using waffle, then the current platform might not be GLX.
	 * So we can't call any GLX functions.
	 *
	 * FIXME: Detect the waffle platform and handle piglit_automatic
	 * FIXME: appropriately.
	 */
	if (piglit_automatic)
		piglit_glx_set_no_input();
#endif

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(piglit_escape_exit_key);

#ifdef PIGLIT_USE_OPENGL
	glewInit();
#endif
}

void
piglit_framework_glut_run(const struct piglit_gl_test_info *info)
{
	glutMainLoop();
	piglit_report_result(result);
}

void
piglit_framework_glut_swap_buffers(void)
{
	glutSwapBuffers();
}
