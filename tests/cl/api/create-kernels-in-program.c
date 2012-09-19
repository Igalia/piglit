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

/**
 * @file create-kernels-in-progrm.c
 *
 * Test API function:
 *
 *   cl_int clCreateKernelsInProgram (cl_program  program,
 *                                    cl_uint num_kernels,
 *                                    cl_kernel *kernels,
 *                                    cl_uint *num_kernels_ret)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clCreateKernelsInProgram";
	config.version_min = 10;

	config.run_per_platform = true;
	config.create_context = true;

	config.program_source = "kernel void dummy_kernel_1() {}\
	                         kernel void dummy_kernel_2() {}";

PIGLIT_CL_API_TEST_CONFIG_END


static bool
test(cl_program program,
     cl_uint num_kernels,
     cl_kernel *kernels,
     cl_uint *num_kernels_ret,
     cl_int expected_error,
     enum piglit_result *result,
     const char* test_str)
{
	cl_int errNo;

	errNo = clCreateKernelsInProgram(program,
	                                 num_kernels,
	                                 kernels,
	                                 num_kernels_ret);
	if(!piglit_cl_check_error(errNo, expected_error)) {
		fprintf(stderr, "Failed (error code: %s): %s.\n",
		        piglit_cl_get_error_name(errNo), test_str);
		piglit_merge_result(result, PIGLIT_FAIL);
		return false;
	}

	return true;
}

enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_api_test_config* config,
               const struct piglit_cl_api_test_env* env)
{
	enum piglit_result result = PIGLIT_PASS;

	int i;
	cl_int errNo;
	cl_kernel* kernels;
	cl_uint num_kernels;
	cl_program temp_program;

	/*** Normal usage ***/
	if(!test(env->program, 0, NULL, &num_kernels,
	         CL_SUCCESS, &result,
	         "Get number of kernels in program")) {
		return result;
	} else {
		kernels = malloc(num_kernels * sizeof(cl_kernel));

		if(test(env->program, num_kernels, kernels, NULL,
		     CL_SUCCESS, &result,
		     "Get all kernels in program")) {
			for(i = 0; i < num_kernels; i++) {
				clReleaseKernel(kernels[i]);
			}
		}

		free(kernels);
	}

	/*** Errors ***/

	/*
	 * CL_INVALID_PROGRAM if program is not a valid program object.
	 */
	test(NULL, 0, NULL, &num_kernels,
	     CL_INVALID_PROGRAM, &result,
	     "Trigger CL_INVALID_PROGRAM when program is not a valid program object");

	/*
	 * CL_INVALID_PROGRAM_EXECUTABLE if there is no successfully built
	 * executable for any device in program.
	 */
	temp_program = clCreateProgramWithSource(env->context->cl_ctx,
	                                         1,
	                                         (const char**)&config->program_source,
	                                         NULL,
	                                         &errNo);
	if(piglit_cl_check_error(errNo, CL_SUCCESS)) {
		test(temp_program, 0, NULL, &num_kernels,
		     CL_INVALID_PROGRAM_EXECUTABLE, &result,
		     "Trigger CL_INVALID_PROGRAM_EXECUTABLE when there is no successfully built executable for any device in program");
		clReleaseProgram(temp_program);
	} else {
		fprintf(stderr,
		        "Failed (error code: %s): Create program with source.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}

	/*
	 * CL_INVALID_VALUE if kernels is not NULL and num_kernels is less than the
	 * number of kernels in program.
	 */
	test(env->program, 1, kernels, NULL,
	     CL_INVALID_VALUE, &result,
	     "Trigger CL_INVALID_VALUE when kernels is not NULL and num_kernels is less than number of kernels in program.");

	return result;
}
