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
 * @file create-command-queue.c
 *
 * Test API function:
 *  cl_command_queue clCreateCommandQueue(cl_context context,
 *                                        cl_device_id device,
 *                                        cl_command_queue_properties properties,
 *                                        cl_int *errcode_ret)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clCreateCommandQueue";
	config.version_min = 10;

	config.run_per_device = true;

PIGLIT_CL_API_TEST_CONFIG_END


/*
 * @param mask  Defines which fields to use
 */
static cl_command_queue_properties
get_mixed_command_queue_properties(int mask,
                                   const cl_command_queue_properties properties[]) {
	int i = 0;
	cl_command_queue_properties mixed_properties = 0;

	while(mask > 0) {
		if(mask%2 == 1) {
			mixed_properties |= properties[i];
		}
		mask >>= 1;
		i++;
	}

	return mixed_properties;
}

static bool
properties_forbidden(const cl_command_queue_properties properties,
                     const struct piglit_cl_api_test_env* env)
{
	int num_command_queue_properties_mutexes =
		PIGLIT_CL_ENUM_NUM(cl_command_queue_properties_mutexes, env->version);
	const cl_command_queue_properties* command_queue_properties_mutexes =
		PIGLIT_CL_ENUM_ARRAY(cl_command_queue_properties_mutexes);
	int i = 0;
	for (; i < num_command_queue_properties_mutexes; ++i)
		if (properties == command_queue_properties_mutexes[i])
			return true;
	return false;
}

enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_api_test_config* config,
               const struct piglit_cl_api_test_env* env)
{
	enum piglit_result result = PIGLIT_PASS;

	int i;
	int mask;
	cl_int errNo;
	cl_context cl_ctx;
	cl_command_queue command_queue;
	cl_uint num_devices;
	cl_device_id* devices;
	cl_command_queue_properties mixed_command_queue_properties[4] =
		{CL_QUEUE_PROPERTIES, 0, 0, 0};

	cl_context_properties context_properties[] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties)env->platform_id,
		0
	};

	int num_command_queue_properties =
		PIGLIT_CL_ENUM_NUM(cl_command_queue_properties, env->version);
	const cl_command_queue_properties* command_queue_properties =
		PIGLIT_CL_ENUM_ARRAY(cl_command_queue_properties);

	/*** Normal usage ***/

	/* create context */
	cl_ctx = clCreateContext(context_properties,
	                         1,
	                         &env->device_id,
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

	/*
	 * For each command queue properties mix.
	 * There are 2^(num_command_queue_properties)-1 possible options.
	 */
	for(mask = 0; mask < (1 << num_command_queue_properties); mask++) {
		mixed_command_queue_properties[1] =
			get_mixed_command_queue_properties(mask, command_queue_properties);
		if (properties_forbidden(mixed_command_queue_properties[1], env))
			continue;
#if defined CL_VERSION_2_0
		if (env->version >= 20) {
			command_queue = clCreateCommandQueueWithProperties(
			                             cl_ctx,
		                                     env->device_id,
		                                     mixed_command_queue_properties,
		                                     &errNo);
		} else
#endif //CL_VERSION_2_0
		{
			command_queue = clCreateCommandQueue(cl_ctx,
		                                     env->device_id,
		                                     mixed_command_queue_properties[1],
		                                     &errNo);
		}
		if(errNo != CL_SUCCESS && errNo != CL_INVALID_QUEUE_PROPERTIES) {
			piglit_cl_check_error(errNo, CL_SUCCESS);
			fprintf(stderr,
			        "Failed (error code: %s): Create command queue using 0x%X as command queue properties.\n",
			        piglit_cl_get_error_name(errNo),
			        (unsigned int)mixed_command_queue_properties[1]);
			piglit_merge_result(&result, PIGLIT_FAIL);
		}
		clReleaseCommandQueue(command_queue);
	}
	
	/*** Errors ***/
	
	/*
	 * CL_INVALID_CONTEXT if context is not a valid context.
	 */
	clCreateCommandQueue(NULL, env->device_id, 0, &errNo);
	if(!piglit_cl_check_error(errNo, CL_INVALID_CONTEXT)) {
		fprintf(stderr,
		        "Failed (error code: %s): Trigger CL_INVALID_CONTEXT if contest is not a valid context.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}

	/*
	 * CL_INVALID_DEVICE if device is not a valid device or is
	 * not associated with context.
	 */
	clCreateCommandQueue(cl_ctx, NULL, 0, &errNo);
	if(!piglit_cl_check_error(errNo, CL_INVALID_DEVICE)) {
		fprintf(stderr,
		        "Failed (error code: %s): Trigger CL_INVALID_DEVICE if device is not a valid device.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}

	num_devices = piglit_cl_get_device_ids(env->platform_id,
	                                       CL_DEVICE_TYPE_ALL,
	                                       &devices);
	for(i = 0; i < num_devices; i++) {
		if(devices[i] != env->device_id) {
			clCreateCommandQueue(cl_ctx, devices[i], 0, &errNo);
			if(!piglit_cl_check_error(errNo, CL_INVALID_DEVICE)) {
				fprintf(stderr,
				        "Failed (error code: %s): Trigger CL_INVALID_DEVICE if device that is not associated with context.\n",
				        piglit_cl_get_error_name(errNo));
				piglit_merge_result(&result, PIGLIT_FAIL);
			}
		}
	}
	free(devices);

	/*
	 * CL_INVALID_VALUE if values specified in properties are not valid.
	 */
	clCreateCommandQueue(cl_ctx, env->device_id, 0XFFFFFFFF, &errNo);
	if(!piglit_cl_check_error(errNo, CL_INVALID_VALUE)) {
		fprintf(stderr,
		        "Failed (error code: %s): Trigger CL_INVALID_VALUE if values specified in properties are not valid.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}

	/*
	 * CL_INVALID_QUEUE_PROPERTIES if values specified in properties
	 * are valid but are not supported by the device.
	 *
	 * Note: already tested in 'normal usage' section
	 */

	clReleaseContext(cl_ctx);

	return result;
}
