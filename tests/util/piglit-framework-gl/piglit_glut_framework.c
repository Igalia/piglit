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
#include "piglit-util-gl.h"

/* Don't warn about using deprecated GLUT functions. */
#ifdef __APPLE__
#	pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

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
error_func(const char *fmt, va_list ap)
{
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	piglit_report_result(PIGLIT_SKIP);
}

static void
init_glut(void)
{
	const struct piglit_gl_test_config *test_config = glut_fw.gl_fw.test_config;
	char *argv[] = {"piglit"};
	int argc = 1;
	unsigned flags = GLUT_RGB;

	if (test_config->window_visual & PIGLIT_GL_VISUAL_RGBA)
		flags |= GLUT_ALPHA;
	if (test_config->window_visual & PIGLIT_GL_VISUAL_DEPTH)
		flags |= GLUT_DEPTH;
	if (test_config->window_visual & PIGLIT_GL_VISUAL_STENCIL)
		flags |= GLUT_STENCIL;
	if (test_config->window_visual & PIGLIT_GL_VISUAL_ACCUM)
		flags |= GLUT_ACCUM;

	if (test_config->window_visual & PIGLIT_GL_VISUAL_DOUBLE)
		flags |= GLUT_DOUBLE;
	else
		flags |= GLUT_SINGLE;

	glutInit(&argc, argv);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(test_config->window_width,
	                   test_config->window_height);
	glutInitDisplayMode(flags);

#ifdef PIGLIT_USE_GLUT_INIT_ERROR_FUNC
	glutInitErrorFunc(error_func);
#else
	(void)error_func;
#endif
#ifdef GLUT_CORE_PROFILE
	if (test_config->supports_gl_core_version) {
		glutInitContextVersion(test_config->supports_gl_core_version / 10,
				       test_config->supports_gl_core_version % 10);
		if (test_config->supports_gl_core_version >= 32) {
			glutInitContextProfile(GLUT_CORE_PROFILE);
		}
	} else {
		glutInitContextVersion(test_config->supports_gl_compat_version / 10,
				       test_config->supports_gl_compat_version % 10);
		if (test_config->supports_gl_compat_version >= 32) {
			glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
		}
	}

	int context_flags = 0;
	/* There are no 3.1 core profiles -- the closest is 3.1 context without
	 * ARB_compatibility or a 3.2 core context --, and setting
	 * forward-compatible flag should ensure we don't get a 3.1 context w/
	 * ARB_compatibility.
	 */
	if (test_config->require_forward_compatible_context ||
	    test_config->supports_gl_core_version == 31) {
		context_flags |= GLUT_FORWARD_COMPATIBLE;
	}
	if (test_config->require_debug_context) {
		context_flags |= GLUT_DEBUG;
	}
	if (context_flags) {
		glutInitContextFlags(context_flags);
	}
#endif

	glut_fw.window = glutCreateWindow("Piglit");

	glutDisplayFunc(display);
	glutReshapeFunc(default_reshape_func);
	glutKeyboardFunc(piglit_escape_exit_key);

#ifdef PIGLIT_USE_OPENGL
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);
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

/**
 * Check that the context's actual version is no less than the
 * requested version.
 */
static bool
check_gl_version(const struct piglit_gl_test_config *test_config)
{
	int actual_version = piglit_get_gl_version();

	if (test_config->supports_gl_core_version) {
		if (actual_version < test_config->supports_gl_core_version) {
			printf("Test requires GL version %d.%d, but actual version is "
			       "%d.%d\n",
			       test_config->supports_gl_core_version / 10,
			       test_config->supports_gl_core_version % 10,
			       actual_version / 10,
			       actual_version % 10);
			return false;
		}
		return true;
	}

	if (actual_version < test_config->supports_gl_compat_version) {
		printf("Test requires GL version %d.%d, but actual version is "
		       "%d.%d\n",
		       test_config->supports_gl_compat_version / 10,
		       test_config->supports_gl_compat_version % 10,
		       actual_version / 10,
		       actual_version % 10);
		return false;
	}

	if (piglit_is_core_profile &&
	    test_config->supports_gl_core_version == 0) {
		/* We have a core profile context but the test needs a
		 * compat profile.  We can't run the test.
		 */
		printf("Test requires compat version %d.%d or later but "
		       "context is core profile %d.%d.\n",
		       test_config->supports_gl_compat_version / 10,
		       test_config->supports_gl_compat_version % 10,
		       actual_version / 10,
		       actual_version % 10);
		return false;
	}

	return true;
}

struct piglit_gl_framework*
piglit_glut_framework_create(const struct piglit_gl_test_config *test_config)
{
	bool ok = true;

#ifndef GLUT_CORE_PROFILE
	if (!test_config->supports_gl_compat_version) {
		printf("GLUT can create only GL compatibility contexts, "
			"which the test does not support running under.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
#endif

	if (test_config->window_samples > 1) {
		printf("GLUT doesn't support MSAA visuals.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	ok = piglit_gl_framework_init(&glut_fw.gl_fw, test_config);
	if (!ok)
		return NULL;

	init_glut();

        /* Check if we actually have a core profile */
	{
		int actual_version = piglit_get_gl_version();
		if (actual_version >= 31 &&
		    !piglit_is_extension_supported("GL_ARB_compatibility"))
			piglit_is_core_profile = true;
	}

	if (!check_gl_version(test_config))
		piglit_report_result(PIGLIT_SKIP);

	glut_fw.gl_fw.swap_buffers = swap_buffers;
	glut_fw.gl_fw.run_test = run_test;
	glut_fw.gl_fw.post_redisplay = post_redisplay;
	glut_fw.gl_fw.set_keyboard_func = set_keyboard_func;
	glut_fw.gl_fw.set_reshape_func = set_reshape_func;
	glut_fw.gl_fw.destroy = destroy;

	return &glut_fw.gl_fw;
}
