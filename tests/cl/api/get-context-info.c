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
 * @file get-context-info.c
 *
 * Test API function:
 *
 *   cl_int clGetContextInfo (cl_context context,
 *                            cl_context_info param_name,
 *                            size_t param_value_size,
 *                            void *param_value,
 *                            size_t param_value_size_ret)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clGetContextInfo";
	config.version_min = 10;

	config.run_per_platform = true;

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
	cl_context cl_ctx;

	size_t param_value_size;
	void* param_value;
	
	cl_context_properties context_properties[] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties)env->platform_id,
		0
	};

	int num_context_infos = PIGLIT_CL_ENUM_NUM(cl_context_info, env->version);
	const cl_context_info *context_infos = PIGLIT_CL_ENUM_ARRAY(cl_context_info);

	/*** Normal usage ***/

	cl_ctx = clCreateContextFromType(context_properties,
	                                 CL_DEVICE_TYPE_ALL,
	                                 NULL,
	                                 NULL,
	                                 &errNo);
	if(errNo == CL_DEVICE_NOT_FOUND) {
		fprintf(stderr, "No available devices.\n");
		return PIGLIT_SKIP;
	}
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Failed (error code: %s): Create context.\n",
		        piglit_cl_get_error_name(errNo));
		return PIGLIT_FAIL;
	}
	
	for(i = 0; i < num_context_infos; i++) {
		printf("%s ", piglit_cl_get_enum_name(context_infos[i]));

		errNo = clGetContextInfo(cl_ctx,
		                                           context_infos[i],
		                                           0,
		                                           NULL,
		                                           &param_value_size);
		if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
			fprintf(stderr,
			        "Failed (error code: %s): Get size of %s.\n",
			        piglit_cl_get_error_name(errNo),
			        piglit_cl_get_enum_name(context_infos[i]));
			piglit_merge_result(&result, PIGLIT_FAIL);
			continue;
		}

		param_value = malloc(param_value_size);
		errNo = clGetContextInfo(cl_ctx,
		                                           context_infos[i],
		                                           param_value_size,
		                                           param_value,
		                                           NULL);
		if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
			fprintf(stderr,
			        "Failed (error code: %s): Get value of %s.\n",
			        piglit_cl_get_error_name(errNo),
			        piglit_cl_get_enum_name(context_infos[i]));
			piglit_merge_result(&result, PIGLIT_FAIL);
		}

		//TODO: output returned values
		printf("\n");
		free(param_value);
	}

	/*** Errors ***/

	/*
	 * CL_INVALID_VALUE if param_name is not one of the supported
	 * values or if size in bytes specified by param_value_size is
	 * less than size of return type and param_value is not a NULL
	 * value.
	 */
	errNo = clGetContextInfo(cl_ctx,
	                         CL_PLATFORM_NAME,
	                         0,
	                         NULL,
	                         &param_value_size);
	if(!piglit_cl_check_error(errNo, CL_INVALID_VALUE)) {
		fprintf(stderr,
		        "Failed (error code: %s): Trigger CL_INVALID_VALUE if param_name is not one of the supported values.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}

	errNo = clGetContextInfo(cl_ctx,
	                         CL_CONTEXT_REFERENCE_COUNT,
	                         1,
	                         param_value,
	                         NULL);
	if(!piglit_cl_check_error(errNo, CL_INVALID_VALUE)) {
		fprintf(stderr,
		        "Failed (error code: %s): Trigger CL_INVALID_VALUE if size in bytes specified by param_value is less than size of return type and param_value is not a NULL value.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}
	
	/*
	 * CL_INVALID_CONTEXT if context is not a valid context.
	 */
	errNo = clGetContextInfo(NULL,
	                         CL_CONTEXT_DEVICES,
	                         0,
	                         NULL,
	                         &param_value_size);
	if(!piglit_cl_check_error(errNo, CL_INVALID_CONTEXT)) {
		fprintf(stderr,
		        "Failed (error code: %s): Trigger CL_INVALID_CONTEXT if context is not a valid context.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}


	clReleaseContext(cl_ctx);

	return result;
}
