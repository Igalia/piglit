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

#include "sample_common.h"
#include "image_common.h"

/**
 * @file export.c
 *
 * Test verifies that we can succesfully export imported dmabuf.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const int format_table[] = {
	 DRM_FORMAT_P010,
	 DRM_FORMAT_P012,
	 DRM_FORMAT_P016,
	 DRM_FORMAT_NV12,
	 DRM_FORMAT_XRGB8888,
	 DRM_FORMAT_ARGB8888,
	 DRM_FORMAT_YUV420,
	 DRM_FORMAT_YVU420,
	 DRM_FORMAT_AYUV,
	 DRM_FORMAT_XYUV8888,
};

static int
fourcc_num_planes(int fourcc)
{
	switch (fourcc) {
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
		return 3;
	case DRM_FORMAT_P010:
	case DRM_FORMAT_P012:
	case DRM_FORMAT_P016:
	case DRM_FORMAT_NV12:
		return 2;
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_AYUV:
	case DRM_FORMAT_XYUV8888:
		return 1;
	};
	return -1;
}

/* dummy */
enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	EGLDisplay egl_dpy = eglGetCurrentDisplay();

	piglit_require_egl_extension(egl_dpy, "EGL_EXT_image_dma_buf_import");
	piglit_require_egl_extension(egl_dpy, "EGL_MESA_image_dma_buf_export");

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
		piglit_report_result(PIGLIT_FAIL);
	}

	for (uint32_t i = 0; i < ARRAY_SIZE(format_table); i++) {

		int fourcc = format_table[i];
		enum piglit_result res;

		/* Create piglit_dma_buf and EGLimage. */
		struct piglit_dma_buf *buf;

		const unsigned char src[] = {
			10, 20, 30, 40,
			50, 60, 70, 80,
			11, 22, 33, 44,
			55, 66, 77, 88 };

		res = piglit_create_dma_buf(2, 2, fourcc, src, &buf);
		if (res != PIGLIT_PASS)
			piglit_report_result(res);

		EGLImageKHR img;
		res = egl_image_for_dma_buf_fd(buf, buf->fd, fourcc, &img);
		if (res != PIGLIT_PASS)
			piglit_report_result(res);

		/* Export the buffer and query properties. */
		int prop_fourcc = -1;
		int num_planes = -1;
		EGLuint64KHR modifiers[64];

		/* Query the image properties, verify fourcc and num planes. */
		if (!dmabuf_query(egl_dpy, img, &prop_fourcc, &num_planes,
				  modifiers)) {
			fprintf(stderr, "export dmabuf image query failed!\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		if (!piglit_check_egl_error(EGL_SUCCESS)) {
			fprintf(stderr, "image export failed!\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		if (prop_fourcc != fourcc) {
			fprintf(stderr,
				"fourcc mismatch, got %d expected %d\n",
				prop_fourcc, fourcc);
			piglit_report_result(PIGLIT_FAIL);
		}

		if (num_planes != fourcc_num_planes(prop_fourcc)) {
			fprintf(stderr, "planes mismatch, got %d expected %d\n",
				num_planes, fourcc_num_planes(prop_fourcc));
			piglit_report_result(PIGLIT_FAIL);
		}

		int *fds = malloc(num_planes * sizeof(int));
		EGLint *strides = malloc(num_planes * sizeof(EGLint));
		EGLint *offsets = malloc(num_planes * sizeof(EGLint));

		/* Export the image, verify success. */
		if (!dmabuf_export(egl_dpy, img, fds, strides, offsets)) {
			fprintf(stderr, "image export failed!\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		if (!piglit_check_egl_error(EGL_SUCCESS)) {
			fprintf(stderr, "image export failed!\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		/* Verify that we got a valid stride and offset for each fd. */
		for (uint32_t i = 0; i < num_planes; i++) {
			if (fds[i] != -1 &&
			    (strides[i] < 1 || offsets[i] < 0)) {
				fprintf(stderr, "invalid data from driver: "
					"format %c%c%c%c, fd %d stride %d "
					"offset %d\n", (fourcc), (fourcc)>>8,
					(fourcc)>>16, (fourcc)>>24, fds[i],
					strides[i], offsets[i]);
				piglit_report_result(PIGLIT_FAIL);
			}
		}

		free(fds);
		free(strides);
		free(offsets);

		piglit_destroy_dma_buf(buf);
	}

	piglit_report_result(PIGLIT_PASS);
}
