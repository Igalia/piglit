/* Copyright © 2012 Intel Corporation
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

#include "piglit-util-gl-common.h"
#include <X11/Xlib.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

extern EGLDisplay egl_dpy;
extern EGLConfig cfg;
extern EGLContext ctx;

extern bool parse_version_string(const char *string, int *major, int *minor);
extern bool EGL_KHR_create_context_setup(EGLint renderable_type_mask);
extern void EGL_KHR_create_context_teardown(void);

static inline bool
version_is_valid_for_context(int ctx_major, int major, int minor)
{
	if (ctx_major == 1) {
		if (major == 1 && (minor == 0 || minor == 1)) {
			return true;
		}
	} else if (ctx_major == 2) {
		/* GLES 3.0 is backward compatible with 2.0 and is the only version
		 * currently available that is compatible with 2.0.
		 */
		if ((major == 2 || major == 3) && minor == 0) {
			return true;
		}
	}
	return false;
}
