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
 * @file invalid_attributes.c
 *
 * From the EXT_image_dma_buf_import spec:
 *
 * "* If <target> is EGL_LINUX_DMA_BUF_EXT and <buffer> is not NULL, the
 *    error EGL_BAD_PARAMETER is generated."
 *
 * and
 *
 * "* If <target> is EGL_LINUX_DMA_BUF_EXT, and the EGL_LINUX_DRM_FOURCC_EXT
 *    attribute indicates a single-plane format, EGL_BAD_ATTRIBUTE is
 *    generated if any of the EGL_DMA_BUF_PLANE1_* or EGL_DMA_BUF_PLANE2_*
 *    attributes are specified.
 *
 *  * If <target> is EGL_LINUX_DMA_BUF_EXT and one or more of the values
 *    specified for a plane's pitch or offset isn't supported by EGL,
 *    EGL_BAD_ACCESS is generated."
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 10;

PIGLIT_GL_TEST_CONFIG_END

#define DRM_FORMAT_INVALID fourcc_code('F', 'O', 'O', '0')

static bool
test_excess_attributes(unsigned w, unsigned h, int fd, unsigned stride,
		unsigned offset, EGLint attr_id, EGLint attr_val)
{
	const EGLint excess_attr[2 * 7 + 1] = {
			EGL_HEIGHT, w,
		 	EGL_WIDTH, h,
			EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_ARGB8888,
			EGL_DMA_BUF_PLANE0_FD_EXT, fd,
			EGL_DMA_BUF_PLANE0_OFFSET_EXT, offset,
			EGL_DMA_BUF_PLANE0_PITCH_EXT, stride,
			attr_id, attr_val,
			EGL_NONE };
	EGLImageKHR img = eglCreateImageKHR(eglGetCurrentDisplay(),
				EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT,
				(EGLClientBuffer)NULL, excess_attr);

	if (!piglit_check_egl_error(EGL_BAD_ATTRIBUTE)) {
		if (img)
			eglDestroyImageKHR(eglGetCurrentDisplay(), img);
		return false;
	}

	return true;
}

static bool
test_buffer_not_null(unsigned w, unsigned h, int fd, unsigned stride,
		unsigned offset)
{
	EGLImageKHR img;
	EGLint attr[] = {
		EGL_WIDTH, w,
		EGL_HEIGHT, h,
		EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_ARGB8888,
		EGL_DMA_BUF_PLANE0_FD_EXT, fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, offset,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, stride,
		EGL_NONE
	};

	/**
	 * The spec says:
	 *
	 *     "If <target> is EGL_LINUX_DMA_BUF_EXT, <dpy> must be a valid
	 *      display, <ctx> must be EGL_NO_CONTEXT, and <buffer> must be
	 *      NULL, cast into the type EGLClientBuffer."
	 */
	img = eglCreateImageKHR(eglGetCurrentDisplay(), EGL_NO_CONTEXT,
			EGL_LINUX_DMA_BUF_EXT, (EGLClientBuffer)1, attr);

	if (!piglit_check_egl_error(EGL_BAD_PARAMETER)) {
		if (img)
			eglDestroyImageKHR(eglGetCurrentDisplay(), img);
		return false;
	}

	return true;
}

static bool
test_invalid_context(unsigned w, unsigned h, int fd, unsigned stride,
		unsigned offset)
{
	EGLImageKHR img;
	EGLint attr[] = {
		EGL_WIDTH, w,
		EGL_HEIGHT, h,
		EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_ARGB8888,
		EGL_DMA_BUF_PLANE0_FD_EXT, fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, offset,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, stride,
		EGL_NONE
	};

	/**
	 * The spec says:
	 *
	 *     "If <target> is EGL_LINUX_DMA_BUF_EXT, <dpy> must be a valid
	 *      display, <ctx> must be EGL_NO_CONTEXT, and <buffer> must be
	 *      NULL, cast into the type EGLClientBuffer."
	 */
	img = eglCreateImageKHR(eglGetCurrentDisplay(), eglGetCurrentContext(),
			EGL_LINUX_DMA_BUF_EXT, (EGLClientBuffer)NULL, attr);

	if (!piglit_check_egl_error(EGL_BAD_PARAMETER)) {
		if (img)
			eglDestroyImageKHR(eglGetCurrentDisplay(), img);
		return false;
	}

	return true;
}

