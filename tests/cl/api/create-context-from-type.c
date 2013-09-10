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
 * @file create-context-from-type.c
 *
 * Test API function:
 *
 *   cl_context clCreateContextFromType(cl_context_properties *properties,
 *                                      cl_device_type device_type,
 *                                      void *pfn_notify (
 *                                          const char *errinfo,
 *                                          const void *private_info,
 *                                          size_t cb,
 *                                          void *user_data
 *                                      ),
 *                                      void *user_data,
 *                                      cl_int *errcode_ret)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clCreateContextFromType";
	config.version_min = 10;

	config.run_per_platform = true;

PIGLIT_CL_API_TEST_CONFIG_END


static void
test(cl_context_properties *properties,
     const cl_device_type device_type,
     void pfn_notify (
         const char *errinfo,
         const void *private_info,
         size_t cb,
         void *user_data
     ),
     void *user_data,
     cl_int expected_error,
     enum piglit_result* result,
     const char* test_str) {
	cl_int errNo;
	cl_context cl_ctx;

	/* with errNo */
	cl_ctx = clCreateContextFromType(properties,
	                                 device_type,
	                                 pfn_notify,
	                                 user_data,
	                                 &errNo);
	
	if(errNo != CL_DEVICE_NOT_FOUND) {
		if(!piglit_cl_check_error(errNo, expected_error)) {
			fprintf(stderr, "Failed (error code: %s): %s.\n",
			        piglit_cl_get_error_name(errNo), test_str);
			piglit_merge_result(result, PIGLIT_FAIL);
			return;
		};
		if(expected_error == CL_SUCCESS) {
			if(cl_ctx == NULL) {
				printf("Expecting non-NULL cl_context\n");
				fprintf(stderr, "Failed (NULL value returned): %s.\n", test_str);
				piglit_merge_result(result, PIGLIT_FAIL);
				return;
			}
			clReleaseContext(cl_ctx);
		} else if(cl_ctx != NULL) {
			printf("Expecting NULL cl_context\n");
			fprintf(stderr, "Failed (non-NULL value returned): %s.\n", test_str);
			piglit_merge_result(result, PIGLIT_FAIL);
			return;
		}
	}

	/* without errNo */
	cl_ctx = clCreateContextFromType(properties,
	                                 device_type,
	                                 pfn_notify,
	                                 user_data,
	                                 NULL);
	
	if(errNo != CL_DEVICE_NOT_FOUND) {
		if(expected_error == CL_SUCCESS) {
			if(cl_ctx == NULL) {
				printf("Expecting non-NULL cl_context\n");
				fprintf(stderr, "Failed (NULL value returned): %s.\n", test_str);
				piglit_merge_result(result, PIGLIT_FAIL);
				return;
			}
			clReleaseContext(cl_ctx);
		} else if(cl_ctx != NULL) {
			printf("Expecting NULL cl_context\n");
			fprintf(stderr, "Failed (non-NULL value returned): %s.\n", test_str);
			piglit_merge_result(result, PIGLIT_FAIL);
			return;
		}
	}
}

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
	char test_str[1024];

	int i, mask;
	cl_int errNo;
	cl_context cl_ctx;
	cl_device_type mixed_device_types;

	bool found_invalid_platform = false;
	cl_platform_id* platform_ids;
	unsigned int num_platform_ids;
	cl_platform_id invalid_platform_id;

	//TODO: test also CL_CONTEXT_INTEROP_USER_SYNC
	cl_context_properties context_properties[] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties)env->platform_id,
		0
	};
	cl_context_properties invalid_context_properties[] = {
		CL_DEVICE_NAME, (cl_context_properties)env->platform_id,
		0
	};
	cl_context_properties invalid_platform_context_properties[] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties)NULL,
		0
	};
	cl_context_properties multiple_platform_context_properties[] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties)env->platform_id,
		CL_CONTEXT_PLATFORM, (cl_context_properties)env->platform_id,
		0
	};
	
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
	invalid_platform_context_properties[1] =
		(cl_context_properties)invalid_platform_id;

	/*** Normal usage ***/

	/*
	 * For each device types mix.
	 * There are 2^(num_device_types)-1 possible options.
	 */
	for(mask = 1; mask < (1 << num_device_types); mask++) {
		mixed_device_types = get_mixed_device_types(mask, device_types);

		sprintf(test_str,
		        "Create context using 0x%X as device types",
		        (unsigned int)mixed_device_types);

		test(context_properties, mixed_device_types, NULL, NULL,
		     CL_SUCCESS, &result, test_str);
		//TODO: test callback functions
	}


	/*** Errors ***/

    /*
	 * CL_INVALID_VALUE if context property name in properties is
	 * not a supported property name, or if pfn_notify is NULL but
	 * user_data is not NULL.
	 *
	 * Version: 1.0
	 *
	 * CL_INVALID_VALUE if pfn_notify is NULL but user_data is
	 * not NULL.
	 *
	 * Version 1.1
	 */
	if(env->version <= 10) {
		test(invalid_context_properties, CL_DEVICE_TYPE_ALL, NULL, NULL,
		     CL_INVALID_VALUE, &result,
		     "Trigger CL_INVALID_VALUE if context property name in properties is not a supported property name");
	}
	
	test(context_properties, CL_DEVICE_TYPE_ALL, NULL, &context_properties,
	     CL_INVALID_VALUE, &result,
	     "Trigger CL_INVALID_VALUE if pfn_notify is NULL and user_data is not NULL");


	/*
	 * CL_INVALID_PROPERTY if context property name in properties
	 * is not a supported property name, if the value specified for
	 * a supported property name is not valid, or if the same
	 * property name is specified more than once.
	 *
	 * Version: 1.1
	 *
	 * Note: 'if the value specified for a supported property name is
	 * not valid' was already tested
	 */
#if defined(CL_VERSION_1_1)
	if(env->version >= 11) {
		test(multiple_platform_context_properties, CL_DEVICE_TYPE_ALL,
		     NULL, NULL,
		     CL_INVALID_PROPERTY, &result,
		     "Trigger CL_INVALID_PROPERTY if the same property name is pecified more than once");
	}
#endif

	/*
	 * CL_DEVICE_NOT_AVAILABLE if a device in devices is currently
	 * not available even though the device was returned by clGetDeviceIDs.
	 *
	 * Note: Can not test
	 */

	return result;
}
