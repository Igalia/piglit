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

#include "piglit-util-gl.h"
#include "piglit_drm_dma_buf.h"
#include "drm_fourcc.h"
#ifdef HAVE_LIBDRM_INTEL
#include <libdrm/intel_bufmgr.h>
#endif
#ifdef PIGLIT_HAS_GBM_BO_MAP
#include <gbm.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <xf86drm.h>
#include <stdbool.h>
#include <xcb/dri2.h>
#include <drm.h>
#include <unistd.h>

#define ALIGN(value, alignment) (((value) + alignment - 1) & ~(alignment - 1))

struct piglit_drm_driver {
	int fd;
	char *name;

	bool
	(*create)(unsigned w, unsigned h, unsigned cpp,
		  const unsigned char *src_data,
		  struct piglit_dma_buf *buf);

	bool
	(*export)(struct piglit_dma_buf *buf);

	void
	(*destroy)(struct piglit_dma_buf *buf);
};

static const struct piglit_drm_driver *piglit_drm_get_driver(void);

static bool
piglit_drm_x11_authenticate(int fd)
{
	drm_magic_t magic;
	xcb_connection_t *conn;
	int screen;
	const xcb_setup_t *setup;
	xcb_screen_iterator_t screen_iter;
	xcb_dri2_authenticate_cookie_t auth_cookie;
	xcb_dri2_authenticate_reply_t *auth_reply;
	int ret = 0;

	conn = xcb_connect(NULL, &screen);
	if (!conn) {
		printf("piglit: failed to connect to X server for DRI2 "
		       "authentication\n");
		return false;
	}

	ret = drmGetMagic(fd, &magic);
	if (ret) {
		printf("piglit: failed to get DRM magic\n");
		return false;
	}

	setup = xcb_get_setup(conn);
	if (!setup) {
		printf("piglit: xcb_get_setup() failed\n");
		return false;
	}

	screen_iter = xcb_setup_roots_iterator(setup);
	auth_cookie = xcb_dri2_authenticate_unchecked(conn,
	                                              screen_iter.data->root,
	                                              magic);
	auth_reply = xcb_dri2_authenticate_reply(conn, auth_cookie, NULL);

	if (auth_reply == NULL || !auth_reply->authenticated) {
		printf("piglit: failed to authenticate with DRI2\n");
		return false;
	}
	free(auth_reply);

	return true;
}

#ifdef HAVE_LIBDRM_INTEL
static drm_intel_bufmgr *
piglit_intel_bufmgr_get(void)
{
	static const unsigned batch_sz = 8192 * sizeof(uint32_t);
	const struct piglit_drm_driver *drv = piglit_drm_get_driver();
	static drm_intel_bufmgr *mgr = NULL;

	if (mgr)
		return mgr;

	if (!drv)
		return NULL;

	mgr = intel_bufmgr_gem_init(drv->fd, batch_sz);

	return mgr;
}

static bool
piglit_intel_buf_create(unsigned w, unsigned h, unsigned fourcc,
			const unsigned char *src_data, struct piglit_dma_buf *buf)
{
	unsigned i;
	drm_intel_bo *bo;
	drm_intel_bufmgr *mgr = piglit_intel_bufmgr_get();
	unsigned stride, src_stride, cpp;
	unsigned buf_h = h;

	if (!mgr || h % 2)
		return false;

	switch (fourcc) {
	case DRM_FORMAT_R8:
		cpp = 1;
		break;
	case DRM_FORMAT_GR88:
	case DRM_FORMAT_RG88:
		cpp = 2;
		break;
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_BGRX8888:
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_BGRA8888:
	case DRM_FORMAT_AYUV:
	case DRM_FORMAT_XYUV8888:
		cpp = 4;
		break;
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
		cpp = 1;
		buf_h = h * 3 / 2;
		break;
	case DRM_FORMAT_P010:
	case DRM_FORMAT_P012:
	case DRM_FORMAT_P016:
		cpp = 2;
		buf_h = h * 3 / 2;
		break;
	default:
		fprintf(stderr, "invalid fourcc: %.4s\n", (char *)&fourcc);
		return false;
	}

	src_stride = cpp * w;
	stride = ALIGN(w * cpp, 4);

	bo = drm_intel_bo_alloc(mgr, "piglit_dma_buf", buf_h * stride, 4096);
	if (!bo)
		return false;

	for (i = 0; i < buf_h; ++i) {
		if (drm_intel_bo_subdata(bo, i * stride, w * cpp,
			src_data + i * src_stride)) {
			drm_intel_bo_unreference(bo);
			return false;
		}
	}

	buf->w = w;
	buf->h = h;
	buf->offset[0] = 0;
	buf->stride[0] = stride;
	buf->fd = 0;
	buf->priv = bo;

	switch (fourcc) {
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_P010:
	case DRM_FORMAT_P012:
	case DRM_FORMAT_P016:
		buf->offset[1] = stride * h;
		buf->stride[1] = stride;
		break;
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
		buf->offset[1] = stride * h;
		buf->stride[1] = stride / 2;
		buf->offset[2] = buf->offset[1] + (stride * h / 2 / 2);
		buf->stride[2] = stride / 2;
		break;
	default:
		break;
	}

	return true;
}

