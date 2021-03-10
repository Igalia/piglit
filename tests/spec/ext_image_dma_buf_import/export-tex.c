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
 * @file export-tex.c
 *
 * Verifies that we can correctly sample from an image that was:
 *	- Rendered to
 *	- Exported as DMA BUF
 *	- Imported as EGLImage/Texture
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 30;

PIGLIT_GL_TEST_CONFIG_END

#define CLEAR_VALUE 1.0, 1.0, 1.0, 1.0

enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}

static void
tex_clear(GLuint tex, uint32_t w, uint32_t h)
{
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0);

	const float ones[] = { CLEAR_VALUE };
	glClearBufferfv(GL_COLOR, 0, ones);

	glDeleteFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static EGLImageKHR
create_cleared_eglImage(EGLDisplay egl_dpy, int w, int h)
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);

	EGLImageKHR img =
		eglCreateImageKHR(egl_dpy, eglGetCurrentContext(),
				  EGL_GL_TEXTURE_2D_KHR,
				  (EGLClientBuffer)(uintptr_t)tex,
				  NULL);
	tex_clear(tex, w, h);
	glFinish();

	glDeleteTextures(1, &tex);
	return img;
}

static bool
eglImage_to_dma_buf(EGLDisplay egl_dpy, EGLImageKHR img,
		    int *fourcc, int *num_planes, EGLuint64KHR *modifiers,
		    int *fd, EGLint *stride, EGLint *offset)
{
	PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC dmabuf_query;
	PFNEGLEXPORTDMABUFIMAGEMESAPROC dmabuf_export;

	dmabuf_query =
		(PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC) eglGetProcAddress(
			"eglExportDMABUFImageQueryMESA");
	dmabuf_export =
		(PFNEGLEXPORTDMABUFIMAGEMESAPROC) eglGetProcAddress(
			"eglExportDMABUFImageMESA");

	if (!dmabuf_query || !dmabuf_export) {
		fprintf(stderr, "could not find extension entrypoints\n");
		return false;
	}


	/* Query the image properties, verify fourcc and num planes. */
	if (!dmabuf_query(egl_dpy, img, fourcc, num_planes, modifiers))
		return false;

	if (!piglit_check_egl_error(EGL_SUCCESS))
		return false;


	/* Export the image, verify success. */
	if (!dmabuf_export(egl_dpy, img, fd, stride, offset))
		return false;

	if (!piglit_check_egl_error(EGL_SUCCESS))
		return false;

	/* Verify that we got a valid stride and offset for the fd. */
	if (*fd != -1 && (*stride < 1 || *offset < 0)) {
		fprintf(stderr, "invalid data from driver: "
			"fd %d stride %d offset %d\n",
			*fd, *stride, *offset);
		return false;
	}

	return true;
}

static bool
dma_buf_to_eglImage(EGLDisplay egl_dpy, EGLImageKHR *out_img, int w, int h,
		    int fourcc, int num_planes, EGLuint64KHR modifier,
		    int fd, EGLint stride, EGLint offset)
{
	const EGLint attrs[] = {
		EGL_IMAGE_PRESERVED, EGL_TRUE,
		EGL_WIDTH, w,
		EGL_HEIGHT, h,
		EGL_LINUX_DRM_FOURCC_EXT, fourcc,
	        EGL_DMA_BUF_PLANE0_FD_EXT, fd,
	        EGL_DMA_BUF_PLANE0_OFFSET_EXT, offset,
	        EGL_DMA_BUF_PLANE0_PITCH_EXT, stride,
	        EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, modifier,
	        EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, modifier >> 32,
		EGL_NONE, EGL_NONE
	};

	EGLImageKHR img = eglCreateImageKHR(eglGetCurrentDisplay(),
					    EGL_NO_CONTEXT,
					    EGL_LINUX_DMA_BUF_EXT,
					    (EGLClientBuffer)0, attrs);

	if (!piglit_check_egl_error(EGL_SUCCESS)) {
		*out_img = NULL;
		return false;
	}

	*out_img = img;
	return true;
}

void
piglit_init(int argc, char **argv)
{
	EGLDisplay egl_dpy = eglGetCurrentDisplay();

	piglit_require_egl_extension(egl_dpy, "EGL_MESA_image_dma_buf_export");
	piglit_require_egl_extension(egl_dpy, "EGL_EXT_image_dma_buf_import");
	piglit_require_egl_extension(egl_dpy, "EGL_KHR_gl_texture_2D_image");
	piglit_require_extension("GL_OES_EGL_image_external");

	/* Create EGLImage */
	const int w = 128;
	const int h = 32;
	EGLImageKHR img = create_cleared_eglImage(egl_dpy, w, h);

	/* Export DMABUF from EGLImage */
	int fourcc = -1;
	int num_planes = -1;
	EGLuint64KHR modifiers[64] = { -1, };
	int fd = -1;
	EGLint stride = -1;
	EGLint offset = -1;
	if (!eglImage_to_dma_buf(egl_dpy, img, &fourcc, &num_planes,
				 modifiers, &fd, &stride, &offset)) {
		fprintf(stderr, "image export failed!\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (num_planes != 1) {
		fprintf(stderr, "Test only supports single plane\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	/* Delete the EGLImage. */
	eglDestroyImageKHR(egl_dpy, img);

	/* Import EGLImage from DMABUF */
	EGLImageKHR imported_img;
	if (!dma_buf_to_eglImage(egl_dpy, &imported_img, w, h,
				 fourcc, num_planes,
				 modifiers[0], fd, stride, offset)) {
		fprintf(stderr, "dmabuf import failed!\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Draw EGLImage contents */
	GLuint imported_tex;
	texture_for_egl_image(imported_img, &imported_tex);
	sample_tex(imported_tex, 0, 0, 1, piglit_height);

	/* Verify the contents */
	const float ones[] = { CLEAR_VALUE };
	if (piglit_probe_pixel_rgba(0, 0, ones))
		piglit_report_result(PIGLIT_PASS);
	else
		piglit_report_result(PIGLIT_FAIL);

}
