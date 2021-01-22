/* Copyright (c) 2021 Collabora Ltd
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

/**
 * \file
 * \brief Tests for libgbm.
 */

#include <drm_fourcc.h>
#include <errno.h>
#include <fcntl.h>
#include <gbm.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "piglit-util.h"

#define NUM_PLANES 3
#define WIDTH 512
#define HEIGHT 512
#define FORMAT GBM_FORMAT_R8

static bool
gem_handles_match(struct gbm_device *gbm, int fd, int old_handle)
{
	bool match;
	struct gbm_import_fd_data import = {
		.width = WIDTH,
		.height = HEIGHT,
		.format = FORMAT,
		.fd = fd,
	};
	struct gbm_bo *bo = gbm_bo_import(gbm, GBM_BO_IMPORT_FD, &import, 0);
	if (!bo)
		piglit_report_result(PIGLIT_FAIL);

	match = gbm_bo_get_handle(bo).u32 == old_handle;
	gbm_bo_destroy(bo);

	return match;
}

int
main(int argc, char **argv)
{
	int drm_fd;
	char *nodename;
	struct gbm_device *gbm;
	struct gbm_bo *bos[NUM_PLANES];
	struct gbm_bo *multi_plane_bo;
	struct gbm_import_fd_modifier_data import_mod = {
		.width = WIDTH,
		.height = HEIGHT,
		.format = GBM_FORMAT_YUV420,
		.num_fds = NUM_PLANES,
		.modifier = DRM_FORMAT_MOD_LINEAR,
	};

	/* Strip common piglit args. */
	piglit_strip_arg(&argc, argv, "-fbo");
	piglit_strip_arg(&argc, argv, "-auto");

	nodename = getenv("WAFFLE_GBM_DEVICE");
	if (!nodename)
		nodename = "/dev/dri/renderD128";
	drm_fd = open(nodename, O_RDWR);
	if (drm_fd == -1) {
		perror("Error opening render node");
		piglit_report_result(PIGLIT_FAIL);
	}

	gbm = gbm_create_device(drm_fd);
	if (!gbm)
		piglit_report_result(PIGLIT_FAIL);

	for (int i = 0; i < NUM_PLANES; i++) {
		bos[i] = gbm_bo_create(gbm, WIDTH, HEIGHT, FORMAT,
			GBM_BO_USE_RENDERING | GBM_BO_USE_LINEAR);
		if (!bos[i])
			piglit_report_result(PIGLIT_FAIL);

		import_mod.fds[i] = gbm_bo_get_fd(bos[i]);
		import_mod.strides[i] = gbm_bo_get_stride(bos[i]);
		import_mod.offsets[i] = gbm_bo_get_offset(bos[i], 0);
	}

	multi_plane_bo = gbm_bo_import(gbm, GBM_BO_IMPORT_FD_MODIFIER,
		&import_mod, 0);
	if (!multi_plane_bo)
		piglit_report_result(PIGLIT_FAIL);

	for (int i = 0; i < NUM_PLANES; i++) {
		int fd = gbm_bo_get_fd_for_plane(multi_plane_bo, i);
		if (fd < 0)
			piglit_report_result(PIGLIT_FAIL);

		if (fcntl(fd, F_GETFL) == -1 && errno == EBADF)
			piglit_report_result(PIGLIT_FAIL);

		if (!gem_handles_match(gbm, fd, gbm_bo_get_handle(bos[i]).u32))
			piglit_report_result(PIGLIT_FAIL);

		if (import_mod.strides[i] != gbm_bo_get_stride_for_plane(multi_plane_bo, i))
			piglit_report_result(PIGLIT_FAIL);

		if (import_mod.offsets[i] != gbm_bo_get_offset(multi_plane_bo, i))
			piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}
