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

	config.name = "clCreateSampler";
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

static enum piglit_result
no_image_tests(const struct piglit_cl_api_test_env* env)
{
	enum piglit_result result = PIGLIT_PASS;
	cl_context cl_ctx = env->context->cl_ctx;
	cl_bool normalized_coords = false;
	cl_addressing_mode addressing_mode = CL_ADDRESS_NONE;
	cl_filter_mode filter_mode = CL_FILTER_NEAREST;
	cl_int errcode_ret;

	clCreateSampler(cl_ctx, normalized_coords, addressing_mode, filter_mode,
			&errcode_ret);

	if (!piglit_cl_check_error(errcode_ret, CL_INVALID_OPERATION)) {
		fprintf(stderr, "clCreateSampler: CL_INVALID_OPERATION "
				"expected when no devices support images.\n");
		piglit_merge_result(&result, PIGLIT_FAIL);
	}

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
