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
 * @file create-program-with-source.c
 *
 * Test API function:
 *
 *   cl_program clCreateProgramWithSource (cl_context context,
 *                                         cl_uint count,
 *                                         const char **strings,
 *                                         const size_t *lengths,
 *                                         cl_int *errcode_ret)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clCreateProgramWithSource";
	config.version_min = 10;

	config.run_per_platform = true;
	config.create_context = true;

PIGLIT_CL_API_TEST_CONFIG_END


char* dummy_function = "void dummy_function() {}";
char* dummy_kernel = "kernel void dummy_kernel() { dummy_function(); }";

static void
test(cl_context cl_ctx,
     cl_uint count,
     const char **strings,
     const size_t *lengths,
     cl_int expected_error,
     enum piglit_result* result,
     const char* test_str) {
	cl_int errNo;
	cl_program program;

	/* with errNo */
	program = clCreateProgramWithSource(cl_ctx,
	                                    count,
	                                    strings,
	                                    lengths,
	                                    &errNo);
	
	if(!piglit_cl_check_error(errNo, expected_error)) {
		fprintf(stderr, "Failed (error code: %s): %s.\n",
		        piglit_cl_get_error_name(errNo), test_str);
		piglit_merge_result(result, PIGLIT_FAIL);
		return;
	}
	if(expected_error == CL_SUCCESS) {
		if(program == NULL) {
			printf("Expecting non-NULL cl_program\n");
			fprintf(stderr, "Failed (NULL value returned): %s.\n", test_str);
			piglit_merge_result(result, PIGLIT_FAIL);
			return;
		}
		clReleaseProgram(program);
	} else if (program != NULL) {
		printf("Expecting NULL cl_program\n");
		fprintf(stderr, "Failed (non-NULL value returned): %s.\n", test_str);
		piglit_merge_result(result, PIGLIT_FAIL);
		return;
	}

	/* without errNo */
	program = clCreateProgramWithSource(cl_ctx,
	                                    count,
	                                    strings,
	                                    lengths,
	                                    NULL);
	
	if(expected_error == CL_SUCCESS) {
		if(program == NULL) {
			printf("Expecting non-NULL cl_program\n");
			fprintf(stderr, "Failed (NULL value returned): %s.\n", test_str);
			piglit_merge_result(result, PIGLIT_FAIL);
			return;
		}
		clReleaseProgram(program);
	} else if (program != NULL) {
		printf("Expecting NULL cl_program\n");
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

	int i;

	const char* null = NULL;
	const char** strings = malloc(2 * sizeof(char*));
	size_t* lengths = malloc(2 * sizeof(size_t));

	strings[0] = dummy_function;
	strings[1] = dummy_kernel;
	lengths[0] = strlen(dummy_function);
	lengths[1] = strlen(dummy_kernel);

	/*** Normal usage ***/

	for(i = 0; i < 2; i++) {
		size_t* partial_lengths = malloc(2 * sizeof(size_t));

		/* separate */
		test(env->context->cl_ctx, 1, &strings[i], &lengths[i],
		     CL_SUCCESS, &result,
		     "Create program with 1 source string and defined length");
		test(env->context->cl_ctx, 1, &strings[i], NULL,
		     CL_SUCCESS, &result,
		     "Create program with 1 source string and lenghts == NULL");
		
		/* all, i-th length is 0 */
		partial_lengths[i] = 0;
		partial_lengths[(i+1)%2] = lengths[(i+1)%2];
		
		test(env->context->cl_ctx, 2, strings, partial_lengths,
		     CL_SUCCESS, &result,
		     "Create program with multiple source strings and only some lenghts defined (others are NULL)");

		free(partial_lengths);
	}

	/* all */
	test(env->context->cl_ctx, 2, strings, lengths,
	     CL_SUCCESS, &result,
	     "Create program with multiple source strings and defined lengths");
	test(env->context->cl_ctx, 2, strings, NULL,
	     CL_SUCCESS, &result,
	     "Create program with multiple source strings and lenghts == NULL");
	
	
	/*** Errors ***/

	/*
	 * CL_INVALID_CONTEXT if context is not a valid context.
	 */
	test(NULL, 2, strings, NULL,
	     CL_INVALID_CONTEXT, &result,
	     "Trigger CL_INVALID_CONTEXT when context is not a valid context");

	/*
	 * CL_INVALID_VALUE if count is zero or if strings or
	 * any entry in strings is NULL.
	 */
	test(env->context->cl_ctx, 0, strings, NULL,
	     CL_INVALID_VALUE, &result,
	     "Trigger CL_INVALID_VALUE when count is zero");
	test(env->context->cl_ctx, 0, NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "Trigger CL_INVALID_VALUE when strings is NULL");
	test(env->context->cl_ctx, 1, &null, NULL,
	     CL_INVALID_VALUE, &result,
	     "Trigger CL_INVALID_VALUE when any entry in strings is NULL");

	free(strings);
	free(lengths);

	return result;
}
