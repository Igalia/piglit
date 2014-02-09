/*
 * Copyright © 2013 Intel Corporation
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

#include "image_common.h"

/**
 * @file intel_external_sampler_with_dma_only.c
 *
 * Intel driver allows image external sampler to be used only with imported 
 * dma buffers. This test creates an egl image based on a 2D-texture, attempts
 * to use the image as target for an external texture, and expects to fail with
 * GL_INVALID_OPERATION.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 10;

PIGLIT_GL_TEST_CONFIG_END

static EGLImageKHR
create_tex_based_egl_image(unsigned w, unsigned h, const unsigned char *pixels)
{
	GLuint tex;
	EGLImageKHR img;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
	       GL_UNSIGNED_BYTE, pixels);

	img = eglCreateImageKHR(eglGetCurrentDisplay(), eglGetCurrentContext(),
	       EGL_GL_TEXTURE_2D_KHR, (EGLClientBuffer)(intptr_t)tex, 0);
	if (!img)
	       printf("failed to create EGL image out of texture\n");

	glDeleteTextures(1, &tex);

	return img;
}

enum piglit_result
piglit_display(void)
{
	GLuint tex;
	enum piglit_result result = PIGLIT_FAIL;
	const unsigned char src[] = { 0x00, 0x00, 0x00, 0x00 };
	EGLImageKHR img = create_tex_based_egl_image(1, 1, src);

	if (img == EGL_NO_IMAGE_KHR)
	       return PIGLIT_FAIL;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, tex);

	glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES,
			       (GLeglImageOES)img);

	if (piglit_check_gl_error(GL_INVALID_OPERATION))
	       result = PIGLIT_PASS;

	glDeleteTextures(1, &tex);
	eglDestroyImageKHR(eglGetCurrentDisplay(), img);

	return result;
}

void
piglit_init(int argc, char **argv)
{
	static const char intel_id[] = "Intel Open Source Technology Center";
	const char *vendor_str;
	EGLDisplay egl_dpy = eglGetCurrentDisplay();

	piglit_require_egl_extension(egl_dpy, "EGL_EXT_image_dma_buf_import");
	piglit_require_egl_extension(egl_dpy, "EGL_KHR_image_base");

	vendor_str = (const char *)glGetString(GL_VENDOR);
	if (strncmp(vendor_str, intel_id, sizeof(intel_id) - 1) != 0) {
		printf("Test requires intel gpu\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}
