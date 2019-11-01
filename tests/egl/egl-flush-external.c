/*
 * Copyright © 2019 Intel Corporation
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

#include "piglit-util-egl.h"
#include "piglit-util-gl.h"

/**
 * @file egl-flush-external.c
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 20;

PIGLIT_GL_TEST_CONFIG_END

typedef EGLBoolean (EGLAPIENTRYP PFNEGLIMAGEFLUSHEXTERNALEXTPROC) (EGLDisplay dpy, EGLImage image, const EGLAttribKHR *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLIMAGEINVALIDATEEXTERNALEXTPROC) (EGLDisplay dpy, EGLImage image, const EGLAttribKHR *attrib_list);
#define EGL_IMAGE_EXTERNAL_FLUSH_EXT 0x32A2

/* dummy */
enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_OES_EGL_image");

	PFNEGLCREATEIMAGEKHRPROC peglCreateImageKHR = NULL;
	peglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)
		eglGetProcAddress("eglCreateImageKHR");
	if (!peglCreateImageKHR) {
		fprintf(stderr, "eglCreateImageKHR missing\n");
		piglit_report_result(PIGLIT_SKIP);
        }

	PFNEGLDESTROYIMAGEKHRPROC peglDestroyImageKHR = NULL;
	peglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)
		eglGetProcAddress("eglDestroyImageKHR");
	if (!peglDestroyImageKHR) {
		fprintf(stderr, "eglDestroyImageKHR missing\n");
		piglit_report_result(PIGLIT_SKIP);
        }

	/* Require EGL_MESA_platform_surfaceless extension. */
	const char *exts = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	if (!strstr(exts, "EGL_MESA_platform_surfaceless"))
		piglit_report_result(PIGLIT_SKIP);

	EGLint major, minor;
	EGLDisplay dpy;

	dpy = piglit_egl_get_default_display(EGL_PLATFORM_SURFACELESS_MESA);

	if (!eglInitialize(dpy, &major, &minor))
		piglit_report_result(PIGLIT_FAIL);

	piglit_require_egl_extension(dpy, "EGL_MESA_configless_context");
	piglit_require_egl_extension(dpy, "EGL_EXT_image_flush_external");

	PFNEGLIMAGEFLUSHEXTERNALEXTPROC peglImageFlushExternalEXT = NULL;
	peglImageFlushExternalEXT = (PFNEGLIMAGEFLUSHEXTERNALEXTPROC)
		eglGetProcAddress("eglImageFlushExternalEXT");
	if (!peglImageFlushExternalEXT) {
		fprintf(stderr, "eglImageFlushExternalEXT missing\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	PFNEGLIMAGEINVALIDATEEXTERNALEXTPROC peglImageInvalidateExternalEXT = NULL;
	peglImageInvalidateExternalEXT = (PFNEGLIMAGEINVALIDATEEXTERNALEXTPROC)
		eglGetProcAddress("eglImageInvalidateExternalEXT");
	if (!peglImageInvalidateExternalEXT) {
		fprintf(stderr, "eglImageInvalidateExternalEXT missing\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	EGLint ctx_attr[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	EGLContext ctx =
		eglCreateContext(dpy, EGL_NO_CONFIG_KHR, EGL_NO_CONTEXT, ctx_attr);
	if (ctx == EGL_NO_CONTEXT) {
		fprintf(stderr, "could not create EGL context\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx);

	/* Create a texture. */
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, NULL);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Create EGLImage with EGL_IMAGE_EXTERNAL_FLUSH_EXT set. */
	EGLint attribs[] = { EGL_IMAGE_EXTERNAL_FLUSH_EXT, EGL_TRUE, EGL_NONE };
	EGLImageKHR egl_image;
	egl_image = peglCreateImageKHR(dpy, ctx, EGL_GL_TEXTURE_2D,
				       (EGLClientBuffer) (intptr_t) texture,
				       attribs);
	if (!egl_image) {
		fprintf(stderr, "failed to create ImageKHR\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	EGLBoolean status = EGL_FALSE;

	/* Call flush & invalidate with invalid parameters. */
	const EGLAttribKHR bad_attrs[] = { EGL_RED_SIZE, EGL_GREEN_SIZE };

	status = peglImageFlushExternalEXT(dpy, egl_image, bad_attrs);
	if (!piglit_check_egl_error(EGL_BAD_PARAMETER))
		piglit_report_result(PIGLIT_FAIL);
	if (status == EGL_TRUE) {
		fprintf(stderr, "expected EGL_FALSE but got 0x%x\n", status);
		piglit_report_result(PIGLIT_FAIL);
	}

	status = peglImageInvalidateExternalEXT(dpy, egl_image, bad_attrs);
	if (!piglit_check_egl_error(EGL_BAD_PARAMETER))
		piglit_report_result(PIGLIT_FAIL);
	if (status == EGL_TRUE) {
		fprintf(stderr, "expected EGL_FALSE but got 0x%x\n", status);
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Call flush & invalidate with proper parameters. */
	const EGLAttribKHR good_attrs[] = { EGL_NONE, EGL_NONE };
	status = peglImageFlushExternalEXT(dpy, egl_image, good_attrs);
	if (!piglit_check_egl_error(EGL_SUCCESS))
		piglit_report_result(PIGLIT_FAIL);

	status = peglImageInvalidateExternalEXT(dpy, egl_image, good_attrs);
	if (!piglit_check_egl_error(EGL_SUCCESS))
		piglit_report_result(PIGLIT_FAIL);

	glDeleteTextures(1, &texture);
	peglDestroyImageKHR(dpy, egl_image);

	eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglTerminate(dpy);

	piglit_report_result(PIGLIT_PASS);
}
