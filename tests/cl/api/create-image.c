/*
 * Copyright 2014 Advanced Micro Devices, Inc.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors: Tom Stellard <thomas.stellard@amd.com>
 *
 */

#include "piglit-framework-cl-api.h"

PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clCreateImage";
	config.version_min = 10;

	config.run_per_platform = true;
	config.create_context = true;

PIGLIT_CL_API_TEST_CONFIG_END

static bool context_has_image_support(const piglit_cl_context ctx)
{
	unsigned i;
	for(i = 0; i < ctx->num_devices; i++) {
		int *image_support =
			piglit_cl_get_device_info(ctx->device_ids[i],
						CL_DEVICE_IMAGE_SUPPORT);
		if (*image_support) {
			return true;
		}
	}
	return false;
}

static void
no_image_check_invalid(
	cl_int errcode_ret,
	enum piglit_result *result,
	const char *name)
{
	if (!piglit_cl_check_error(errcode_ret, CL_INVALID_OPERATION)) {
		fprintf(stderr, "%s: CL_INVALID_OPERATION expected when no "
				"devices support images.\n", name);
		piglit_merge_result(result, PIGLIT_FAIL);
	}
}

static enum piglit_result
no_image_tests(const struct piglit_cl_api_test_env* env)
{
	enum piglit_result result = PIGLIT_PASS;
	cl_context cl_ctx = env->context->cl_ctx;
	cl_mem_flags flags = CL_MEM_READ_ONLY;
	cl_image_format image_format;
	size_t image_width = 1;
	size_t image_height = 1;
	size_t image_depth = 2;
	size_t image_row_pitch = 0;
	size_t image_slice_pitch = 0;
	void *host_ptr = NULL;
	cl_int errcode_ret;

	image_format.image_channel_order = CL_RGBA;
	image_format.image_channel_data_type = CL_FLOAT;

	clCreateImage2D(cl_ctx, flags, &image_format, image_width,
			image_height, image_row_pitch, host_ptr,
			&errcode_ret);

	no_image_check_invalid(errcode_ret, &result, "clCreateImage2D");

	clCreateImage3D(cl_ctx, flags, &image_format, image_width,
			image_height, image_depth, image_row_pitch,
			image_slice_pitch, host_ptr, &errcode_ret);

	no_image_check_invalid(errcode_ret, &result, "clCreateImage3D");

	return result;
}

enum piglit_result
piglit_cl_test(const int argc,
               const char **argv,
               const struct piglit_cl_api_test_config* config,
               const struct piglit_cl_api_test_env* env)
{
	if (!context_has_image_support(env->context)) {
		return no_image_tests(env);
	} else {
		return PIGLIT_PASS;
	}
}
