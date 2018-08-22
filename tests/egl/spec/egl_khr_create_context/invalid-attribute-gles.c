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
#include "piglit-util-egl.h"
#include "common.h"

static bool try_attribute(int attribute)
{
	/* If the attribute is EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR
	 * or EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, use a valid value for that
	 * attribute.  This ensures that the attribute is rejected for the
	 * correct reason that GLES doesn't support profiles.
	 */
	const EGLint attribs[] = {
		attribute,
		(attribute == EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR)
		? EGL_NO_RESET_NOTIFICATION_KHR :
			(attribute == EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR)
			? EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR : 0,
		EGL_NONE
	};
	bool pass = true;

	ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT, attribs);
	if (ctx != NULL) {
		fprintf(stderr,
			"Created GLES context with invalid attribute "
			"0x%08x, but this should have failed.\n",
			attribute);
		eglDestroyContext(egl_dpy, ctx);
		pass = false;
	}

	/* The EGL_KHR_create_context spec says:
	 *
	 *     "* If an attribute name or attribute value in <attrib_list> is not
	 *        recognized (including unrecognized bits in bitmask attributes),
	 *        then an EGL_BAD_ATTRIBUTE error is generated."
	 */
	if (!piglit_check_egl_error(EGL_BAD_ATTRIBUTE))
		piglit_report_result(PIGLIT_FAIL);

	return pass;
}

int main(int argc, char **argv)
{
	/* The EGL_KHR_create_context spec says:
	 *
	 *    "The value for attribute EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR
	 *    specifies a <profile> of the OpenGL API. This attribute is only
	 *    meaningful for OpenGL contexts, and specifying it for other types of
	 *    contexts, including OpenGL ES contexts, will generate an error."
	 *
	 * and
	 *
	 *    "The attribute name
	 *    EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR specifies the
	 *    <reset notification behavior> of the rendering context. This
	 *    attribute is only meaningful for OpenGL contexts, and specifying it
	 *    for other types of contexts, including OpenGL ES contexts, will
	 *    generate an error."
	 *
	 * However, after making the extension part of core EGL 1.5,
	 * EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR was made a valid
	 * attribute for OpenGL ES contexts:
	 *
	 *    "The attribute EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY
	 *    specifies reset notification behavior for a context supporting
	 *    robust buffer access.  The attribute value may be either
	 *    EGL_NO_RESET_NOTIFICATION or EGL_LOSE_CONTEXT_ON_RESET, which
	 *    respectively result in reset notification behavior of
	 *    GL_NO_RESET_NOTIFICATION_ARB and GL_LOSE_CONTEXT_ON_RESET_ARB, as
	 *    described by the OpenGL GL_ARB_robustness extension, or by
	 *    equivalent functionality.
	 *
	 *    This attribute is supported only for OpenGL and OpenGL ES
	 *    contexts."
	 */
	EGLint bad_attributes[] = {
		0xffff0000,
		EGL_SAMPLE_BUFFERS,
		EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR,
		EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR
	};
	bool pass = true;
	unsigned i;

	if (!EGL_KHR_create_context_setup(EGL_OPENGL_ES_BIT)) {
		if (!EGL_KHR_create_context_setup(EGL_OPENGL_ES2_BIT)) {
			fprintf(stderr, "ES 2 not available.\n");
			piglit_report_result(PIGLIT_SKIP);
		}
	}

	for (i = 0; i < ARRAY_SIZE(bad_attributes); i++) {
		/* Skip EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR if
		 * it's EGL 1.5+
		 */
		if ((bad_attributes[i] == EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR) &&
		    check_egl_version_at_least(1, 5)) {
			continue;
		}

		pass = try_attribute(bad_attributes[i]) && pass;
	}

	EGL_KHR_create_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);

	return EXIT_SUCCESS;
}
