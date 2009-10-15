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

int piglit_automatic = 0;

static void
display(void)
{
	const enum piglit_result result = piglit_display();

	if (piglit_automatic)
		piglit_report_result(result);
}


int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	if (argc >= 2 && !strcmp(argv[1], "-auto"))
		piglit_automatic = 1;
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(piglit_width, piglit_height);
	glutInitDisplayMode(piglit_window_mode);
	glutCreateWindow(argv[0]);
	glutDisplayFunc(display);
	glutKeyboardFunc(piglit_escape_exit_key);

	glewInit();

	piglit_init(argc, argv);

	glutMainLoop();
	return 0;
}
