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
 * @file get-kernel-work-group-info.c
 *
 * Test API function:
 *
 *   cl_int clGetKernelWorkGroupInfo (cl_kernel  kernel,
 *                                    cl_device_id  device,
 *                                    cl_kernel_work_group_info  param_name,
 *                                    size_t  param_value_size,
 *                                    void  *param_value,
 *                                    size_t  *param_value_size_ret)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clGetKernelWorkGroupInfo";
	config.version_min = 10;

	config.run_per_device = true;
	config.create_context = true;

	config.program_source =  "kernel __attribute__((reqd_work_group_size(1, 1, 1))) void dummy_kernel() {}\n";

PIGLIT_CL_API_TEST_CONFIG_END

static bool
check_size(size_t expected_size, size_t actual_size, enum piglit_result *result) {
	if (expected_size != actual_size) {
		printf(": failed, expected and actual size differ. Expect %lu, got %lu",
		       expected_size, actual_size);
		piglit_merge_result(result, PIGLIT_FAIL);
		return false;
	}

	return true;
}

static void
print_u(size_t i) {
	printf(": %zu", i);
}

static void
check_info(cl_bool is_custom_device,
           cl_kernel_work_group_info kind,
           void* param_value, size_t param_value_size,
           enum piglit_result *result) {

	switch (kind) {
		case CL_KERNEL_WORK_GROUP_SIZE:
			if (check_size(sizeof(size_t), param_value_size, result)) {
				print_u(*(size_t*)param_value);
			}
			break;
		case CL_KERNEL_COMPILE_WORK_GROUP_SIZE:
			if (check_size(sizeof(size_t) * 3, param_value_size, result)) {
				printf(": ");
				size_t* v = (size_t*)param_value;
				if (v[0] != 1 && v[1] != 1 && v[2] != 1) {
					printf("failed, expected and actual value differ. Expect (1,1,1), got ");
					piglit_merge_result(result, PIGLIT_FAIL);
				}
				printf("(%zu,%zu,%zu)", v[0], v[1], v[2]);
			}
			break;
		case CL_KERNEL_LOCAL_MEM_SIZE:
			if (check_size(sizeof(cl_ulong), param_value_size, result)) {
				print_u(*(cl_ulong*)param_value);
			}
			break;
		case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
			if (check_size(sizeof(size_t), param_value_size, result)) {
				print_u(*(size_t*)param_value);
			}
			break;
		case CL_KERNEL_PRIVATE_MEM_SIZE:
			if (check_size(sizeof(cl_ulong), param_value_size, result)) {
				print_u(*(cl_ulong*)param_value);
			}
			break;
		case CL_KERNEL_GLOBAL_WORK_SIZE:
			if (is_custom_device &&
			    check_size(sizeof(size_t) * 3, param_value_size, result)) {
				size_t* v = (size_t*)param_value;
				printf(": (%zu,%zu,%zu)", v[0], v[1], v[2]);
			}
			break;
		default:
			printf(": WARN unchecked value");
			piglit_merge_result(result, PIGLIT_WARN);
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
	cl_int errNo;
	cl_kernel kernel;
	cl_uint* dev_count_ptr;

	cl_bool is_custom_device = false;

	size_t param_value_size;
	void* param_value;
	
	int num_kernel_work_group_infos =
		PIGLIT_CL_ENUM_NUM(cl_kernel_work_group_info, env->version);
	const cl_kernel_work_group_info* kernel_work_group_infos =
		PIGLIT_CL_ENUM_ARRAY(cl_kernel_work_group_info);

	kernel = clCreateKernel(env->program,
	                        "dummy_kernel",
	                        &errNo);
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Failed (error code: %s): Create kernel.\n",
		        piglit_cl_get_error_name(errNo));
		return PIGLIT_FAIL;
	}

	/*** Normal usage ***/
	for(i = 0; i < num_kernel_work_group_infos; i++) {
		cl_int success_code = CL_SUCCESS;
		param_value_size = 0;

#if defined(CL_VERSION_1_2)
		/* CL_KERNEL_GLOBAL_WORK_SIZE query
		 * is valid for custom device or build-in kernel
		 */
		if (kernel_work_group_infos[i] == CL_KERNEL_GLOBAL_WORK_SIZE) {
			cl_device_type* dev_type_ptr =
			   piglit_cl_get_device_info(env->device_id, CL_DEVICE_TYPE);
			is_custom_device = *dev_type_ptr == CL_DEVICE_TYPE_CUSTOM;
			if (!is_custom_device)
				success_code = CL_INVALID_VALUE;
			free(dev_type_ptr);
		}
#endif

		printf("%s", piglit_cl_get_enum_name(kernel_work_group_infos[i]));

		errNo = clGetKernelWorkGroupInfo(kernel,
		                                 env->device_id,
		                                 kernel_work_group_infos[i],
		                                 0,
		                                 NULL,
		                                 &param_value_size);
		if(!piglit_cl_check_error(errNo, success_code)) {
			fprintf(stderr,
			        ": Failed (error code: %s): Get size of %s.\n",
			        piglit_cl_get_error_name(errNo),
			        piglit_cl_get_enum_name(kernel_work_group_infos[i]));
			piglit_merge_result(&result, PIGLIT_FAIL);
			continue;
		}

		param_value = malloc(param_value_size);
		errNo = clGetKernelWorkGroupInfo(kernel,
		                                 env->device_id,
		                                 kernel_work_group_infos[i],
		                                 param_value_size,
		                                 param_value,
		                                 NULL);
		if(!piglit_cl_check_error(errNo, success_code)) {
			fprintf(stderr,
			        ": Failed (error code: %s): Get value of %s.\n",
			        piglit_cl_get_error_name(errNo),
			        piglit_cl_get_enum_name(kernel_work_group_infos[i]));
			piglit_merge_result(&result, PIGLIT_FAIL);
		}

		check_info(is_custom_device, kernel_work_group_infos[i], param_value, param_value_size, &result);

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
	errNo = clGetKernelWorkGroupInfo(kernel,
	                                 env->device_id,
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

	errNo = clGetKernelWorkGroupInfo(kernel,
	                                 env->device_id,
	                                 CL_KERNEL_WORK_GROUP_SIZE,
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
	 * CL_INVALID_KERNEL if kernel is a not a valid kernel object.
	 */
	errNo = clGetKernelWorkGroupInfo(NULL,
	                                 env->device_id,
	                                 CL_KERNEL_WORK_GROUP_SIZE,
	                                 0,
	                                 NULL,
	                                 &param_value_size);
	if(!piglit_cl_check_error(errNo, CL_INVALID_KERNEL)) {
		fprintf(stderr,
		        "Failed (error code: %s): Trigger CL_INVALID_KERNEL if kernel is not a valid kernel object.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}

	/*
	 * CL_INVALID_DEVICE if device is not in the list of devices associated with
	 * kernel or if device is NULL but there is more than one device associated
	 * with kernel
	 * or
	 * CL_SUCCESS if device is NULL but there is only one device associated with kernel.
	 */
	dev_count_ptr = piglit_cl_get_program_info(env->program, CL_PROGRAM_NUM_DEVICES);
	errNo = clGetKernelWorkGroupInfo(kernel,
	                                 NULL,
	                                 CL_KERNEL_WORK_GROUP_SIZE,
	                                 0,
	                                 NULL,
	                                 &param_value_size);
	if (*dev_count_ptr == 1) {
		if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
			fprintf(stderr,
			        "Failed (error code: %s): Trigger CL_SUCCESS if device is NULL but there is only one device associated with kernel.\n",
			        piglit_cl_get_error_name(errNo));
			piglit_merge_result(&result, PIGLIT_FAIL);
		}
	} else {
		if(!piglit_cl_check_error(errNo, CL_INVALID_DEVICE)) {
			fprintf(stderr,
			        "Failed (error code: %s): Trigger CL_INVALID_DEVICE if device is NULL but there is more than one device associated with kernel.\n",
			        piglit_cl_get_error_name(errNo));
			piglit_merge_result(&result, PIGLIT_FAIL);
		}
	}
	free(dev_count_ptr);

	clReleaseKernel(kernel);

	return result;
}
