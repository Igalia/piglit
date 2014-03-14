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
#include "piglit-util-cl.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clEnqueueCopyBufferRect";
	config.version_min = 10;

	config.run_per_platform = true;
	config.create_context = true;

PIGLIT_CL_API_TEST_CONFIG_END

#define BUFFER_SIZE 672

enum piglit_result
piglit_cl_test(const int argc,
	       const char **argv,
	       const struct piglit_cl_api_test_config* config,
	       const struct piglit_cl_api_test_env* env)
{
	char *host_src_buffer = malloc(BUFFER_SIZE);
	char *host_dst_buffer = malloc(BUFFER_SIZE);
	cl_mem device_src_buffer, device_dst_buffer;
	cl_command_queue queue = env->context->command_queues[0];
	cl_int err;
	int i;
	size_t src_origin[3] = {0, 0, 0};
	size_t dst_origin[3] = {1, 0, 0};
	size_t region[3] = {1, 21, 1};
	size_t src_row_pitch = 32;
	size_t src_slice_pitch = 0;
	size_t dst_row_pitch = 32;
	size_t dst_slice_pitch = 0;

	memset(host_src_buffer, 0, BUFFER_SIZE);
	memset(host_dst_buffer, 0xff, BUFFER_SIZE);

	device_src_buffer = piglit_cl_create_buffer(
		env->context, CL_MEM_READ_WRITE, BUFFER_SIZE);
	device_dst_buffer = piglit_cl_create_buffer(
		env->context, CL_MEM_READ_WRITE, BUFFER_SIZE);
	if (!piglit_cl_write_whole_buffer(queue,
					device_src_buffer, host_src_buffer) ||
	    !piglit_cl_write_whole_buffer(queue,
					device_dst_buffer, host_dst_buffer)) {
		return PIGLIT_FAIL;
	}

	err = clEnqueueCopyBufferRect(queue, device_src_buffer, device_dst_buffer,
				src_origin, dst_origin, region, src_row_pitch,
				src_slice_pitch, dst_row_pitch, dst_slice_pitch,
				0, NULL, NULL);
	if (!piglit_cl_check_error(err, CL_SUCCESS)) {
		return PIGLIT_FAIL;
	}

	clFinish(queue);

	if (!piglit_cl_read_whole_buffer(queue, device_dst_buffer,
							host_dst_buffer)) {
		return PIGLIT_FAIL;
	}

	// Check that bytes were written to the correct locations.
	for (i = 0; i < region[1]; i++) {
		unsigned dst_idx = (i *  dst_row_pitch) + dst_origin[0];
		char dst_value = host_dst_buffer[dst_idx];
		char src_value = host_src_buffer[(i * src_row_pitch) + src_origin[0]];
		if (!piglit_cl_probe_integer(dst_value, src_value, 0)){
			printf("Error at %d\n", dst_idx);
			return PIGLIT_FAIL;
		}
	}

	// Check that bytes weren't written to any wrong places
	for (i = 0; i < BUFFER_SIZE; i++) {
		if (i < dst_origin[0] || (i - dst_origin[0]) % dst_row_pitch == 0) {
			continue;
		}
		if (!piglit_cl_probe_integer(host_dst_buffer[i], (char)0xff, 0)) {
			printf("Error at %d\n", i);
			return PIGLIT_FAIL;
		}
	}
	return PIGLIT_PASS;
}
