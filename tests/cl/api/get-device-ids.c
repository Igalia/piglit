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
 * @file get-device-ids.c
 *
 * Test API function:
 *
 *   cl_int clGetDeviceIDs(cl_platform_id platform,
 *                         cl_device_type device_type,
 *                         cl_uint num_entries,
 *                         cl_device_id *devices,
 *                         cl_uint *num_devices)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clGetDeviceIDs";
	config.version_min = 10;

	config.run_per_platform = true;

PIGLIT_CL_API_TEST_CONFIG_END

/*
 * @param mask  Defines which fields to use
 */
static cl_device_type
get_mixed_device_types(int mask, const cl_device_type device_types[]) {
	int i = 0;
	cl_device_type mixed_device_types = 0;
	
	while(mask > 0) {
		if(mask%2 == 1) {
			mixed_device_types |= device_types[i];
		}
		mask >>= 1;
		i++;
	}

	return mixed_device_types;
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
	cl_uint num_devices;
	cl_device_id* devices;
	cl_device_id device_random;
	cl_device_type mixed_device_types;

	bool found_invalid_platform = false;
	cl_platform_id* platform_ids;
	unsigned int num_platform_ids;
	cl_platform_id invalid_platform_id;

	int num_device_types = PIGLIT_CL_ENUM_NUM(cl_device_type, env->version);
	const cl_device_type *device_types = PIGLIT_CL_ENUM_ARRAY(cl_device_type);

	/* Find invalid platform_id */
	invalid_platform_id = 0;
	num_platform_ids = piglit_cl_get_platform_ids(&platform_ids);
	while(!found_invalid_platform) {
		found_invalid_platform = true;
		invalid_platform_id = (cl_platform_id)1;
		for(i = 0; i < num_platform_ids; i++) {
			if(invalid_platform_id == platform_ids[i]) {
				found_invalid_platform = false;
				break;
			}
		}
	}
	free(platform_ids);

	/*** Normal usage ***/

	/*
	 * For each device types mix.
	 * There are 2^(num_device_types)-1 possible options.
	 */
	for(mask = 1; mask < (1 << num_device_types); mask++) {
		mixed_device_types = get_mixed_device_types(mask, device_types);

		/* get number of devices */
		errNo = clGetDeviceIDs(env->platform_id,
		                       mixed_device_types,
		                       0,
		                       NULL,
		                       &num_devices);

		/*
		 * Get device list.
		 * Try returning from 1 to num_devices devices.
		 */
		if(errNo == CL_SUCCESS) {
			for(i = 1; i <= num_devices; i++) {
				devices = malloc(i * sizeof(cl_device_id));
				errNo = clGetDeviceIDs(env->platform_id,
				                       mixed_device_types,
				                       num_devices,
				                       devices,
				                       NULL);
				if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
						fprintf(stderr,
						        "Failed (error code: %s): Get devices list.\n",
						        piglit_cl_get_error_name(errNo));
					piglit_merge_result(&result, PIGLIT_FAIL);
				}
				free(devices);
			}
		} else if (errNo == CL_DEVICE_NOT_FOUND) {
			/* skip retrieving devices */
		} else {
			piglit_cl_check_error(errNo, CL_SUCCESS);
			fprintf(stderr,
			        "Failed (error code: %s): Get size of devices list.\n",
			        piglit_cl_get_error_name(errNo));
			piglit_merge_result(&result, PIGLIT_FAIL);
		}
	}
	
	/*** Errors ***/
	
	/*
	 * CL_INVALID_VALUE if num_entries is equal to zero and devices
	 * is not NULL or if both num_devices and devices are NULL.
	 */
	errNo = clGetDeviceIDs(env->platform_id,
	                       CL_DEVICE_TYPE_ALL,
	                       0,
	                       &device_random,
	                       NULL);
	if(!piglit_cl_check_error(errNo, CL_INVALID_VALUE)) {
		fprintf(stderr,
		        "Failed (error code: %s): Trigger CL_INVALID_VALUE if num_entries is equeal to zero and devices is not NULL.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}
	errNo = clGetDeviceIDs(env->platform_id,
	                       CL_DEVICE_TYPE_ALL,
	                       100,
	                       NULL,
	                       NULL);
	if(!piglit_cl_check_error(errNo, CL_INVALID_VALUE)) {
		piglit_merge_result(&result, PIGLIT_FAIL);
		fprintf(stderr,
		        "Failed (error code: %s): Trigger CL_INVALID_VALUE if both num_devices and devices are NULL.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}
	/*
	 * CL_INVALID_DEVICE_TYPE if device_type is not a valid value.
	 *
	 * Note: Cannot test, because there are no mutually exclusive flags.
	 *
	 * piglit_cl_expect_error(clGetDeviceIDs(env->platform_id,
	 *                                       0,
	 *                                       0,
	 *                                       NULL,
	 *                                       &num_devices),
	 *                        CL_INVALID_DEVICE_TYPE,
	 *                        PIGLIT_FAIL);
	 */

	/*
	 * CL_INVALID_PLATFORM if platform is not a valid platform.
	 */
	errNo = clGetDeviceIDs(invalid_platform_id,
	                       CL_DEVICE_TYPE_ALL,
	                       0,
	                       NULL,
	                       &num_devices);
	if(!piglit_cl_check_error(errNo, CL_INVALID_PLATFORM)) {
		fprintf(stderr,
		        "Failed (error code: %s): Trigger CL_INVALID_PLATFORM if platform is not a valid platform.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}

	return result;
}
