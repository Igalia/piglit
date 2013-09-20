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
 * @file invalid_hints.c
 *
 * From the EXT_image_dma_buf_import spec:
 *
 * "Accepted as the value for the EGL_YUV_COLOR_SPACE_HINT_EXT attribute:
 *
 *       EGL_ITU_REC601_EXT   0x327F
 *       EGL_ITU_REC709_EXT   0x3280
 *       EGL_ITU_REC2020_EXT  0x3281
 * 
 * Accepted as the value for the EGL_SAMPLE_RANGE_HINT_EXT attribute:
 * 
 *       EGL_YUV_FULL_RANGE_EXT    0x3282
 *       EGL_YUV_NARROW_RANGE_EXT  0x3283
 *
 * Accepted as the value for the EGL_YUV_CHROMA_HORIZONTAL_SITING_HINT_EXT &
 * EGL_YUV_CHROMA_VERTICAL_SITING_HINT_EXT attributes:
 *
 *       EGL_YUV_CHROMA_SITING_0_EXT    0x3284
 *       EGL_YUV_CHROMA_SITING_0_5_EXT  0x3285"
 *
 *
 * In order to test these, one needs to have the following in place:
 * EGL_WIDTH, EGL_HEIGHT, EGL_LINUX_DRM_FOURCC_EXT, EGL_DMA_BUF_PLANE0_FD_EXT,
 * EGL_DMA_BUF_PLANE0_OFFSET_EXT and EGL_DMA_BUF_PLANE0_PITCH_EXT.
 *
 * \see invalid_attributes.c and missing_attributes.c
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 10;

PIGLIT_GL_TEST_CONFIG_END

static bool
test_invalid_hint(unsigned w, unsigned h, int fd, unsigned stride,
		unsigned offset, int hint, int val)
{
	EGLImageKHR img;
	EGLint attr[] = {
		EGL_WIDTH, w,
		EGL_HEIGHT, h,
		EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_ARGB8888,
		EGL_DMA_BUF_PLANE0_FD_EXT, fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, offset,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, stride,
		hint, val,
		EGL_NONE
	};

	img = eglCreateImageKHR(eglGetCurrentDisplay(), EGL_NO_CONTEXT,
			EGL_LINUX_DMA_BUF_EXT, (EGLClientBuffer)NULL, attr);

	if (!piglit_check_egl_error(EGL_BAD_ATTRIBUTE)) {
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

	pass = test_invalid_hint(w, h, fd, stride, offset,
			EGL_YUV_COLOR_SPACE_HINT_EXT, 0) && pass;
	pass = test_invalid_hint(w, h, fd, stride, offset,
			EGL_SAMPLE_RANGE_HINT_EXT, 0) && pass;
	pass = test_invalid_hint(w, h, fd, stride, offset,
			EGL_YUV_CHROMA_HORIZONTAL_SITING_HINT_EXT, 0) && pass;
	pass = test_invalid_hint(w, h, fd, stride, offset,
			EGL_YUV_CHROMA_VERTICAL_SITING_HINT_EXT, 0) && pass;

	piglit_destroy_dma_buf(buf);

	/**
	 * EGL stack can claim the ownership of the file descriptor only when it
	 * succeeds. Close the descriptor and check that it really wasn't closed
	 * by EGL.
	 */
	pass = (close(fd) == 0) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_egl_extension("EGL_EXT_image_dma_buf_import");
}
