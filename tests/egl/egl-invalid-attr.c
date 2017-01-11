/*
 * Copyright © 2016 Collabora Ltd.
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
 * @file egl-invalid-attr.c
 *
 * From EGL_KHR_image_base.txt: If an attribute specified in <attrib_list> is
 * not one of the attributes listed in Table bbb, the error EGL_BAD_PARAMETER
 * is generated.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_compat_version = 10;

PIGLIT_GL_TEST_CONFIG_END


enum piglit_result
piglit_display(void)
{
	GLuint tex;
	const unsigned char src[] = { 0x00, 0x00, 0x00, 0x00 };
	EGLint attr[] = {
		0xFFFF, 0, //invalid attr
		EGL_NONE
	};
	PFNEGLCREATEIMAGEKHRPROC peglCreateImageKHR = NULL;

	peglCreateImageKHR =
           (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress ("eglCreateImageKHR");
	if (!peglCreateImageKHR) {
		printf ("could not get address for "
                        "eglCreateImageKHR, skipping\n");
		return PIGLIT_SKIP;
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA,
	       GL_UNSIGNED_BYTE, src);

	peglCreateImageKHR(eglGetCurrentDisplay(),
                           eglGetCurrentContext(),
                           EGL_GL_TEXTURE_2D_KHR,
                           (EGLClientBuffer)(intptr_t)tex,
                           attr);

	if (!piglit_check_egl_error(EGL_BAD_PARAMETER)) {
		fprintf(stderr, "eglCreateImageKHR() "
                        "error wasn't EGL_BAD_PARAMETER\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glDeleteTextures(1, &tex);


	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	EGLDisplay dpy;

	/* this check could be turned into a helper fn*/
	dpy = eglGetCurrentDisplay();
	if (!dpy) {
		printf ("EGL not supported on this platform, skipping\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_require_egl_extension(dpy, "EGL_KHR_image_base");
	piglit_require_egl_extension(dpy, "EGL_KHR_gl_texture_2D_image");
}
