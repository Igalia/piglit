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
 * @file
 *
 * The Intel driver supports glEGLImageTargetRenderbufferStorageOES and
 * glEGLImageTargetTexture2DOES(GL_TEXTURE_2D) for EGLImages imported with
 * EGL_EXT_image_dma_buf_import, as long as the image has a single plane and
 * a non-exotic format. This test verifies that the two calls succeed with
 * an RGBA dma_buf.
 *
 * This is only an API test. It doesn't actually render to or texture from the
 * EGLImage.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 10;

PIGLIT_GL_TEST_CONFIG_END

static PFNGLGENRENDERBUFFERSOESPROC piglit_glGenRenderbuffersOES;
static PFNGLBINDRENDERBUFFEROESPROC piglit_glBindRenderbufferOES;
static PFNGLDELETERENDERBUFFERSOESPROC piglit_glDeleteRenderbuffersOES;

static EGLImageKHR
create_image(unsigned w, unsigned h, int fd, unsigned stride, unsigned offset)
{
	EGLint attr[] = {
		EGL_WIDTH, w,
		EGL_HEIGHT, h,
		EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_ARGB8888,
		EGL_DMA_BUF_PLANE0_FD_EXT, fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, offset,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, stride,
		EGL_NONE
	};

	return eglCreateImageKHR(eglGetCurrentDisplay(), EGL_NO_CONTEXT,
			EGL_LINUX_DMA_BUF_EXT, (EGLClientBuffer)NULL, attr);
}

static bool
try_as_texture_2d(EGLImageKHR img)
{
	GLuint tex;
	bool res;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	/* Set the image as level zero */
	glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)img);
	res = piglit_check_gl_error(GL_NO_ERROR);

	glDeleteTextures(1, &tex);

	return res;
}

static bool
try_as_render_buffer(EGLImageKHR img)
{
	GLuint rbo;
	bool res;

	piglit_glGenRenderbuffersOES(1, &rbo);
	piglit_glBindRenderbufferOES(GL_RENDERBUFFER_OES, rbo);

	glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER_OES, img);
	res = piglit_check_gl_error(GL_NO_ERROR);

	piglit_glDeleteRenderbuffersOES(1, &rbo);

	return res;
}

enum piglit_result
piglit_display(void)
{
	const unsigned w = 2;
	const unsigned h = 2;
	const unsigned cpp = 4;
	const unsigned char *pixels = alloca(w * h * cpp);
	struct piglit_dma_buf *buf;
	unsigned stride;
	unsigned offset;
	int fd;
	EGLImageKHR img;
	enum piglit_result res;
	bool pass = true;

	res = piglit_create_dma_buf(w, h, cpp, pixels, w * cpp,
				&buf, &fd, &stride, &offset);
	if (res != PIGLIT_PASS)
		return res;

	img = create_image(w, h, fd, stride, offset);

	if (!img) {
		piglit_destroy_dma_buf(buf);

		/* unsupported format (BAD_MATCH) is not an error. */
		return piglit_check_egl_error(EGL_BAD_MATCH) ?
					PIGLIT_SKIP : PIGLIT_FAIL;
	}

	pass = try_as_texture_2d(img) && pass;
	pass = try_as_render_buffer(img) && pass;

	eglDestroyImageKHR(eglGetCurrentDisplay(), img);
	piglit_destroy_dma_buf(buf);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	static const char intel_id[] = "Intel Open Source Technology Center";
	const char *vendor_str;
	EGLDisplay egl_dpy = eglGetCurrentDisplay();

	piglit_require_egl_extension(egl_dpy, "EGL_EXT_image_dma_buf_import");
	piglit_require_egl_extension(egl_dpy, "EGL_KHR_image_base");
	piglit_require_extension("GL_OES_EGL_image");
	piglit_require_extension("GL_OES_framebuffer_object");
	
	piglit_glGenRenderbuffersOES =
		(PFNGLGENRENDERBUFFERSOESPROC)
		eglGetProcAddress("glGenRenderbuffersOES");
	piglit_glBindRenderbufferOES =
		(PFNGLBINDRENDERBUFFEROESPROC)
		eglGetProcAddress("glBindRenderbufferOES");
	piglit_glDeleteRenderbuffersOES =
		(PFNGLDELETERENDERBUFFERSOESPROC)
		eglGetProcAddress("glDeleteRenderbuffersOES");
	if (!piglit_glGenRenderbuffersOES ||
	    !piglit_glBindRenderbufferOES ||
	    !piglit_glDeleteRenderbuffersOES)
		piglit_report_result(PIGLIT_FAIL);

	vendor_str = (const char *)glGetString(GL_VENDOR);
	if (strncmp(vendor_str, intel_id, sizeof(intel_id) - 1) != 0) {
		printf("Test requires intel gpu\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}
