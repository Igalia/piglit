/*
 * Copyright © 2021 Intel Corporation
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
 * @file reimport-bug.c
 *
 * Test verifies that we can succesfully reimport and map a DMABUF.  This
 * specifically checks that drivers, which may map a DMABUF, invalidates any
 * such mappings (as needed) when it is reimported. This test has been tuned
 * specifically for the iris driver.
*/

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END


/* dummy */
enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}

static void
pbo_upload_bound_tex()
{
	GLuint pbo;
	glGenBuffersARB(1, &pbo);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER, pbo);
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER, sizeof(uint32_t), NULL,
			GL_STREAM_DRAW_ARB);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA,
			GL_UNSIGNED_BYTE, 0);

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER, 0);
	glDeleteBuffersARB(1, &pbo);
}

static bool
ref_map_unref(struct piglit_dma_buf *buf, int fourcc, bool mark_busy)
{
	enum piglit_result res;

	/* Import DMABUF as EGLImage */
	EGLImageKHR img;
	res = egl_image_for_dma_buf_fd(buf, buf->fd, fourcc, &img);
	if (res != PIGLIT_PASS)
		return false;

	/* Import EGLImage as GL_TEXTURE_2D */
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)img);
	eglDestroyImageKHR(eglGetCurrentDisplay(), img);

	/**
	 * EGL may not support binding of external textures, this is not an
	 * error.
	 */
	GLenum error;
	error = glGetError();
	if (error == GL_INVALID_OPERATION)
		return false;

	if (error != GL_NO_ERROR) {
		printf("glEGLImageTargetTexture2DOES() failed: %s 0x%x\n",
			piglit_get_gl_error_name(error), error);
		return false;
	}

	/* Attempt to make the driver map the buffer object. */
	uint32_t pixels = 0;
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA,
			GL_UNSIGNED_BYTE, &pixels);

	if (mark_busy) {
		/* Attempt to make the driver mark the buffer object as busy.
		 * Avoid using a draw call so that references aren't kept
		 * around longer than we'd like.
		 */
		pbo_upload_bound_tex();
		glFinish();
	}

	/* Delete driver references to the DMABUF */
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &tex);
	return true;
}

void
piglit_init(int argc, char **argv)
{
	EGLDisplay egl_dpy = eglGetCurrentDisplay();
	piglit_require_egl_extension(egl_dpy, "EGL_EXT_image_dma_buf_import");
	piglit_require_egl_extension(egl_dpy, "EGL_KHR_gl_texture_2D_image");

	enum piglit_result res;

	/* Create a DMABUF */
	const uint64_t src = 0;
	const int fourcc = DRM_FORMAT_ABGR8888;
	struct piglit_dma_buf *buf;
	res = piglit_create_dma_buf(1, 2, fourcc, &src, &buf);
	if (res != PIGLIT_PASS)
		piglit_report_result(PIGLIT_SKIP);

	if (!ref_map_unref(buf, fourcc, true)) {
		piglit_destroy_dma_buf(buf);
		piglit_report_result(PIGLIT_SKIP);
	}

	if (!ref_map_unref(buf, fourcc, false)) {
		piglit_destroy_dma_buf(buf);
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_destroy_dma_buf(buf);
	piglit_report_result(PIGLIT_PASS);
}