static bool
piglit_intel_buf_export(struct piglit_dma_buf *buf)
{
	if (drm_intel_bo_gem_export_to_prime((drm_intel_bo *)buf->priv,
					&buf->fd)) {
		drm_intel_bo_unreference((drm_intel_bo *)buf->priv);
		return false;
	}

	return true;
}

static void
piglit_intel_buf_destroy(struct piglit_dma_buf *buf)
{
	drm_intel_bo_unreference((drm_intel_bo *)buf->priv);
}
#endif /* HAVE_LIBDRM_INTEL */

#ifdef PIGLIT_HAS_GBM_BO_MAP
static struct gbm_device *
piglit_gbm_get(void)
{
	const struct piglit_drm_driver *drv = piglit_drm_get_driver();
	static struct gbm_device *gbm = NULL;

	if (gbm)
		return gbm;

	gbm = gbm_create_device(drv->fd);

	return gbm;
}

static bool
piglit_gbm_buf_create(unsigned w, unsigned h, unsigned fourcc,
			const unsigned char *src_data, struct piglit_dma_buf *buf)
{
	unsigned i;
	struct gbm_bo *bo;
	uint32_t dst_stride;
	struct gbm_device *gbm = piglit_gbm_get();
	void *dst_data;
	void *map_data = NULL;
	enum gbm_bo_format format;
	unsigned cpp = 0, src_stride;
	unsigned buf_w = w;
	unsigned buf_h = h;

	if (!gbm || h % 2 || w % 2)
		return false;

	switch (fourcc) {
	case DRM_FORMAT_R8:
		format = GBM_FORMAT_R8;
		cpp = 1;
		src_stride = cpp * w;
		break;
	case DRM_FORMAT_GR88:
	case DRM_FORMAT_RG88:
		format = GBM_FORMAT_GR88;
		cpp = 2;
		src_stride = cpp * w;
		break;
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_BGRX8888:
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_BGRA8888:
		format = GBM_BO_FORMAT_ARGB8888;
		cpp = 4;
		src_stride = cpp * w;
		break;
	/* For YUV formats, the U/V planes might have a greater relative
	 * pitch.  For example, if the driver needs pitch aligned to 32
	 * pixels, for a 4x4 YUV image, the stride of both the Y and U/V
	 * planes will be 32 bytes.  Not 32 for Y and 16 for U/V.  To
	 * account for this, use a 2cpp format with half the width.  For
	 * hardware that only has stride requirements in bytes (rather
	 * than pixels) this will work out the same.  For hardware that
	 * has pitch alignment requirements in pixels, this will give an
	 * overly conservative alignment for Y but a sufficient alignment
	 * for U/V.
	 */
	case DRM_FORMAT_NV12:
		format = GBM_FORMAT_GR88;
		buf_w = w / 2;
		buf_h = h * 3 / 2;
		src_stride = w;
		cpp = 1;
		break;
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
		format = GBM_FORMAT_GR88;
		buf_w = w / 2;
		buf_h = h * 2;    /* U/V not interleaved */
		src_stride = w;
		cpp = 1;
		break;
	default:
		fprintf(stderr, "invalid fourcc: %.4s\n", (char *)&fourcc);
		return false;
	}

	bo = gbm_bo_create(gbm, buf_w, buf_h, format, GBM_BO_USE_RENDERING);
	if (!bo)
		return false;

	dst_data = gbm_bo_map(bo, 0, 0, buf_w, buf_h, GBM_BO_TRANSFER_WRITE,
			      &dst_stride, &map_data);
	if (!dst_data) {
		fprintf(stderr, "Failed to map GBM bo\n");
		gbm_bo_destroy(bo);
		return NULL;
	}

	buf->w = w;
	buf->h = h;
	buf->offset[0] = gbm_bo_get_offset(bo, 0);
	buf->stride[0] = gbm_bo_get_stride_for_plane(bo, 0);
	buf->fd = -1;
	buf->priv = bo;

	for (i = 0; i < h; ++i) {
		memcpy((char *)dst_data + i * dst_stride,
		       src_data + i * src_stride,
		       w * cpp);
	}

	switch (fourcc) {
	case DRM_FORMAT_NV12:
		for (i = 0; i < h/2; ++i) {
			memcpy(((char *)dst_data + dst_stride * h) + i * dst_stride,
				(src_data + (w*h)) + i * src_stride, w);
		}
		buf->offset[1] = gbm_bo_get_offset(bo, 1);
		buf->stride[1] = gbm_bo_get_stride_for_plane(bo, 1);
		break;
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
		for (i = 0; i < h/2; ++i) {
			memcpy(((char *)dst_data + dst_stride * h) + i * dst_stride / 2,
				(src_data + (w*h)) + i * src_stride / 2, w / 2);
		}
		unsigned cpu_offset2 = dst_stride * h + (dst_stride * h / 2 / 2);
		for (i = 0; i < h/2; ++i) {
			memcpy(((char *)dst_data + cpu_offset2) + i * dst_stride / 2,
				(src_data + (w*h) + (w*h/4)) + i * src_stride / 2, w / 2);
		}
		buf->offset[1] = gbm_bo_get_offset(bo, 1);
		buf->stride[1] = gbm_bo_get_stride_for_plane(bo, 1);
		buf->offset[2] = gbm_bo_get_offset(bo, 2);
		buf->stride[2] = gbm_bo_get_stride_for_plane(bo, 2);
		break;
	default:
		break;
	}

	gbm_bo_unmap(bo, map_data);


	return true;
}

