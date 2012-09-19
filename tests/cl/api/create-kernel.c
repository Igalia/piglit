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
 * @file create-kernel.c
 *
 * Test API function:
 *
 *   cl_kernel clCreateKernel (cl_program  program,
 *                             const char *kernel_name,
 *                             cl_int *errcode_ret)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clCreateKernel";
	config.version_min = 10;

	config.run_per_platform = true;
	config.create_context = true;

	config.program_source = "kernel void dummy_kernel() {}";

PIGLIT_CL_API_TEST_CONFIG_END


static void
test(cl_program program,
     const char* kernel_name,
     cl_int expected_error,
     enum piglit_result* result,
     const char* test_str) {
	cl_int errNo;
	cl_kernel kernel;

	/* with errNo */
	kernel = clCreateKernel(program, kernel_name, &errNo);
	
	if(!piglit_cl_check_error(errNo, expected_error)) {
		fprintf(stderr, "Failed (error code: %s): %s.\n",
		        piglit_cl_get_error_name(errNo), test_str);
		piglit_merge_result(result, PIGLIT_FAIL);
		return;
	}
	if(expected_error == CL_SUCCESS) {
		if(kernel == NULL) {
			printf("Expecting non-NULL cl_kernel\n");
			fprintf(stderr, "Failed (NULL value returned): %s.\n", test_str);
			piglit_merge_result(result, PIGLIT_FAIL);
			return;
		}
		clReleaseKernel(kernel);
	} else if (kernel != NULL) {
		printf("Expecting NULL cl_kernel\n");
		fprintf(stderr, "Failed (non-NULL value returned): %s.\n", test_str);
		piglit_merge_result(result, PIGLIT_FAIL);
		return;
	}

	/* without errNo */
	kernel = clCreateKernel(program, kernel_name, &errNo);
	
	if(expected_error == CL_SUCCESS) {
		if(kernel == NULL) {
			printf("Expecting non-NULL cl_kernel\n");
			fprintf(stderr, "Failed (NULL value returned): %s.\n", test_str);
			piglit_merge_result(result, PIGLIT_FAIL);
			return;
		}
		clReleaseKernel(kernel);
	} else if (kernel != NULL) {
		printf("Expecting NULL cl_kernel\n");
		fprintf(stderr, "Failed (non-NULL value returned): %s.\n", test_str);
		piglit_merge_result(result, PIGLIT_FAIL);
		return;
	}
}

enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_api_test_config* config,
               const struct piglit_cl_api_test_env* env)
{
	enum piglit_result result = PIGLIT_PASS;

	cl_int errNo;
	cl_program temp_program;

	/*** Normal usage ***/
	test(env->program, "dummy_kernel",
	     CL_SUCCESS, &result, "Create kernel");

	/*** Errors ***/

	/*
	 * CL_INVALID_PROGRAM if program is not a valid program object.
	 */
	test(NULL, "dummy_kernel",
	     CL_INVALID_PROGRAM, &result,
	     "Trigger CL_INVALID_PROGRAM if program is not a valid program");

	/*
	 * CL_INVALID_PROGRAM_EXECUTABLE if there is no successfully built
	 * executable for program.
	 */
	temp_program = clCreateProgramWithSource(env->context->cl_ctx,
	                                         1,
	                                         (const char**)&config->program_source,
	                                         NULL,
	                                         &errNo);
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Failed (error code: %s): Create program with source.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	} else {
		test(temp_program, "dummy_kernel",
		     CL_INVALID_PROGRAM_EXECUTABLE, &result,
		     "Trigger CL_INVALID_PROGRAM_EXECUTABLE if there is no successfully built executable program");
		
		clReleaseProgram(temp_program);
	}

	/*
	 * CL_INVALID_KERNEL_NAME if kernel_name is not found in program.
	 */
	test(env->program, "wrong_kernel_name",
	     CL_INVALID_KERNEL_NAME, &result,
	     "Trigger CL_INVALID_KERNEL_NAME if kernel_name is not found in program");

	/*
	 * CL_INVALID_KERNEL_DEFINITION if the function definition for __kernel
	 * function given by kernel_name such as the number of arguments, the
	 * argument types are not the same for all devices for which the program
	 * executable has been built.
	 *
	 * TODO
	 */

	/*
	 * CL_INVALID_VALUE if kernel_name is NULL.
	 */
	test(env->program, NULL,
	     CL_INVALID_VALUE, &result,
	     "Trigger CL_INVALID_VALUE if kernel_name is NULL");

	return result;
}
