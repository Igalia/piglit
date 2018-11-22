/*
 * Copyright 2013 Advanced Micro Devices, Inc.
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

/* The r600g driver stores all global buffers in a single memory pool and
 * has a simple memory manager to allocate and deallocate buffers in the pool.
 * This test was inspired by a bug in the allocator where assigning a new
 * buffer the lowest offset in the pool, would delete the buffer which
 * previously had the lowest offset.
 */

#include "piglit-framework-cl-custom.h"

PIGLIT_CL_CUSTOM_TEST_CONFIG_BEGIN

	config.name = "clFlush() after clEnqueueNDRangeKernel()";
	config.run_per_device = true;

PIGLIT_CL_CUSTOM_TEST_CONFIG_END

#define BUFFER_SIZE 1024 * 1024

char *source =
"kernel void test(global int *out) { *out = 1; }\n";

enum piglit_result
piglit_cl_test(const int argc,
	       const char **argv,
	       const struct piglit_cl_custom_test_config *config,
	       const struct piglit_cl_custom_test_env *env)
{
	size_t global_size = 1, local_size = 1;
	piglit_cl_context context = NULL;
	cl_command_queue queue = NULL;
	cl_mem buffer0 = NULL, buffer1 = NULL, buffer2 = NULL, buffer3 = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	int data[BUFFER_SIZE / sizeof(int)];
	unsigned i;

	context = piglit_cl_create_context(env->platform_id, &env->device_id, 1);
	queue = context->command_queues[0];

	buffer0 = piglit_cl_create_buffer(context, CL_MEM_WRITE_ONLY, BUFFER_SIZE);
	buffer1 = piglit_cl_create_buffer(context, CL_MEM_WRITE_ONLY, BUFFER_SIZE);

	program = piglit_cl_build_program_with_source(context, 1, &source, "");
	kernel = piglit_cl_create_kernel(program, "test");

	/* Use the first buffer */
	if (!piglit_cl_set_kernel_arg(kernel, 0, sizeof(cl_mem), &buffer0)) {
		return PIGLIT_FAIL;
	}

	if (!piglit_cl_enqueue_ND_range_kernel(queue, kernel, 1, NULL,
	                                       &global_size, &local_size,
					       NULL)) {
		return PIGLIT_FAIL;
	}

	/* Use the second buffer */
	if (!piglit_cl_set_kernel_arg(kernel, 0, sizeof(cl_mem), &buffer1)) {
		return PIGLIT_FAIL;
	}

	if (!piglit_cl_enqueue_ND_range_kernel(queue, kernel, 1, NULL,
	                                       &global_size, &local_size,
					       NULL)) {
		return PIGLIT_FAIL;
	}

	/* Delete the first buffer */
	clReleaseMemObject(buffer0);

	/* Create and use the third buffer */
	buffer2 = piglit_cl_create_buffer(context, CL_MEM_WRITE_ONLY, BUFFER_SIZE);
	if (!piglit_cl_set_kernel_arg(kernel, 0, sizeof(cl_mem), &buffer2)) {
		return PIGLIT_FAIL;
	}

	if (!piglit_cl_enqueue_ND_range_kernel(queue, kernel, 1, NULL,
	                                       &global_size, &local_size,
					       NULL)) {
		return PIGLIT_FAIL;
	}

	/* Create the fourth buffer. */
	buffer3 = piglit_cl_create_buffer(context, CL_MEM_WRITE_ONLY, BUFFER_SIZE);

	/* At this point, the bug in r600g will cause buffer3 and buffer1 to
	 * have the same offset, so if we write to buffer3, then the data
	 * will appear in buffer1.
	 */

	/* Clear both buffers */
	memset(data, 0, sizeof(data));
	piglit_cl_write_whole_buffer(queue, buffer1, data);
	piglit_cl_write_whole_buffer(queue, buffer3, data);

	/* Write data to buffer1 */
	memset(data, 0xff, sizeof(data));
	piglit_cl_write_whole_buffer(queue, buffer3, data);

	/* Check that the data wasn't also written to buffer1 */
	memset(data, 0, sizeof(data));
	if (!piglit_cl_read_whole_buffer(queue, buffer1, data)) {
		return PIGLIT_FAIL;
	}

	for (i = 0; i < BUFFER_SIZE / sizeof(int); i++) {
		if (data[i]) {
			fprintf(stderr, "Error at data[%u]\n", i);
			return PIGLIT_FAIL;
		}
	}
	return PIGLIT_PASS;
}
