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
 */

/**
 * Simple test case framework.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "piglit-util.h"
#include "piglit-framework.h"
#ifdef USE_GLX
#include "piglit-glx-util.h"
#endif

int piglit_automatic = 0;
bool piglit_use_fbo = false;
GLuint piglit_fbo;
static int piglit_window;
static enum piglit_result result;
#ifdef USE_GLX
Display *piglit_glx_dpy;
Window piglit_glx_window;
XVisualInfo *piglit_glx_visinfo;
GLXContext piglit_glx_context;
#endif

static void
display(void)
{
	result = piglit_display();

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

static void
piglit_framework_glut_init(int argc, char *argv[])
{
	piglit_glutInit(argc, argv);

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(piglit_width, piglit_height);
	glutInitDisplayMode(piglit_window_mode);
	piglit_window = glutCreateWindow(argv[0]);

#ifdef USE_GLX
	if (piglit_automatic)
		piglit_glx_set_no_input();
#endif

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(piglit_escape_exit_key);

#ifdef USE_OPENGL
	glewInit();
#endif
}

#ifdef USE_GLX
static void
piglit_framework_fbo_glx_init()
{
	piglit_glx_dpy = piglit_get_glx_display();

	/* Unfortunately in GLX we need a drawable to bind our context
	 * to.  Make an unmapped window.
	 */
	piglit_glx_visinfo = piglit_get_glx_visual(piglit_glx_dpy);

	piglit_glx_context = piglit_get_glx_context(piglit_glx_dpy,
						    piglit_glx_visinfo);

	piglit_glx_window = piglit_get_glx_window_unmapped(piglit_glx_dpy,
							   piglit_glx_visinfo);

	glXMakeCurrent(piglit_glx_dpy, piglit_glx_window, piglit_glx_context);
}
#endif

static void
piglit_framework_fbo_glx_destroy()
{
#ifdef USE_GLX
	glXMakeCurrent(piglit_glx_dpy, None, None);
	glXDestroyContext(piglit_glx_dpy, piglit_glx_context);
	XFree(piglit_glx_visinfo);
	XCloseDisplay(piglit_glx_dpy);
#endif
}

static bool
piglit_framework_fbo_init()
{
	GLuint tex, depth = 0;
	GLenum status;

#ifdef USE_GLX
	piglit_framework_fbo_glx_init();
#else
	return false;
#endif

#ifdef USE_OPENGL
	glewInit();

	if (!GLEW_VERSION_2_0)
		return false;
#endif

	glGenFramebuffers(1, &piglit_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_fbo);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		     piglit_width, piglit_height, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER,
			       GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D,
			       tex,
			       0);

	if (piglit_window_mode & (GLUT_DEPTH | GLUT_STENCIL)) {
		GLenum depth_stencil;

#ifdef USE_OPENGL
		depth_stencil = GL_DEPTH_STENCIL;
#else
		depth_stencil = GL_DEPTH_STENCIL_OES;
#endif

		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, depth_stencil,
			     piglit_width, piglit_height, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER,
				       GL_DEPTH_ATTACHMENT,
				       GL_TEXTURE_2D,
				       depth,
				       0);
		glFramebufferTexture2D(GL_FRAMEBUFFER,
				       GL_STENCIL_ATTACHMENT,
				       GL_TEXTURE_2D,
				       depth,
				       0);
	}

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr,
			"-fbo resulted in incomplete FBO, falling back\n");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return false;
	}

	return true;
}

static void
piglit_framework_fbo_destroy()
{
	piglit_framework_fbo_glx_destroy();
}

static void
delete_arg(char *argv[], int argc, int arg)
{
	int i;

	for (i = arg + 1; i < argc; i++) {
		argv[i - 1] = argv[i];
	}
}

int main(int argc, char *argv[])
{
	int j;

	/* Find/remove "-auto" and "-fbo" from the argument vector.
	 */
	for (j = 1; j < argc; j++) {
		if (!strcmp(argv[j], "-auto")) {
			piglit_automatic = 1;
			delete_arg(argv, argc--, j--);
		} else if (!strcmp(argv[j], "-fbo")) {
			piglit_use_fbo = true;
			delete_arg(argv, argc--, j--);
		} else if (!strcmp(argv[j], "-rlimit")) {
			char *ptr;
			unsigned long lim;
			int i;

			j++;
			if (j >= argc) {
				fprintf(stderr,
					"-rlimit requires an argument\n");
				piglit_report_result(PIGLIT_FAIL);
			}

			lim = strtoul(argv[j], &ptr, 0);
			if (ptr == argv[j]) {
				fprintf(stderr,
					"-rlimit requires an argument\n");
				piglit_report_result(PIGLIT_FAIL);
			}

			piglit_set_rlimit(lim);

			/* Remove 2 arguments (hence the 'i - 2') from the
			 * command line.
			 */
			for (i = j + 1; i < argc; i++) {
				argv[i - 2] = argv[i];
			}
			argc -= 2;
			j -= 2;
		}
	}

	if (piglit_use_fbo) {
		if (!piglit_framework_fbo_init())
			piglit_use_fbo = false;
	}

	if (!piglit_use_fbo)
		piglit_framework_glut_init(argc, argv);

	piglit_init(argc, argv);

	if (piglit_use_fbo) {
		result = piglit_display();
		piglit_framework_fbo_destroy();
	} else {
		glutMainLoop();
	}

	piglit_report_result(result);
	/* UNREACHED */
	return 0;
}
