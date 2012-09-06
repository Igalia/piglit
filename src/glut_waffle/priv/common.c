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
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

struct glut_waffle_state _glut_waffle_state = {
	.display_mode = GLUT_RGB,
	.window_width = 300,
	.window_height = 300,
	.window_id_pool = 0,
};

struct glut_waffle_state *const _glut = &_glut_waffle_state;

void
glutFatal(char *format, ...)
{
	va_list args;

	va_start(args, format);

	fflush(stdout);
	fprintf(stderr, "glut_waffle: error: ");
	vfprintf(stderr, format, args);
	va_end(args);
	putc('\n', stderr);

	exit(1);
}

void
glutFatalWaffleError(const char *waffle_func)
{
	const struct waffle_error_info *info = waffle_error_get_info();
	const char *code = waffle_error_to_string(info->code);

	if (info->message_length > 0)
		glutFatal("%s() failed: %s: %s",
		          waffle_func, code, info->message);
	else
		glutFatal("%s() failed: %s",
		          waffle_func, code);
}
