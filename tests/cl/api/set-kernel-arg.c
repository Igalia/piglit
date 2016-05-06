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
 * @file set-kernel-arg.c
 *
 * Test API function:
 *
 *   cl_int clSetKernelArg (cl_kernel kernel,
 *                          cl_uint arg_index,
 *                          size_t arg_size,
 *                          const void *arg_value)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clSetKernelArg";
	config.version_min = 10;

	config.run_per_device = true;
	config.create_context = true;

	config.program_source =
		"typedef struct struct_arg {\n"
		"   int   m1;\n"
		"   int4  m2;\n"
		"   float m3;\n"
		"} struct_arg_t;\n"
		"\n"
		"kernel void kernel_fun(__global int* arr,     \
		                        float float_num,       \
		                        __local int* int_ptr,  \
		                        sampler_t sampler,     \
		                        int3 vec3,             \
		                        struct_arg_t s_arg) {}";

PIGLIT_CL_API_TEST_CONFIG_END

static void
test (cl_kernel kernel,
      cl_uint arg_index,
      size_t arg_size,
      const void* arg_value,
      cl_int expected_error,
      enum piglit_result *result,
      const char* test_str)
{
	cl_int errNo;
	enum piglit_result res = PIGLIT_PASS;

	errNo = clSetKernelArg(kernel, arg_index, arg_size, arg_value);
	if(!piglit_cl_check_error(errNo, expected_error)) {
		fprintf(stderr, "Failed (error code: %s): %s.\n",
		        piglit_cl_get_error_name(errNo), test_str);
		res = PIGLIT_FAIL;
		piglit_merge_result(result, PIGLIT_FAIL);
	}
	if (expected_error == CL_SUCCESS)
		piglit_report_subtest_result(res, "%s", test_str);
}

enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_api_test_config* config,
               const struct piglit_cl_api_test_env* env)
{
	typedef struct struct_arg {
		cl_int   m1;
		cl_int4  m2;
		cl_float m3;
	} struct_arg_t;

	enum piglit_result result = PIGLIT_PASS;
	const char * test_str = NULL;

	cl_int errNo;
	cl_kernel kernel;

	cl_mem buffer;
	cl_float float_num = 1.1;
	cl_int int_num = 1;
	cl_sampler sampler = NULL;
	cl_int3 vec3;
	struct_arg_t s_arg;

	cl_mem invalid_buffer;

	cl_bool image_support =
		piglit_cl_get_device_image_support(env->context->device_ids[0]);

	/*** Normal usage ***/
	kernel = clCreateKernel(env->program, "kernel_fun", &errNo);
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Failed (error code: %s): Create kernel.\n",
		        piglit_cl_get_error_name(errNo));
		return PIGLIT_FAIL;
	}

	buffer = clCreateBuffer(env->context->cl_ctx,
	                        CL_MEM_READ_WRITE,
	                        512,
	                        NULL,
	                        &errNo);
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Failed (error code: %s): Create buffer.\n",
		        piglit_cl_get_error_name(errNo));
		return PIGLIT_FAIL;
	}

	if (image_support) {
		sampler = clCreateSampler(env->context->cl_ctx,
		                          CL_TRUE,
		                          CL_ADDRESS_NONE,
		                          CL_FILTER_NEAREST,
		                          &errNo);
		if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
			fprintf(stderr,
			        "Failed (error code: %s): Create sampler.\n",
			        piglit_cl_get_error_name(errNo));
			return PIGLIT_FAIL;
		}
	}

	test(kernel, 0, sizeof(cl_mem), &buffer,
	     CL_SUCCESS, &result,
	     "Set kernel argument for buffer");
	test(kernel, 1, sizeof(cl_float), &float_num,
	     CL_SUCCESS, &result,
	     "Set kernel argument for scalar");
	test(kernel, 2, sizeof(cl_int), NULL,
	     CL_SUCCESS, &result,
	     "Set kernel argument for array");

	test_str = "Set kernel argument for sampler";
	if (image_support) {
		test(kernel, 3, sizeof(cl_sampler), &sampler,
		     CL_SUCCESS, &result, test_str);
	} else {
		piglit_report_subtest_result(PIGLIT_SKIP, "%s", test_str);
	}

	test(kernel, 4, sizeof(cl_int3), &vec3,
	     CL_SUCCESS, &result,
	     "Set kernel argument for cl_int3");

	test(kernel, 5, sizeof(struct_arg_t), &s_arg,
	     CL_SUCCESS, &result,
	     "Set kernel argument for struct");

	/*
	 * Next line is also valid.
	 *
	 * If the argument is a buffer object, the arg_value pointer can be NULL or
	 * point to a NULL value...
	 */
	test(kernel, 0, sizeof(cl_mem), NULL,
	     CL_SUCCESS, &result,
	     "Set kernel argument for buffer which is NULL");

	/*** Errors ***/
	enum piglit_result input_check_result = PIGLIT_PASS;
	/*
	 * CL_INVALID_KERNEL if kernel is not a valid kernel object.
	 */
	test(NULL, 1, sizeof(cl_float), &float_num,
	     CL_INVALID_KERNEL, &input_check_result,
	     "Trigger CL_INVALID_KERNEL if kernel is not a valid kernel object");

	/*
	 * CL_INVALID_ARG_INDEX if arg_index is not a valid argument index.
	 */
	test(kernel, 11, sizeof(cl_float), &float_num,
	     CL_INVALID_ARG_INDEX, &input_check_result,
	     "Trigger CL_INVALID_ARG_INDEX if arg_index is not a valid argument index");

	/*
	 * CL_INVALID_ARG_VALUE if arg_value specified is NULL for an argument that
	 * is not declared with the __local qualifier or vice-versa.
	 *
	 * Version: 1.0
	 *
	 * CL_INVALID_ARG_VALUE if arg_value specified is not a valid value.
	 *
	 * Version : 1.2
	 */
	test(kernel, 1, sizeof(cl_float), NULL,
	     CL_INVALID_ARG_VALUE, &input_check_result,
	     "Trigger CL_INVALID_ARG_VALUE if arg_value specified is NULL for an argument that is not declared with the __local qualifier");
	test(kernel, 2, sizeof(cl_int), &int_num,
	     CL_INVALID_ARG_VALUE, &input_check_result,
	     "Trigger CL_INVALID_ARG_VALUE if arg_value specified is not NULL for an argument that is declared with the __local qualifier");

	/*
	 * CL_INVALID_MEM_OBJECT for an argument declared to be a memory object when
	 * the specified arg_value is not a valid memory object.
	 */
	test_str = "Trigger CL_INVALID_MEM_OBJECT for an argument declared to be a memory object when the specified arg_value is not a valid memory object.";
	errNo = clSetKernelArg(kernel, 0, sizeof(cl_mem), &invalid_buffer);
	if(   errNo != CL_INVALID_MEM_OBJECT
	   && errNo != CL_INVALID_ARG_VALUE) { // two possible values
		piglit_cl_check_error(errNo, CL_INVALID_MEM_OBJECT);
		fprintf(stderr,
		        "Failed (error code: %s): %s\n",
		        piglit_cl_get_error_name(errNo), test_str);
		piglit_merge_result(&input_check_result, PIGLIT_FAIL);
	}

	/*
	 * CL_INVALID_SAMPLER for an argument declared to be of type sampler_t when
	 * the specified arg_value is not a valid sampler object.
	 */
	test_str = "Trigger CL_INVALID_SAMPLER for an argument declared to be a memory object when the specified arg_value is not a valid memory object.";
	errNo = clSetKernelArg(kernel, 3, sizeof(cl_sampler), NULL);
	if(   errNo != CL_INVALID_SAMPLER
	   && errNo != CL_INVALID_ARG_VALUE) { // two possible values
		piglit_cl_check_error(errNo, CL_INVALID_SAMPLER);
		fprintf(stderr, "Failed (error code: %s): %s.\n",
		        piglit_cl_get_error_name(errNo), test_str);
		piglit_merge_result(&input_check_result, PIGLIT_FAIL);
	}

	/*
	 * CL_INVALID_ARG_SIZE if arg_size does not match the size of the data type
	 * for an argument that is not a memory object or if the argument is a
	 * memory object and arg_size != sizeof(cl_mem) or if arg_size is zero and
	 * the argument is declared with the __local qualifier or if the argument is
	 * a sampler and arg_size != sizeof(cl_sampler).
	 */
	test(kernel, 1, sizeof(cl_float)+1, &float_num,
	     CL_INVALID_ARG_SIZE, &input_check_result,
	     "Trigger CL_INVALID_ARG_SIZE if arg_size does not match the size of the data type for an argument that is not a memory object");
	test(kernel, 0, sizeof(cl_mem)+1, &buffer,
	     CL_INVALID_ARG_SIZE, &input_check_result,
	     "Trigger CL_INVALID_ARG_SIZE if the argument is a memory object and arg_size != sizeof(cl_mem)");
	test(kernel, 2, 0, NULL,
	     CL_INVALID_ARG_SIZE, &input_check_result,
	     "Trigger CL_INVALID_ARG_SIZE if arg_size is zero and the argument is declared with the __local qualifier");
	test(kernel, 3, sizeof(cl_sampler)+1, &sampler,
	     CL_INVALID_ARG_SIZE, &input_check_result,
	     "Trigger CL_INVALID_ARG_SIZE if the argument is a sampler and arg_size != sizeof(cl_sampler)");

	/*
	 * CL_INVALID_ARG_VALUE if the argument is an image declared with the
	 * read_only qualifier and arg_value refers to an image object created with
	 * cl_mem_flags of CL_MEM_WRITE or if the image argument is declared with
	 * the write_only qualifier and arg_value refers to an image object created
	 * with cl_mem_flags of CL_MEM_READ.
	 *
	 * Version: 1.2
	 *
	 * TODO
	 */
	piglit_report_subtest_result(input_check_result, "Input checks");
	piglit_merge_result(&result, input_check_result);

	clReleaseMemObject(buffer);
	clReleaseSampler(sampler);
	clReleaseKernel(kernel);

	return result;
}
