/*
 * Copyright 2017 Google
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

#undef EGL_EGLEXT_PROTOTYPES

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "piglit-dispatch.h"
#include "piglit-util-egl.h"
#include "piglit-util-gl.h"

static PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;


/* Some drivers will allocate a private auxiliary metadata surface if
 * the image is large enough. We want to test if the auxiliary surface
 * is shared correctly through the EGLImage.
 */
static const uint32_t width = 1024;
static const uint32_t height = 1024;

static EGLContext
create_context(EGLDisplay dpy)
{
	bool ok = false;
	EGLConfig config = 0;
	EGLint num_configs = 0;
	EGLContext ctx = 0;

	static const EGLint config_attribs[] = {
		EGL_RED_SIZE,		EGL_DONT_CARE,
		EGL_GREEN_SIZE,		EGL_DONT_CARE,
		EGL_BLUE_SIZE,		EGL_DONT_CARE,
		EGL_ALPHA_SIZE,		EGL_DONT_CARE,
		EGL_DEPTH_SIZE, 	EGL_DONT_CARE,
		EGL_STENCIL_SIZE, 	EGL_DONT_CARE,
		EGL_RENDERABLE_TYPE, 	EGL_OPENGL_BIT,
		EGL_SURFACE_TYPE,	0,
		EGL_NONE,
	};

	static const EGLint context_attribs[] = {
		EGL_CONTEXT_MAJOR_VERSION, 3,
		EGL_CONTEXT_MINOR_VERSION, 2,
		EGL_NONE,
	};

	ok = eglChooseConfig(dpy, config_attribs, &config, 1,
			     &num_configs);
	if (!ok || !config || num_configs == 0) {
		EGLint egl_error = eglGetError();
		piglit_loge("failed to get EGLConfig: %s(0x%x)",
			  piglit_get_egl_error_name(egl_error), egl_error);
		piglit_report_result(PIGLIT_SKIP);
	}

	ok = piglit_egl_bind_api(EGL_OPENGL_API);
	if (!ok) {
		piglit_loge("failed to bind EGL_OPENGL_API");
		piglit_report_result(PIGLIT_FAIL);

	}

	ctx = eglCreateContext(dpy, config, EGL_NO_CONTEXT, context_attribs);
	if (!ctx) {
		EGLint egl_error = eglGetError();
		piglit_loge("failed to create EGLContext: %s(0x%x)",
			  piglit_get_egl_error_name(egl_error), egl_error);
		piglit_report_result(PIGLIT_FAIL);
	}

	return ctx;
}

