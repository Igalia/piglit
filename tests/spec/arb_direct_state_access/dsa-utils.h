/*
 * Copyright 2014 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** @file dsa-utils.h
 *
 * Contains some common functionality for writing arb_direct_state_access
 * Piglit tests.
 *
 * @author Laura Ekstrand (laura@jlekstrand.net)
 */

#pragma once
#ifndef __DSA_UTILS_H__
#define __DSA_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "piglit-util-gl.h"

#define SUBTEST(error, global, format, args...) \
do { \
	bool local = piglit_check_gl_error((error)); \
	global = global && local; \
	piglit_report_subtest_result(local ? PIGLIT_PASS : PIGLIT_FAIL, \
	                             (format), ##args); \
} while (0)

#define SUBTESTCONDITION(condition, global, format, args...) \
do { \
	bool cond = (condition); \
	global = global && cond; \
	piglit_report_subtest_result(cond ? PIGLIT_PASS : PIGLIT_FAIL, \
	                             (format), ##args); \
} while (0)

void dsa_init_program(void);

void dsa_texture_with_unit(GLuint);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* __DSA_UTILS_H__ */
