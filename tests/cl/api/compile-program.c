/*
 * Copyright © 2014 EdB <edb+piglit@sigluy.net>
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
 *
 * created from build-program.c
 * Copyright © 2012 Blaž Tomažič <blaz.tomazic@gmail.com>
 */

/**
 * @file compile-program.c
 *
 * Test API function:
 *
 *   cl_int clCompileProgram(cl_program d_prog, cl_uint num_devs,
 *               const cl_device_id *d_devs, const char *p_opts,
 *               cl_uint num_headers, const cl_program *d_header_progs,
 *               const char **headers_names,
 *               void (*pfn_notify)(cl_program, void *),
 *               void *user_data)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clCompileProgram";
	config.version_min = 12;

	config.run_per_platform = true;
	config.create_context = true;

PIGLIT_CL_API_TEST_CONFIG_END


const char* strings[] = {
	"#include \"header.h\"\n",
	"kernel void dummy_kernel() { w_int i = 0; }",
	"kernel void dummy_kernel() { int i = 0; }"
};

const char* headers_strings[] = {
	"typedef int w_int;",
	"int w_int;" //to trigger invalid
};

const char* headers_names[] = {
	"header.h"
};

const char* empty_strings[] = {
	""
};

static bool
test(cl_program program,
     cl_uint num_devices, const cl_device_id *device_list,
     const char *options,
     cl_uint num_headers, const cl_program *d_header_progs, const char **headers_names,
     void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data), void *user_data,
     cl_int expected_error,
     enum piglit_result* result,
     const char* test_str) {
	cl_int errNo;

	errNo = clCompileProgram(program,
	                       num_devices, device_list,
	                       options,
	                       num_headers, d_header_progs, headers_names,
	                       pfn_notify, user_data);

	if(!piglit_cl_check_error(errNo, expected_error)) {
		fprintf(stderr, "Failed (error code: %s): %s.\n",
		        piglit_cl_get_error_name(errNo), test_str);
		piglit_merge_result(result, PIGLIT_FAIL);
		return false;
	}

	return true;
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
	cl_program header;
	cl_program header_invalid;
	cl_program program;
	cl_program temp_program;
	cl_kernel kernel;

	/*** Normal usage ***/

	/* Create program */

	/* with source */
	header = clCreateProgramWithSource(env->context->cl_ctx,
	                                   1,
	                                   &headers_strings[0],
	                                   NULL,
	                                   &errNo);
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Failed (error code: %s): Create program with source.\n",
		        piglit_cl_get_error_name(errNo));
		return PIGLIT_FAIL;
	}

	program = clCreateProgramWithSource(env->context->cl_ctx,
	                                    2,
	                                    strings,
	                                    NULL,
	                                    &errNo);
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Failed (error code: %s): Create program with source.\n",
		        piglit_cl_get_error_name(errNo));
		return PIGLIT_FAIL;
	}

	test(program, env->context->num_devices, env->context->device_ids,
	     "",
	     1, &header, headers_names,
	     NULL, NULL,
	     CL_SUCCESS, &result, "Compile program");


	// TODO: test callback

	/*** Errors ***/

	/*
	 * CL_INVALID_PROGRAM if program is not a valid program object.
	 */
	test(NULL, env->context->num_devices, env->context->device_ids,
	     "",
	     1, &header, headers_names,
	     NULL, NULL,
	     CL_INVALID_PROGRAM, &result,
	     "Trigger CL_INVALID_PROGRAM if program is not a valid program object");

	/*
	 * CL_INVALID_VALUE if device_list is NULL and num_devices is greater than
	 * zero, or if device_list is not NULL and num_devices is zero.
	 */
	test(program, 1, NULL,
	     "",
	     1, &header, headers_names,
	     NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "Trigger CL_INVALID_VALUE if device_list is NULL and num_devices is greater than zero");
	test(program, 0, env->context->device_ids,
	     "",
	     1, &header, headers_names,
	     NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "Trigger CL_INVALID_VALUE if device_list is not NULL and num_devices is zero");

	/*
	 * CL_INVALID_VALUE if num_input_headers is zero and header_include_names or
	 * input_headers are not NULL or if num_input_headers is not zero and
	 * header_include_names or input_headers are NULL.
	 */
	test(program, 0, env->context->device_ids,
	     "",
	     0, &header, NULL,
	     NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "Trigger CL_INVALID_VALUE if num_input_headers is zero and header_include_names or input_headers are not NULL");
	test(program, 0, env->context->device_ids,
	     "",
	     0, NULL, headers_names,
	     NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "Trigger CL_INVALID_VALUE if num_input_headers is zero and header_include_names or input_headers are not NULL");
	test(program, 0, env->context->device_ids,
	     "",
	     1, &header, NULL,
	     NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "Trigger CL_INVALID_VALUE if num_input_headers is not zero and header_include_names or input_headers are NULL.");
	test(program, 0, env->context->device_ids,
	     "",
	     1, NULL, headers_names,
	     NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "Trigger CL_INVALID_VALUE if num_input_headers is not zero and header_include_names or input_headers are NULL.");


	/*
	 * CL_INVALID_VALUE if pfn_notify is NULL but user_data is not NULL.
	 */
	test(program, env->context->num_devices, env->context->device_ids,
	     "",
	     1, &header, headers_names,
	     NULL, &result,
	     CL_INVALID_VALUE, &result,
	     "Trigger CL_INVALID_VALUE if pfn_notify is NULL and user_data is not NULL");

	/*
	 * CL_INVALID_DEVICE if OpenCL devices listed in device_list are not in the
	 * list of devices associated with program.
	 *
	 * TODO
	 */

	/*
	 * CL_INVALID_COMPILER_OPTIONS if the build options specified by options are
	 * invalid.
	 */
	test(program, env->context->num_devices, env->context->device_ids,
	     "-invalid- --build-- options",
	     1, &header, headers_names,
	     NULL, NULL,
	     CL_INVALID_COMPILER_OPTIONS, &result,
	     "Trigger CL_INVALID_COMPILER_OPTIONS if the build options specified by options are invalid");

	/*
	 * CL_INVALID_OPERATION if the compilation or build of a program executable for any of
	 * the devices listed in device_list by a previous call to clCompileProgram or
	 * clBuildProgram for program has not completed.
	 *
	 * TODO
	 */

	/*
	 * CL_COMPILER_NOT_AVAILABLE if program is created with
	 * clCreateProgramWithSource and a compiler is not available i.e.
	 * CL_DEVICE_COMPILER_AVAILABLE specified in the table of OpenCL Device
	 * Queries for clGetDeviceInfo is set to CL_FALSE.
	 *
	 * Note: If this is true for any device, then a normal usage test returns a
	 * false error.
	 */
	for(i = 0; i < env->context->num_devices; i++) {
		cl_bool* compiler_available =
			piglit_cl_get_device_info(env->context->device_ids[i],
			                          CL_DEVICE_COMPILER_AVAILABLE);
		if(!(*compiler_available)) {
			test(program, env->context->num_devices, env->context->device_ids,
			     "",
			     1, &header, headers_names,
			     NULL, NULL,
			     CL_COMPILER_NOT_AVAILABLE, &result,
			     "Trigger CL_COMPILER_NOT_AVAILABLE if program is created with clCreateProgramWithSource and a compiler is not available");
		}
		free(compiler_available);
	}

	/*
	 * CL_COMPILE_PROGRAM_FAILURE if there is a failure to compile the program source.
	 * This error will be returned if clCompileProgram does not return until the compile has
	 * completed.
	 */
	header_invalid = clCreateProgramWithSource(env->context->cl_ctx,
	                                           1,
	                                           &headers_strings[1],
	                                           NULL,
	                                           &errNo);
	if(piglit_cl_check_error(errNo, CL_SUCCESS)) {
		test(program, env->context->num_devices, env->context->device_ids,
		     "",
		     1, &header_invalid, headers_names,
		     NULL, NULL,
		     CL_COMPILE_PROGRAM_FAILURE, &result,
		     "Trigger CL_COMPILE_PROGRAM_FAILURE if there is a failure to compile the program source");
		clReleaseProgram(header_invalid);
	}

	/*
	 * CL_INVALID_OPERATION if there are kernel objects attached to program.
	 */
	temp_program = clCreateProgramWithSource(env->context->cl_ctx,
	                                    1,
	                                    &strings[2],
	                                    NULL,
	                                    &errNo);
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Failed (error code: %s): Create temp program with source.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);;
	} else {
		errNo = clBuildProgram(temp_program,
		                       env->context->num_devices, env->context->device_ids,
		                       "",
		                       NULL, NULL);
		if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
			fprintf(stderr, "Failed (error code: %s): clBuildProgram.\n",
			        piglit_cl_get_error_name(errNo));
			piglit_merge_result(&result, PIGLIT_FAIL);
		} else {
			kernel = clCreateKernel(temp_program, "dummy_kernel", &errNo);
			if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
				fprintf(stderr, "Failed (error code: %s): clCreateKernel.\n",
			        piglit_cl_get_error_name(errNo));
				piglit_merge_result(&result, PIGLIT_FAIL);
			} else {
				test(temp_program, env->context->num_devices, env->context->device_ids,
				     "",
				     1, &header, headers_names,
				     NULL, NULL,
				     CL_INVALID_OPERATION, &result,
				     "Trigger CL_INVALID_OPERATION if there are kernel objects attached to program");
			}
		}
	}
	clReleaseKernel(kernel);
	clReleaseProgram(temp_program);

	/*
	 * CL_SUCCESS when compiling an empty string
	 */
	temp_program = clCreateProgramWithSource(env->context->cl_ctx,
	                                         1,
	                                         empty_strings,
	                                         NULL,
	                                         &errNo);
	if(piglit_cl_check_error(errNo, CL_SUCCESS)) {
		test(temp_program, env->context->num_devices, env->context->device_ids,
		     "",
		     0, NULL, NULL,
		     NULL, NULL,
		     CL_SUCCESS, &result,
		     "CL_SUCCESS when compiling an empty string.");
		clReleaseProgram(temp_program);
	}

	/*
	 * CL_INVALID_OPERATION if program was not created with
	 * clCreateProgramWithSource.
	 *
	 * Version: 1.2
	 *
	 * TODO
	 */

	clReleaseProgram(header);
	clReleaseProgram(program);

	return result;
}
