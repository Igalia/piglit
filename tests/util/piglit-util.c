/*
 * Copyright (c) The Piglit project 2007
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "GL/glut.h"

#include "piglit-util.h"

/** Returns the line in the program string given the character position. */
int FindLine(const char *program, int position)
{
	int i, line = 1;
	for (i = 0; i < position; i++) {
		if (program[i] == '\n')
			line++;
	}
	return line;
}

void
piglit_report_result(enum piglit_result result)
{
	/* Currently we have no way of reporting the "skip" (required extension
	 * not supported) result.
	 */

	if (result == PIGLIT_SUCCESS) {
		printf("PIGLIT: {'result': 'pass' }\n");
		exit(0);
	} else {
		printf("PIGLIT: {'result': 'fail' }\n");
		exit(1);
	}
}

void piglit_require_extension(const char *name)
{
	if (!glutExtensionSupported(name)) {
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}
}

/**
 * Read a pixel from the given location and compare its RGBA value to the
 * given expected values.
 *
 * Print a log message if the color value deviates from the expected value.
 * \return true if the color values match, false otherwise
 */
int piglit_probe_pixel_rgba(int x, int y, const float* expected)
{
	GLfloat probe[4];
	GLfloat delta[4];
	GLfloat deltamax;
	int i;

	glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, probe);

	deltamax = 0.0;
	for(i = 0; i < 4; ++i) {
		delta[i] = probe[i] - expected[i];
		if (fabs(delta[i]) > deltamax)
			deltamax = fabs(delta[i]);
	}

	if (deltamax < 0.01)
		return 1;

	printf("Probe at (%i,%i)\n", x, y);
	printf("  Expected: %f %f %f %f\n", expected[0], expected[1], expected[2], expected[3]);
	printf("  Observed: %f %f %f %f\n", probe[0], probe[1], probe[2], probe[3]);

	return 0;
}
