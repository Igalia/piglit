/*
 * Copyright © 2012 Blaž Tomažič <blaz.tomazic@gmail.com>
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
 */

#include "piglit-framework-cl-custom.h"

PIGLIT_CL_CUSTOM_TEST_CONFIG_BEGIN

	config.name = "Run simple kernel";
	config.run_per_device = true;

PIGLIT_CL_CUSTOM_TEST_CONFIG_END

char* source = "kernel void test(global int* out){ *out = -1; }";

enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_custom_test_config* config,
               const struct piglit_cl_custom_test_env* env)
{
	enum piglit_result result = PIGLIT_PASS;

	size_t global_size = 1;
	size_t local_size = 1;
	cl_int data = 0;

	piglit_cl_context context = NULL;
	cl_mem buffer = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;

	/* Create objects up to the kernel */
	context = piglit_cl_create_context(env->platform_id, &env->device_id, 1);
	buffer = piglit_cl_create_buffer(context, CL_MEM_READ_WRITE,
	                                 sizeof(cl_int));
	program = piglit_cl_build_program_with_source(context, 1, &source, "");
	kernel = piglit_cl_create_kernel(program, "test");

	/* Set kernel arguments and run the kernel */
	piglit_cl_write_buffer(context->command_queues[0], buffer, 0,
	                       sizeof(cl_int), &data);
	piglit_cl_set_kernel_buffer_arg(kernel, 0, &buffer);
	piglit_cl_execute_ND_range_kernel(context->command_queues[0], kernel, 1,
	                                  &global_size, &local_size);

	/* Read the buffer and check the result */
	piglit_cl_read_buffer(context->command_queues[0], buffer, 0,
	                      sizeof(cl_int), &data);
	if(data != -1) {
		fprintf(stderr,
		        "Failed to properly execute the kernel.\n");
		result = PIGLIT_FAIL;
	}

	/* Release resources */
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseMemObject(buffer);
	piglit_cl_release_context(context);

	return result;
}
