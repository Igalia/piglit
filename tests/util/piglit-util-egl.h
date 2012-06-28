/*
 * Copyright (c) Intel 2011
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

#pragma once

#include <EGL/egl.h>

#include "piglit-util-gl-common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Convert an EGL error to a string.
 *
 * For example, given EGL_BAD_DRAWABLE, return "EGL_BAD_DRAWABLE".
 *
 * Return "(unrecognized error)" if the enum is not recognized.
 */
const char* piglit_get_egl_error_name(EGLint error);

/**
 * \brief Check for unexpected EGL errors and possibly terminate the test.
 *
 * If eglGetError() returns an error other than \c expected_error, then
 * print a diagnostic and terminate the test with the given result.
 *
 * If you expect no error, then set \code expected_error = EGL_SUCCESS \endcode.
 */
void piglit_expect_egl_error(EGLint expected_error, enum piglit_result result);

#ifdef __cplusplus
} /* end extern "C" */
#endif
