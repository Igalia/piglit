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
 * @file get-platform-ids.c
 *
 * Test API function:
 *
 *   cl_int clGetPlatformIDs(cl_uint num_entries,
 *                           cl_platform_id *platforms,
 *                           cl_uint *num_platforms)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clGetPlatformIDs";
	config.version_min = 10;

PIGLIT_CL_API_TEST_CONFIG_END


enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_api_test_config* config,
               const struct piglit_cl_api_test_env* env)
{
	enum piglit_result result = PIGLIT_PASS;

	int i;
	cl_int errNo;
	cl_uint num_platforms;
	cl_platform_id* platforms;

	/*** Normal usage ***/

	/* get number of platforms */
	errNo = clGetPlatformIDs(0, NULL, &num_platforms);
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		piglit_cl_check_error(errNo, CL_SUCCESS);
		fprintf(stderr,
		        "Failed (error code: %s): Get size of platform list.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	} else {
		/*
		 * Get platform list.
		 * Try returning from 1 to num_platforms platforms.
		 */
		for(i = 1; i <= num_platforms; i++) {
			platforms = malloc(i * sizeof(cl_platform_id));
			errNo = clGetPlatformIDs(i, platforms, NULL);
			if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
				fprintf(stderr,
				        "Failed (error code: %s): Get platform list.\n",
				        piglit_cl_get_error_name(errNo));
				piglit_merge_result(&result, PIGLIT_FAIL);
			}
			free(platforms);
		}
	}
	
	/*** Errors ***/

	/*
	 * CL_INVALID_VALUE if num_entries is equal
	 * to zero and platforms is not NULL, or if both num_platforms 
	 * and platforms are NULL.
	 */
	errNo = clGetPlatformIDs(0, platforms, NULL);
	if(!piglit_cl_check_error(errNo, CL_INVALID_VALUE)) {
		fprintf(stderr,
		        "Failed (error code: %s): Trigger CL_INVALID_VALUE if num_entries is equeal to zero and platforms is not NULL.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}
	errNo = clGetPlatformIDs(100, NULL, NULL);
	if(!piglit_cl_check_error(errNo, CL_INVALID_VALUE)) {
		fprintf(stderr,
		        "Failed (error code: %s): Trigger CL_INVALID_VALUE if both num_platforms and platforms are NULL.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}

	return result;
}
