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
#include "piglit-framework-fbo.h"
#include "piglit-framework-glut.h"

bool piglit_use_fbo = false;
int piglit_automatic = 0;
unsigned piglit_winsys_fbo = 0;

static enum piglit_result result;

#ifndef _WIN32
__attribute__((weak)) int piglit_width = 100;
__attribute__((weak)) int piglit_height = 100;
__attribute__((weak)) int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

__attribute__((weak)) enum piglit_result piglit_display(void)
{
	return PIGLIT_FAIL;
}
__attribute__((weak)) void piglit_init(int argc, char **argv)
{
}
#endif

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