static EGLDisplay
create_display(void)
{
	EGLDisplay dpy;
	bool ok;
	EGLint egl_major, egl_minor;

	dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (!dpy) {
		fprintf(stderr, "failed to get EGLDisplay\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	ok = eglInitialize(dpy, &egl_major, &egl_minor);
	if (!ok) {
		EGLint egl_error = eglGetError();
		piglit_loge("failed to get EGLConfig: %s(0x%x)",
			  piglit_get_egl_error_name(egl_error), egl_error);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!piglit_is_egl_extension_supported(dpy, "EGL_KHR_gl_renderbuffer_image")) {
		piglit_loge("display does not support EGL_KHR_gl_renderbuffer_image\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	eglCreateImageKHR = (void*) eglGetProcAddress("eglCreateImageKHR");
	if (!eglCreateImageKHR) {
		fprintf(stderr, "eglGetProcAddress(\"eglCreateImageKHR\") failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	return dpy;
}

static void
create_framebuffers(EGLDisplay dpy, EGLContext ctx,
		    GLuint *fb1, GLuint *fb2, GLenum internal_format)
{
	GLuint rb1, rb2;
	EGLImageKHR img;
	GLenum attachment_point = 0;

	switch (internal_format) {
	case GL_RGBA:
		attachment_point = GL_COLOR_ATTACHMENT0;
		break;
	case GL_DEPTH_COMPONENT24:
		attachment_point = GL_DEPTH_ATTACHMENT;
		break;
	default:
		abort();
	}

	glGenRenderbuffers(1, &rb1);
	glBindRenderbuffer(GL_RENDERBUFFER, rb1);
	glRenderbufferStorage(GL_RENDERBUFFER, internal_format, width, height);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glGenFramebuffers(1, fb1);
	glBindFramebuffer(GL_FRAMEBUFFER, *fb1);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment_point, GL_RENDERBUFFER, rb1);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* We intentionally create the EGLImage before using the renderbuffer.
	 * This confuses some versions of the Intel driver.
	 */
	img = eglCreateImageKHR(dpy, ctx, EGL_GL_RENDERBUFFER_KHR, (EGLClientBuffer) (uintptr_t) rb1, NULL);
	if (img == EGL_NO_IMAGE_KHR) {
		/* Skip, not fail, because the spec allows the implementation
		 * to reject image creation.
		 */
		piglit_loge("failed to create EGLImage");
		piglit_report_result(PIGLIT_SKIP);
	}

	glGenRenderbuffers(1, &rb2);
	glBindRenderbuffer(GL_RENDERBUFFER, rb2);
	glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, img);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glGenFramebuffers(1, fb2);
	glBindFramebuffer(GL_FRAMEBUFFER, *fb2);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment_point, GL_RENDERBUFFER, rb2);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}

static void
test_rgba(EGLDisplay dpy, EGLContext ctx)
{
	const float color1[4] = { 0.0, 1.0, 0.0, 1.0 };
	const float color2[4] = { 0.0, 0.0, 1.0, 1.0 };

	bool pass = true;
	GLuint fb1, fb2;

	create_framebuffers(dpy, ctx, &fb1, &fb2, GL_RGBA);

	/* Clear rb1 to color1. Check that rb2 has color1. */
	glBindFramebuffer(GL_FRAMEBUFFER, fb1);
	glClearBufferfv(GL_COLOR, 0, color1);
	glBindFramebuffer(GL_FRAMEBUFFER, fb2);
	pass &= piglit_probe_rect_rgba(0, 0, width, height, color1);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* Clear rb2 to color2. Check that rb1 has color2. */
	glBindFramebuffer(GL_FRAMEBUFFER, fb2);
	glClearBufferfv(GL_COLOR, 0, color2);
	glBindFramebuffer(GL_FRAMEBUFFER, fb1);
	pass &= piglit_probe_rect_rgba(0, 0, width, height, color2);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

static void
test_depth24(EGLDisplay dpy, EGLContext ctx)
{
	const float depth1 = 0.25;
	const float depth2 = 0.75;

	bool pass = true;
	GLuint fb1, fb2;

	create_framebuffers(dpy, ctx, &fb1, &fb2, GL_DEPTH_COMPONENT24);

	/* Clear rb1 to depth1. Check that rb2 has depth1. */
	glBindFramebuffer(GL_FRAMEBUFFER, fb1);
	glClearBufferfv(GL_DEPTH, 0, &depth1);
	glBindFramebuffer(GL_FRAMEBUFFER, fb2);
	pass &= piglit_probe_rect_depth(0, 0, width, height, depth1);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* Clear rb2 to depth2. Check that rb1 has depth2. */
	glBindFramebuffer(GL_FRAMEBUFFER, fb2);
	glClearBufferfv(GL_DEPTH, 0, &depth2);
	glBindFramebuffer(GL_FRAMEBUFFER, fb1);
	pass &= piglit_probe_rect_depth(0, 0, width, height, depth2);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

static void
usage_error(void)
{
	fprintf(stderr, "usage: egl_khr_gl_image <internalformat>\n"
			"\n"
			"internalformats:\n"
			"    GL_RGBA\n"
			"    GL_DEPTH_COMPONENT24\n");
	piglit_report_result(PIGLIT_FAIL);
}

int
main(int argc, char **argv)
{
	GLenum internal_format = 0;

	EGLDisplay dpy;
	EGLContext ctx;
	bool ok;

	/* Strip common piglit args. */
	piglit_strip_arg(&argc, argv, "-fbo");
	piglit_strip_arg(&argc, argv, "-auto");

	if (argc == 2) {
		if (streq(argv[1], "GL_RGBA")) {
			internal_format = GL_RGBA;
		} else if (streq(argv[1], "GL_DEPTH_COMPONENT24")) {
			internal_format = GL_DEPTH_COMPONENT24;
		}
	}

	if (internal_format == 0)
		usage_error();

	dpy = create_display();
	ctx = create_context(dpy);

	ok = eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx);
	if (!ok) {
		piglit_loge("failed to make context current without surface");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_dispatch_default_init(PIGLIT_DISPATCH_ES2);

	if (!piglit_is_extension_supported("GL_OES_EGL_image")) {
		piglit_loge("context does not support GL_OES_EGL_image");
		piglit_report_result(PIGLIT_SKIP);
	}

	switch (internal_format) {
	case GL_RGBA:
		test_rgba(dpy, ctx);
		break;
	case GL_DEPTH_COMPONENT24:
		test_depth24(dpy, ctx);
		break;
	default:
		break;
	}

	/* unreachable */
	abort();
}
