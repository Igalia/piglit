/*
 * Copyright © 2016 Serge Martin <edb+piglit@sigluy.net>
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
 * @file get-extension-function-address-for-platform.c
 *
 * Test API function:
 *
 *   void *
 *   clGetExtensionFunctionAddressForPlatform(cl_platform_id platform,
 *                                            const char *funcname)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clGetExtensionFunctionAddressForPlatform";
	config.version_min = 12;

	config.run_per_platform = true;

PIGLIT_CL_API_TEST_CONFIG_END


enum piglit_result
piglit_cl_test(const int argc,
               const char **argv,
               const struct piglit_cl_api_test_config* config,
               const struct piglit_cl_api_test_env* env)
{
#if defined(CL_VERSION_1_2)
	enum piglit_result result = PIGLIT_PASS;

	void *ptr;
	char *exts_list = piglit_cl_get_platform_info(env->platform_id,
	                                              CL_PLATFORM_EXTENSIONS);

	if(!exts_list) {
		fprintf(stderr, "clGetPlatformInfo error.\n");
		return PIGLIT_FAIL;
	}

	printf("extensions list: %s\n", exts_list);

	/*** Normal usage ***/
	if (strstr(exts_list, "cl_khr_icd") != NULL) {
		printf("cl_khr_icd: clIcdGetPlatformIDsKHR\n");
		ptr = clGetExtensionFunctionAddressForPlatform(env->platform_id,
		                                               "clIcdGetPlatformIDsKHR");
		if (!ptr) {
			fprintf(stderr, "Failed to get clIcdGetPlatformIDsKHR address\n");
			piglit_merge_result(&result, PIGLIT_FAIL);
		}
	}

	/*** Errors ***/

	/* clIcdGetPlatformIDsKHR should be present in most of OpenCL 1.2 libs */
	ptr = clGetExtensionFunctionAddressForPlatform(NULL,
	                                               "clIcdGetPlatformIDsKHR");
	if (ptr) {
		fprintf(stderr, "Failed: return NULL if platform is not a valid platform\n");
		piglit_merge_result(&result, PIGLIT_FAIL);
	}

	ptr = clGetExtensionFunctionAddressForPlatform(env->platform_id,
	                                               "invalid_name");
	if (ptr) {
		fprintf(stderr, "Failed: return NULL if the specified function does not exist\n");
		piglit_merge_result(&result, PIGLIT_FAIL);
	}

	free(exts_list);

	return result;
#else
	return PIGLIT_SKIP;
#endif
}
