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
 * @file missing_attributes.c
 *
 * Tests that EGL detects missing attributes correctly.
 *
 * From the EXT_image_dma_buf_import spec:
 *
 * "If <target> is EGL_LINUX_DMA_BUF_EXT, and the list of attributes is
 *  incomplete, EGL_BAD_PARAMETER is generated."
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 10;

PIGLIT_GL_TEST_CONFIG_END

#define NUM_MANDATORY_ATTRS 6

static bool
test_missing(int fd, const EGLint *attr)
{
	EGLImageKHR img;

	img = eglCreateImageKHR(eglGetCurrentDisplay(), EGL_NO_CONTEXT,
			EGL_LINUX_DMA_BUF_EXT, (EGLClientBuffer)NULL, attr);

	if (!piglit_check_egl_error(EGL_BAD_PARAMETER)) {
		if (img)
			eglDestroyImageKHR(eglGetCurrentDisplay(), img);
		return false;
	}

	return true;
}

static void
fill_full_set(unsigned w, unsigned h, EGLint fd, EGLint offset, EGLint stride,
	EGLint *out)
{
	/* Reference set containing all the mandatory attributes. */
	const EGLint all[2 * NUM_MANDATORY_ATTRS] = {
		EGL_WIDTH, w,
		EGL_HEIGHT, h,
		EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_ARGB8888,
		EGL_DMA_BUF_PLANE0_FD_EXT, fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, offset,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, stride
	};

	memcpy(out, all, sizeof(all));
}

static void
fill_one_missing(const EGLint *all, EGLint *out, EGLint missing)
{
	unsigned i, j;

	for (i = j = 0; i < NUM_MANDATORY_ATTRS; ++i) {
		if (all[2 * i] != missing) {
			out[2 * j + 0] = all[2 * i + 0];
			out[2 * j + 1] = all[2 * i + 1];
			++j;
		}
	}

	out[2 * j] = EGL_NONE;
}

/**
 * Here one tries to create an image with six different attribute sets each
 * missing one of the mandatory attribute.
 *
 * One and same buffer is used for all the tests. Each test is expected to fail
 * meaning that the ownership is not transferred to the EGL in any point.
 */
enum piglit_result
piglit_display(void)
{
	const unsigned w = 2;
	const unsigned h = 2;
	const unsigned cpp = 2;
	const unsigned char pixels[w * h * cpp];
	EGLint all[2 * NUM_MANDATORY_ATTRS];
	EGLint missing[2 * (NUM_MANDATORY_ATTRS - 1) + 1];
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

	fill_full_set(w, h, fd, offset, stride, all);

	fill_one_missing(all, missing, EGL_HEIGHT);
	pass = test_missing(fd, missing) && pass;

	fill_one_missing(all, missing, EGL_WIDTH);
	pass = test_missing(fd, missing) && pass;

	fill_one_missing(all, missing, EGL_LINUX_DRM_FOURCC_EXT);
	pass = test_missing(fd, missing) && pass;

	fill_one_missing(all, missing, EGL_DMA_BUF_PLANE0_FD_EXT);
	pass = test_missing(fd, missing) && pass;

	fill_one_missing(all, missing, EGL_DMA_BUF_PLANE0_OFFSET_EXT);
	pass = test_missing(fd, missing) && pass;

	fill_one_missing(all, missing, EGL_DMA_BUF_PLANE0_PITCH_EXT);
	pass = test_missing(fd, missing) && pass;

	/**
	 * EGL stack can claim the ownership of the file descriptor only when it
	 * succeeds. Close the file descriptor here and check that it really
	 * wasn't closed by EGL.
	 */
	pass = (close(fd) == 0) && pass;

	piglit_destroy_dma_buf(buf);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	EGLDisplay egl_dpy = eglGetCurrentDisplay();
	piglit_require_egl_extension(egl_dpy, "EGL_EXT_image_dma_buf_import");
}
