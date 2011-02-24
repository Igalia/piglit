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
static int piglit_window;
static enum piglit_result result;

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

int main(int argc, char *argv[])
{
	int j;

	glutInit(&argc, argv);

	/* Find/remove "-auto" from the argument vector.
	 */
	for (j = 1; j < argc; j++) {
		if (!strcmp(argv[j], "-auto")) {
			int i;

			piglit_automatic = 1;
		
			for (i = j + 1; i < argc; i++) {
				argv[i - 1] = argv[i];
			}
			argc--;
			j--;
		}
	}
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

	glewInit();

	piglit_init(argc, argv);

	glutMainLoop();

	piglit_report_result(result);
	/* UNREACHED */
	return 0;
}
