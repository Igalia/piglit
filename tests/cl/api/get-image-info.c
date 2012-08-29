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
 * @file get-image-info.c
 *
 * Test API function:
 *
 *   cl_int clGetImageInfo (cl_mem image,
 *                          cl_image_info param_name,
 *                          size_t param_value_size,
 *                          void *param_value,
 *                          size_t *param_value_size_ret)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clGetImageInfo";
	config.version_min = 10;

	config.run_per_platform = true;
	config.create_context = true;

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
	cl_mem image;
	cl_image_format image_format = {
		.image_channel_order = CL_RGBA,
		.image_channel_data_type = CL_FLOAT,
	};

	size_t param_value_size;
	void* param_value;
	
	int num_image_infos = PIGLIT_CL_ENUM_NUM(cl_image_info, env->version);
	const cl_image_info* image_infos = PIGLIT_CL_ENUM_ARRAY(cl_image_info);
	
#if defined CL_VERSION_1_2
	if(env->version >= 12) {
		cl_image_desc image_desc = {
			.image_type = CL_MEM_OBJECT_IMAGE2D,
			.image_width = 128,
			.image_height = 128,
			.image_row_pitch = 0,
			.image_slice_pitch = 0,
			.num_mip_levels = 0,
			.num_samples = 0,
			.buffer = NULL,
		};

		image = clCreateImage(env->context->cl_ctx,
		                      CL_MEM_READ_WRITE,
		                      &image_format,
		                      &image_desc,
		                      NULL,
		                      &errNo);
	} else {
		fprintf(stderr, "Could not create image. Piglit was compiled against OpenCL version >= 1.2 and cannot run this test for versions < 1.2 because clCreateImage function is not present.\n");
		return PIGLIT_WARN;
	}
#else //CL_VERSION_1_2
	if(env->version <= 11) {
		image = clCreateImage2D(env->context->cl_ctx,
		                        CL_MEM_READ_WRITE,
		                        &image_format,
		                        128, 128, 0,
		                        NULL,
		                        &errNo);
	} else {
		fprintf(stderr, "Could not create image. Piglit was compiled against OpenCL version < 1.2 and cannot run this test for versions >= 1.2 because clCreateImage2D function was deprecated.\n");
		return PIGLIT_WARN;
	}
#endif //CL_VERSION_1_2
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Failed (error code: %s): Create an image.\n",
		        piglit_cl_get_error_name(errNo));
		return PIGLIT_FAIL;
	}

	/*** Normal usage ***/
	for(i = 0; i < num_image_infos; i++) {
		printf("%s ", piglit_cl_get_enum_name(image_infos[i]));

		errNo = clGetImageInfo(image,
		                       image_infos[i],
		                       0,
		                       NULL,
		                       &param_value_size);
		if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
			fprintf(stderr,
			        "Failed (error code: %s): Get size of %s.\n",
			        piglit_cl_get_error_name(errNo),
			        piglit_cl_get_enum_name(image_infos[i]));
			piglit_merge_result(&result, PIGLIT_FAIL);
			continue;
		}

		param_value = malloc(param_value_size);
		errNo = clGetImageInfo(image,
		                       image_infos[i],
		                       param_value_size,
		                       param_value,
		                       NULL);
		if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
			fprintf(stderr,
			        "Failed (error code: %s): Get value of %s.\n",
			        piglit_cl_get_error_name(errNo),
			        piglit_cl_get_enum_name(image_infos[i]));
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
	errNo = clGetImageInfo(image,
	                       CL_DEVICE_NAME,
	                       0,
	                       NULL,
	                       &param_value_size);
	if(!piglit_cl_check_error(errNo, CL_INVALID_VALUE)) {
		fprintf(stderr,
		        "Failed (error code: %s): Trigger CL_INVALID_VALUE if param_name is not one of the supported values.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}

	errNo = clGetImageInfo(image,
	                       CL_IMAGE_FORMAT,
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
	 * CL_INVALID_MEM_OBJECT if image is a not a valid image object.
	 */
	errNo = clGetImageInfo(NULL,
	                       CL_IMAGE_FORMAT,
	                       0,
	                       NULL,
	                       &param_value_size);
	if(!piglit_cl_check_error(errNo, CL_INVALID_MEM_OBJECT)) {
		fprintf(stderr,
		        "Failed (error code: %s): Trigger CL_INVALID_MEM_OBJECT if image is not a valid image object.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}

	clReleaseMemObject(image);

	return result;
}
