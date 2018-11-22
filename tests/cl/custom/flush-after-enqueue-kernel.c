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

#include "piglit-framework-cl-custom.h"

PIGLIT_CL_CUSTOM_TEST_CONFIG_BEGIN

	config.name = "clFlush() after clEnqueueNDRangeKernel()";
	config.run_per_device = true;

PIGLIT_CL_CUSTOM_TEST_CONFIG_END

#define BUFFER_ELTS 1024

/* The goal of this kernel is to run long enough, so that it will still be
 * running when we call clEnqueueReadBuffer().  It will be submitted with a
 * large number of work items.
 */
char *source =
"kernel void test (global int *out, global int *in) {\n"
"	unsigned i;\n"
"	for (i = 0; i < BUFFER_ELTS; i++) {\n"
"		out[i] = in[i];\n"
"	}\n"
"}";


enum piglit_result
piglit_cl_test(const int argc,
	       const char **argv,
	       const struct piglit_cl_custom_test_config *config,
	       const struct piglit_cl_custom_test_env *env)
{
	size_t global_size[3] = {16, 16, 16};
	size_t local_size[3]  = {1, 1, 1};

	int  in_data[BUFFER_ELTS];
	int out_data[BUFFER_ELTS];

	piglit_cl_context context = NULL;
	cl_mem in_buffer = NULL, out_buffer = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	char  compile_opts[100];

	unsigned i;

	context = piglit_cl_create_context(env->platform_id, &env->device_id, 1);

	memset(in_data, 1, sizeof(in_data));
	in_buffer = piglit_cl_create_buffer(context, CL_MEM_READ_ONLY, sizeof(in_data));
	piglit_cl_write_whole_buffer(context->command_queues[0], in_buffer, in_data);

	memset(out_data, 0, sizeof(out_data));
	out_buffer = piglit_cl_create_buffer(context, CL_MEM_WRITE_ONLY, sizeof(out_data));
	piglit_cl_write_whole_buffer(context->command_queues[0], out_buffer, out_data);

	sprintf(compile_opts, "-DBUFFER_ELTS=%d", BUFFER_ELTS);

	program = piglit_cl_build_program_with_source(context, 1, &source,
								compile_opts);
	kernel = piglit_cl_create_kernel(program, "test");

	if (!piglit_cl_set_kernel_arg(kernel, 0, sizeof(cl_mem), &out_buffer)) {
		return PIGLIT_FAIL;
	}
	if (!piglit_cl_set_kernel_arg(kernel, 1, sizeof(cl_mem), &in_buffer)) {
		return PIGLIT_FAIL;
	}

	if (!piglit_cl_enqueue_ND_range_kernel(context->command_queues[0],
					kernel, 3, NULL, global_size, local_size,
					NULL)) {
		return PIGLIT_FAIL;
	}

	/* This test will check that the clEnqueueReadBuffer() call waits for
	 * the kernel to finish before copying data.  The test was inspired by
	 * a bug in clover where clEnqueueReadBuffer() was being executed
	 * immediately if the command queue had been previously flushed.
	 */
	clFlush(context->command_queues[0]);

	if (!piglit_cl_read_whole_buffer(context->command_queues[0], out_buffer,
					out_data)) {
		return PIGLIT_FAIL;
	}

	for (i = 0; i < BUFFER_ELTS; ++i) {
		if (!piglit_cl_probe_integer(out_data[i], in_data[i], 0)) {
			fprintf(stderr, "Error at out[%u]\n", i);
			return PIGLIT_FAIL;
		}
	}
	return PIGLIT_PASS;
}
