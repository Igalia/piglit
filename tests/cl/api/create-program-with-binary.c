/*
 * Copyright © 2012 Blaž Tomažič <blaz.tomazic@gmail.com>
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
 */

/**
 * @file create-program-with-binary.c
 *
 * Test API function:
 *
 *  cl_program clCreateProgramWithBinary (cl_context context,
 *                                        cl_uint num_devices,
 *                                        const cl_device_id *device_list,
 *                                        const size_t *lengths,
 *                                        const unsigned char **binaries,
 *                                        cl_int *binary_status,
 *                                        cl_int *errcode_ret)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clCreateProgramWithBinary";
	config.version_min = 10;

	config.run_per_platform = true;
	config.create_context = true;

PIGLIT_CL_API_TEST_CONFIG_END


const char* dummy_kernel = "kernel void dummy_kernel() { }";

static cl_program create_binary_program(const piglit_cl_context ctx)
{
	cl_program program = NULL;
	cl_program binary_program = NULL;
	cl_int errNo;
	unsigned i;

	size_t kernel_length = strlen(dummy_kernel);
	cl_context cl_ctx = ctx->cl_ctx;
	size_t *sizes;
	unsigned char **binaries;

	program = clCreateProgramWithSource(cl_ctx,
	                                    1,
	                                    &dummy_kernel,
	                                    &kernel_length,
	                                    &errNo);

	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr, "clCreateProgramWithSource failed "
				"(error code: %s)\n",
		        piglit_cl_get_error_name(errNo));
		goto fail;
	}


	errNo = clBuildProgram(program, ctx->num_devices,
				ctx->device_ids, NULL, NULL, NULL);

	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr, "clBuildProgram failed "
				"(error code: %s)\n",
		        piglit_cl_get_error_name(errNo));
		goto free_program;
	}


	sizes = calloc(sizeof(size_t), ctx->num_devices);
	errNo = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES,
				 sizeof(size_t) * ctx->num_devices,
				 sizes, NULL);

	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr, "clCreateProgramInfo failed "
				"(error code: %s)\n",
		        piglit_cl_get_error_name(errNo));
		goto free_sizes;
	}

	binaries = calloc(sizeof(char*),  ctx->num_devices);
	for (i = 0; i < ctx->num_devices; i++) {
		binaries[i] = malloc(sizes[i]);
	}

	errNo = clGetProgramInfo(program, CL_PROGRAM_BINARIES,
	                         sizeof(char*) * ctx->num_devices,
				 binaries, NULL);

	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr, "clCreateProgramInfo failed "
				"(error code: %s)\n",
		        piglit_cl_get_error_name(errNo));
		goto free_binaries;
	}

	binary_program = clCreateProgramWithBinary (cl_ctx, ctx->num_devices,
						      ctx->device_ids,
						      sizes, binaries,
						      NULL,
						      &errNo);

	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr, "clCreateProgramWithBinary failed "
				"(error code: %s)\n",
		        piglit_cl_get_error_name(errNo));
		goto free_binaries;
	}

free_binaries:
	for (i = 0; i < ctx->num_devices; i++) {
		free(binaries[i]);
	}
	free(binaries);

free_sizes:
	free(sizes);

free_program:
	clReleaseProgram(program);

fail:
	return binary_program;

}

enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_api_test_config* config,
               const struct piglit_cl_api_test_env* env)
{
	int i;
	cl_program binary_program;
	cl_kernel kernel;
	cl_int errNo;

	piglit_cl_context ctx = env->context;
	enum piglit_result result = PIGLIT_PASS;

	binary_program = create_binary_program(ctx);
	if (!binary_program) {
		piglit_merge_result(&result, PIGLIT_FAIL);
		goto fail;
	}


	/* test0: Execute a binary program */
	kernel = clCreateKernel(binary_program, "dummy_kernel", &errNo);

	if (!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr, "clCreateKernel failed "
				"(error code: %s)\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
		goto done_test0;
	}

	for (i = 0; i < ctx->num_devices; i++) {
		size_t global_work_size = 1;
		size_t local_work_size = 1;
		cl_command_queue queue = ctx->command_queues[i];
		if (!piglit_cl_enqueue_ND_range_kernel(queue, kernel, 1,
					&global_work_size, &local_work_size)) {
			fprintf(stderr, "Failed to execute binary kernel.");
			piglit_merge_result(&result, PIGLIT_FAIL);
		}
	}

	clReleaseKernel(kernel);
done_test0:

	/* test1: Pass binary program to clBuildProgram() */
	errNo = clBuildProgram(binary_program, ctx->num_devices,
				ctx->device_ids, NULL, NULL, NULL);
	if (!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr, "Failed to compile binary program.");
		piglit_merge_result(&result, PIGLIT_FAIL);
	}

	/* test2 Pass binary program to clCompileProgram() */

#ifdef CL_VERSION_1_2

	if (piglit_cl_get_platform_version(ctx->platform_id) >= 12) {
		errNo = clCompileProgram(binary_program, ctx->num_devices,
					ctx->device_ids, NULL, 0, NULL, NULL,
					NULL, NULL);
		if (!piglit_cl_check_error(errNo, CL_INVALID_OPERATION)) {
			fprintf(stderr, "Passing a binary program to "
				"clCompileProgram() should return "
				"CL_INVALID_OPERATION");
			piglit_merge_result(&result, PIGLIT_FAIL);
		}
	}
#endif

	clReleaseProgram(binary_program);
fail:
	return result;
}