static bool
test_invalid_format(unsigned w, unsigned h, int fd, unsigned stride,
		unsigned offset)
{
	EGLImageKHR img;
	EGLint attr[] = {
		EGL_WIDTH, w,
		EGL_HEIGHT, h,
		EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_INVALID,
		EGL_DMA_BUF_PLANE0_FD_EXT, fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, offset,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, stride,
		EGL_NONE
	};

	img = eglCreateImageKHR(eglGetCurrentDisplay(), EGL_NO_CONTEXT,
			EGL_LINUX_DMA_BUF_EXT, (EGLClientBuffer)0, attr);

	if (!piglit_check_egl_error(EGL_BAD_ATTRIBUTE)) {
		if (img)
			eglDestroyImageKHR(eglGetCurrentDisplay(), img);
		return false;
	}

	return true;
}

static bool
test_pitch_zero(unsigned w, unsigned h, int fd, unsigned stride,
		unsigned offset)
{
	EGLImageKHR img;
	EGLint attr[] = {
		EGL_WIDTH, w,
		EGL_HEIGHT, h,
		EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_ARGB8888,
		EGL_DMA_BUF_PLANE0_FD_EXT, fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, offset,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, 0,
		EGL_NONE
	};

	img = eglCreateImageKHR(eglGetCurrentDisplay(), EGL_NO_CONTEXT,
			EGL_LINUX_DMA_BUF_EXT, (EGLClientBuffer)0, attr);

	if (!piglit_check_egl_error(EGL_BAD_ACCESS)) {
		if (img)
			eglDestroyImageKHR(eglGetCurrentDisplay(), img);
		return false;
	}

	return true;
}

/**
 * One and same buffer is used for all the tests. Each test is expected to fail
 * meaning that the ownership is not transferred to the EGL in any point.
 */
enum piglit_result
piglit_display(void)
{
	const unsigned w = 2;
	const unsigned h = 2;
	const unsigned cpp = 4;
	const unsigned char pixels[w * h * cpp];
	struct piglit_dma_buf *buf;
	unsigned stride;
	unsigned offset;
	int fd;
	enum piglit_result res;
	bool pass = true;

	res = piglit_create_dma_buf(w, h, cpp, pixels, w * cpp,
				&buf, &fd, &stride, &offset);
	if (res != PIGLIT_PASS)
		return res;

	pass = test_excess_attributes(w, h, fd, stride, offset,
				EGL_DMA_BUF_PLANE1_FD_EXT, fd) && pass;
	pass = test_excess_attributes(w, h, fd, stride, offset,
				EGL_DMA_BUF_PLANE1_OFFSET_EXT, 0) && pass;
	pass = test_excess_attributes(w, h, fd, stride, offset,
				EGL_DMA_BUF_PLANE1_PITCH_EXT, stride) && pass;
	pass = test_excess_attributes(w, h, fd, stride, offset,
				EGL_DMA_BUF_PLANE2_FD_EXT, fd) && pass;
	pass = test_excess_attributes(w, h, fd, stride, offset,
				EGL_DMA_BUF_PLANE2_OFFSET_EXT, 0) && pass;
	pass = test_excess_attributes(w, h, fd, stride, offset,
				EGL_DMA_BUF_PLANE2_PITCH_EXT, stride) && pass;
	pass = test_buffer_not_null(w, h, fd, stride, offset) && pass;
	pass = test_invalid_context(w, h, fd, stride, offset) && pass;
	pass = test_invalid_format(w, h, fd, stride, offset) && pass;
	pass = test_pitch_zero(w, h, fd, stride, offset) && pass;

	piglit_destroy_dma_buf(buf);

	/**
	 * EGL stack can claim the ownership of the file descriptor only when it
	 * succeeds. Close the file descriptor here and check that it really
	 * wasn't closed by EGL.
	 */
	pass = (close(fd) == 0) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_egl_extension("EGL_EXT_image_dma_buf_import");
}
