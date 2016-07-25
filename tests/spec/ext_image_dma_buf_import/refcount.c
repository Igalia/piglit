/*
 * Copyright © 2016 Broadcom
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

#include "sample_common.h"
#include "image_common.h"

/**
 * @file refcount.c
 *
 * Creates two EGL images from an ARGB8888 dmabuf, samples each one,
 * destroys one, then tests that the other can still be sampled.
 *
 * This gets at a common refcounting bug in drivers: GEM returns the
 * same handle for a given BO re-opened through dmabuf on the same
 * device fd, but that GEM handle is not refcounted.  The userspace
 * driver needs to be sure that it's doing handle refcounting itself.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	static int fourcc = fourcc_code('A', 'R', '2', '4');
	int w = 2, h = 2;
	const unsigned char src[] = {
		0x00, 0x00, 0xff, 0xff,
		0x00, 0xff, 0x00, 0xff,
		0xff, 0x00, 0x00, 0xff,
		0xff, 0xff, 0xff, 0xff
	};
	int cpp = 4;
	enum piglit_result res;
	struct piglit_dma_buf *buf;
	unsigned stride, offset;
	int fd;
	EGLImageKHR img1, img2;
	GLuint tex1, tex2;
	/* Scale up factor for drawing the texture to the screen. */
	int scale = 10;
	int y_spacing = h * scale + 5;
	int i;
	GLubyte *expected;

	res = piglit_create_dma_buf(w, h, cpp, src, w * cpp,
				    &buf, &fd, &stride, &offset);
	if (res != PIGLIT_PASS)
		return res;

	res = egl_image_for_dma_buf_fd(dup(fd), fourcc, w, h, stride, offset,
				       &img1);
	if (res != PIGLIT_PASS)
		return res;

	res = egl_image_for_dma_buf_fd(dup(fd), fourcc, w, h, stride, offset,
				       &img2);
	if (res != PIGLIT_PASS)
		return res;

	close(fd);

	res = texture_for_egl_image(img1, &tex1);
	if (res != PIGLIT_PASS)
		return res;

	res = texture_for_egl_image(img2, &tex2);
	if (res != PIGLIT_PASS)
		return res;

	sample_tex(tex1,
		   0, y_spacing * 0,
		   w * scale, h * scale);
	sample_tex(tex2,
		   0, y_spacing * 1,
		   w * scale, h * scale);

	glDeleteTextures(1, &tex2);
	eglDestroyImageKHR(eglGetCurrentDisplay(), img2);

	sample_tex(tex1,
		   0, y_spacing * 2,
		   w * scale, h * scale);

	expected = piglit_rgbw_image_ubyte(w * scale, h * scale, false);

	for (i = 0; i < 3; i++) {
		if (!piglit_probe_image_ubyte(0,
					      y_spacing * i,
					      w * scale, h * scale,
					      GL_RGBA, expected)) {
			res = PIGLIT_FAIL;
		}
	}

	free(expected);

	piglit_present_results();

	return res;
}

void
piglit_init(int argc, char **argv)
{
	EGLDisplay egl_dpy = eglGetCurrentDisplay();

	piglit_require_egl_extension(egl_dpy, "EGL_EXT_image_dma_buf_import");
	piglit_require_extension("GL_OES_EGL_image_external");
}
