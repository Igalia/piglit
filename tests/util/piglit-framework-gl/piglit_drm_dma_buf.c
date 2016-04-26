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
#include <unistd.h>

#define ALIGN(value, alignment) (((value) + alignment - 1) & ~(alignment - 1))

struct piglit_dma_buf {
	unsigned w;
	unsigned h;
	unsigned stride;
	int fd;
	void *priv;
};

struct piglit_drm_driver {
	int fd;
	char *name;

	bool
	(*create)(unsigned w, unsigned h, unsigned cpp,
		  const unsigned char *src_data, unsigned src_stride,
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
piglit_intel_buf_create(unsigned w, unsigned h, unsigned cpp,
			const unsigned char *src_data, unsigned src_stride,
			struct piglit_dma_buf *buf)
{
	unsigned i;
	drm_intel_bo *bo;
	unsigned stride = ALIGN(w * cpp, 4);
	drm_intel_bufmgr *mgr = piglit_intel_bufmgr_get();

	if (!mgr || h % 2)
		return false;

	bo = drm_intel_bo_alloc(mgr, "piglit_dma_buf", h * stride, 4096);
	if (!bo)
		return false;

	for (i = 0; i < h; ++i) {
		if (drm_intel_bo_subdata(bo, i * stride, w * cpp,
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
