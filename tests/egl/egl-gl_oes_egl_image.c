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
 * @file egl-gl_oes_egl_image.c
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 20;

PIGLIT_GL_TEST_CONFIG_END

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

	EGLint ctx_attr[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	EGLContext ctx =
		eglCreateContext(dpy, EGL_NO_CONFIG_MESA, EGL_NO_CONTEXT, ctx_attr);
	if (ctx == EGL_NO_CONTEXT) {
		fprintf(stderr, "could not create EGL context\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx);

	/* Create a texture. */
	GLuint texture_a;
	glGenTextures(1, &texture_a);
	glBindTexture(GL_TEXTURE_2D, texture_a);

	/* Setup 2 miplevels, and max level. */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 128, 128, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Create EGLImage from texture.  */
	EGLint attribs[] = { EGL_GL_TEXTURE_LEVEL_KHR, 1, EGL_NONE };
	EGLImageKHR egl_image;
	egl_image = peglCreateImageKHR(dpy, ctx, EGL_GL_TEXTURE_2D,
				       (EGLClientBuffer) (intptr_t) texture_a,
				       attribs);
	if (!egl_image) {
		fprintf(stderr, "failed to create ImageKHR\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Create another texture. */
	GLuint texture_b;
	glGenTextures(1, &texture_b);
	glBindTexture(GL_TEXTURE_2D, texture_b);

	/* Specify texture from EGLImage but use wrong target.  */
	glEGLImageTargetTexture2DOES(GL_TEXTURE_CUBE_MAP_ARRAY, egl_image);

	if (!piglit_check_gl_error(GL_INVALID_ENUM))
		piglit_report_result(PIGLIT_FAIL);

	/* Specify texture from EGLImage properly.  */
	glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, egl_image);

	GLint w, h;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

	/* Verify that we got the miplevel 1 dimensions. */
	if (w != 128 || h != 128)
		piglit_report_result(PIGLIT_FAIL);

	glDeleteTextures(1, &texture_a);
	glDeleteTextures(1, &texture_b);
	peglDestroyImageKHR(dpy, egl_image);

	piglit_report_result(PIGLIT_PASS);
}