static bool
piglit_gbm_buf_export(struct piglit_dma_buf *buf)
{
	struct gbm_bo *bo = buf->priv;

	buf->fd = gbm_bo_get_fd(bo);

	return buf->fd >= 0;
}

static void
piglit_gbm_buf_destroy(struct piglit_dma_buf *buf)
{
	struct gbm_bo *bo = buf->priv;

	gbm_bo_destroy(bo);
}
#endif /* PIGLIT_HAS_GBM_BO_MAP */

static const struct piglit_drm_driver *
piglit_drm_get_driver(void)
{
	static struct piglit_drm_driver drv = { /* fd */ -1 };
	drmVersionPtr version;

	if (drv.fd != -1)
		return &drv;

	drv.fd = open("/dev/dri/renderD128", O_RDWR);

	if (drv.fd == -1) {
		drv.fd = open("/dev/dri/card0", O_RDWR);
		if (drv.fd == -1) {
			fprintf(stderr, "error: failed to open /dev/dri/renderD128 and "
			        "/dev/dri/card0\n");
			goto fail;

		}

		if (!piglit_drm_x11_authenticate(drv.fd))
			goto fail;
	}

	version = drmGetVersion(drv.fd);
	if (!version || !version->name) {
		fprintf(stderr, "error: drmGetVersion() failed\n");
		goto fail;
	}

	drv.name = strdup(version->name);
	if (!drv.name) {
		fprintf(stderr, "out of memory\n");
		abort();
	}

	if (0) {
		/* empty */
	}
#ifdef HAVE_LIBDRM_INTEL
	else if (streq(version->name, "i915")) {
		drv.create = piglit_intel_buf_create;
		drv.export = piglit_intel_buf_export;
		drv.destroy = piglit_intel_buf_destroy;
	}
#endif
#ifdef PIGLIT_HAS_GBM_BO_MAP
	else if (true) {
		drv.create = piglit_gbm_buf_create;
		drv.export = piglit_gbm_buf_export;
		drv.destroy = piglit_gbm_buf_destroy;
	}
#endif
	else {
		fprintf(stderr, "error: unrecognized DRM driver name %s\n",
			version->name);
		goto fail;
	}

	return &drv;

  fail:
	if (drv.fd != -1) {
		close(drv.fd);
		drv.fd = -1;
	}

	if (drv.name) {
		free(drv.name);
		drv.name = NULL;
	}

	return NULL;
}

enum piglit_result
piglit_drm_create_dma_buf(unsigned w, unsigned h, unsigned fourcc,
			const void *src_data,  struct piglit_dma_buf **buf)
{
	struct piglit_dma_buf *drm_buf;
	const struct piglit_drm_driver *drv = piglit_drm_get_driver();

	if (!drv)
		return PIGLIT_SKIP;

	drm_buf = calloc(sizeof(struct piglit_dma_buf), 1);
	if (!drm_buf)
		return PIGLIT_FAIL;

	if (!drv->create(w, h, fourcc, src_data, drm_buf)) {
		free(drm_buf);
		return PIGLIT_FAIL;
	}

	if (!drv->export(drm_buf)) {
		free(drm_buf);
		return PIGLIT_FAIL;
	}

	*buf = drm_buf;

	return PIGLIT_PASS;
}

void
piglit_drm_destroy_dma_buf(struct piglit_dma_buf *buf)
{
	const struct piglit_drm_driver *drv;

	if (!buf)
		return;

	drv = piglit_drm_get_driver();
	if (!drv)
		return;

	drv->destroy(buf);
	free(buf);
}
