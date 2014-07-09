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

#include "piglit-util-gl.h"
#include <X11/Xlib.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

/* In case eglext.h isn't new enough: */
#ifndef EGL_KHR_create_context
#define EGL_KHR_create_context 1
#define EGL_CONTEXT_MAJOR_VERSION_KHR			    EGL_CONTEXT_CLIENT_VERSION
#define EGL_CONTEXT_MINOR_VERSION_KHR			    0x30FB
#define EGL_CONTEXT_FLAGS_KHR				    0x30FC
#define EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR		    0x30FD
#define EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR  0x31BD
#define EGL_NO_RESET_NOTIFICATION_KHR			    0x31BE
#define EGL_LOSE_CONTEXT_ON_RESET_KHR			    0x31BF
#define EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR		    0x00000001
#define EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR	    0x00000002
#define EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR	    0x00000004
#define EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR		    0x00000001
#define EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR    0x00000002
#endif

/* Old versions of eglext.h may define EGL_KHR_create_context without defining
 * the EGL_OPENGL_ES3_BIT_KHR, because the bit was not defined until version
 * 13 of the extension.
 */
#ifndef EGL_OPENGL_ES3_BIT_KHR
#define EGL_OPENGL_ES3_BIT_KHR                              0x00000040
#endif

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
