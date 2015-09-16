/*
 * Copyright (c) 2014 -2015 Intel Corporation
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

/** @file common.h
 *
 * Common utility functions for the ARB_compute_shader tests.
 */

#ifndef __PIGLIT_ARB_COMPUTE_SHADER_COMMON_H__
#define __PIGLIT_ARB_COMPUTE_SHADER_COMMON_H__

#include "piglit-util-gl.h"

/**
 * Concatenate a variable number of strings into a newly allocated
 * buffer.  Note that concat() assumes ownership of the provided
 * arguments and that the argument list must be NULL-terminated.
 */
char *
concat(char *hunk0, ...);

static inline char *
hunk(const char *s)
{
	return strdup(s);
}

GLuint
generate_cs_prog(unsigned x, unsigned y, unsigned z, char *ext,
		 char *src);

#endif
