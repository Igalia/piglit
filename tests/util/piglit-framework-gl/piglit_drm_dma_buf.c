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

#include "piglit-util-gl-common.h"
#include "piglit_drm_dma_buf.h"
#ifdef HAVE_LIBDRM_INTEL
#include <libdrm/intel_bufmgr.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <xf86drm.h>
#include <stdbool.h>
#include <xcb/dri2.h>
#include <drm.h>

static const char *drm_device_filename = "/dev/dri/card0";

#define ALIGN(value, alignment) (((value) + alignment - 1) & ~(alignment - 1))

struct piglit_dma_buf {
	unsigned w;
	unsigned h;
	unsigned stride;
	int fd;
	void *priv;
};

struct piglit_drm_driver {
	const char *name;

	bool
	(*create)(unsigned w, unsigned h, unsigned cpp,
		  const unsigned char *src_data, unsigned src_stride,
		  struct piglit_dma_buf *buf);

	bool
	(*export)(struct piglit_dma_buf *buf);

	void
	(*destroy)(struct piglit_dma_buf *buf);
};

static int
piglit_drm_device_get(void)
{
	static int fd = 0;

	if (fd)
		return fd;

	fd = open(drm_device_filename, O_RDWR);

	return fd;
}

static bool
piglit_drm_x11_authenticate(void)
{
	drm_magic_t magic;
	xcb_connection_t *conn;
	int screen;
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

	ret = drmGetMagic(piglit_drm_device_get(), &magic);
	if (ret) {
		printf("piglit: failed to get DRM magic\n");
		return false;
	}

	screen_iter = xcb_setup_roots_iterator(xcb_get_setup(conn));
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
	static drm_intel_bufmgr *mgr = NULL;

	if (mgr)
		return mgr;

	if (!piglit_drm_device_get())
		return NULL;

	if (!piglit_drm_x11_authenticate())
		return NULL;

	mgr = intel_bufmgr_gem_init(piglit_drm_device_get(), batch_sz);

	return mgr;
}

static bool
piglit_intel_buf_create(unsigned w, unsigned h, unsigned cpp,
			const unsigned char *src_data, unsigned src_stride,
			struct piglit_dma_buf *buf)
{
	unsigned i;
	drm_intel_bo *bo;
	unsigned stride = ALIGN(w * cpp, 4);
	drm_intel_bufmgr *mgr = piglit_intel_bufmgr_get();

	if (!mgr || src_stride > stride || h % 2)
		return false;

	bo = drm_intel_bo_alloc(mgr, "piglit_dma_buf", h * stride, 4096);
	if (!bo)
		return false;

	for (i = 0; i < h; ++i) {
		if (drm_intel_bo_subdata(bo, i * stride, src_stride,
			src_data + i * src_stride)) {
			drm_intel_bo_unreference(bo);
			return false;
		}
	}

	buf->w = w;
	buf->h = h;
	buf->stride = stride;
	buf->fd = 0;
	buf->priv = bo;

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

/**
 * The framework makes sure one doesn't try to compile without any hardware
 * support.
 */
static const struct piglit_drm_driver piglit_drm_drivers[] = {
#ifdef HAVE_LIBDRM_INTEL
	{ "i915", piglit_intel_buf_create, piglit_intel_buf_export,
	   piglit_intel_buf_destroy }
#endif /* HAVE_LIBDRM_INTEL */
};

static const struct piglit_drm_driver *
piglit_drm_get_driver(void)
{
	unsigned i;
	drmVersionPtr version;
	int fd = piglit_drm_device_get();

	if (!fd)
		return NULL;

	version = drmGetVersion(fd);
	if (!version || !version->name)
		return NULL;

	for (i = 0; i < ARRAY_SIZE(piglit_drm_drivers); ++i) {
		if (strcmp(piglit_drm_drivers[i].name, version->name) == 0)
			return &piglit_drm_drivers[i];
	}

	return NULL;
}

enum piglit_result
piglit_drm_create_dma_buf(unsigned w, unsigned h, unsigned cpp,
			const void *src_data, unsigned src_stride,
			struct piglit_dma_buf **buf, int *fd,
			unsigned *stride, unsigned *offset)
{
	struct piglit_dma_buf *drm_buf;
	const struct piglit_drm_driver *drv = piglit_drm_get_driver();

	if (!drv)
		return PIGLIT_SKIP;

	drm_buf = calloc(sizeof(struct piglit_dma_buf), 1);
	if (!drm_buf)
		return PIGLIT_FAIL;

	if (!drv->create(w, h, cpp, src_data, src_stride, drm_buf)) {
		free(drm_buf);
		return PIGLIT_FAIL;
	}

	if (!drv->export(drm_buf)) {
		free(drm_buf);
		return PIGLIT_FAIL;
	}

	*buf = drm_buf;
	*fd = drm_buf->fd;
	*stride = drm_buf->stride;
	*offset = 0;

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
