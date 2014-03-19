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

#define EGL_EGLEXT_PROTOTYPES

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "piglit-util.h"

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
 * \brief Check for unexpected EGL errors.
 *
 * If eglGetError() returns an error other than \c expected_error, then
 * print a diagnostic and return false.
 *
 * If you expect no error, then set \code expected_error = EGL_SUCCESS \endcode.
 */
bool
piglit_check_egl_error(EGLint expected_error);

/**
 * \brief Get default display for given platform.
 *
 * If \a platform is EGL_NONE, the this function wraps eglGetDisplay().
 * Otherwise, it wraps eglGetPlatformDisplayEXT().
 *
 * On failure, return EGL_NO_DISPLAY.
 *
 * If EGL does not support the platform extension for the given \a platform,
 * then return EGL_NO_DISPLAY.
 */
EGLDisplay
piglit_egl_get_default_display(EGLenum platform);

/**
 * \brief Checks whether an EGL extension is supported.
 */
bool piglit_is_egl_extension_supported(EGLDisplay egl_dpy, const char *name);

/**
 * \brief Checks for EGL extension and skips if not supported
 */
void piglit_require_egl_extension(EGLDisplay dpy, const char *name);

/**
 * \brief Wrapper for eglBindAPI().
 *
 * Return true if eglBindAPI succeeds. Return false if eglBindAPI fails
 * because the EGL implementation does not support the API; in most cases,
 * the caller should then report SKIP.
 *
 * If eglBindAPI fails for unexpected reasons, then the test fails.
 */
bool
piglit_egl_bind_api(EGLenum api);

#ifdef __cplusplus
} /* end extern "C" */
#endif
