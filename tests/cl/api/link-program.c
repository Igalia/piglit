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
 */

/**
 * @file link-program.c
 *
 * Test API function:
 *
 *   cl_program
 *   clLinkProgram(cl_context context,
 *         cl_uint num_devices, const cl_device_id device_list,
 *         const char *options,
 *         cl_uint num_input_programs, const cl_program *input_programs,
 *         void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
 *         void *user_data,
 *         cl_int *errcode_ret)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clLinkProgram";
	config.version_min = 12;

	config.run_per_platform = true;
	config.create_context = true;

PIGLIT_CL_API_TEST_CONFIG_END


const char* strings[] = {
	"int get_number(void) { return 42; }\n",
	"int get_number(void);\n",
	"kernel void test_kernel(void) { int i = get_number(); }\n",
	"int get_number(void) { return 0; }\n"
};

#if defined(CL_VERSION_1_2)
static cl_program
compile_program(cl_context context,
                cl_uint num_devices, const cl_device_id *device_list,
                cl_uint count, const char **strings,
                const char* err_str) {
	cl_int errNo;
	cl_program program;

	/* Create program with source */
	program = clCreateProgramWithSource(context,
	                                    count,
	                                    strings,
	                                    NULL,
	                                    &errNo);
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Failed (error code: %s): Create program with source (for the %s).\n",
		        piglit_cl_get_error_name(errNo), err_str);
		return NULL;
	}

	/* Compile program */
	errNo = clCompileProgram(program,
	                       num_devices, device_list,
	                       " ",
	                       0, NULL, NULL,
	                       NULL, NULL);

	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		int i;
		fprintf(stderr, "Failed (error code: %s): Compile program (for the %s).\n",
		        piglit_cl_get_error_name(errNo), err_str);

		for(i = 0; i < num_devices; ++i) {
			char *build_log = piglit_cl_get_program_build_info(program, device_list[i], CL_PROGRAM_BUILD_LOG);
			fprintf(stderr, "Build log:\n%s\n", build_log);
			free(build_log);
		}

		clReleaseProgram(program);
		return NULL;
	}

	return program;
}

static bool
test(cl_context context,
     cl_uint num_devices, const cl_device_id *device_list,
     const char *options,
     cl_uint num_input_programs, const cl_program *input_programs,
     void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
     void *user_data,
     cl_program *ret_program,
     cl_int expected_error, enum piglit_result* result,
     const char* test_str) {
	cl_program program;
	cl_int errNo;

	program = clLinkProgram(context,
	                        num_devices, device_list,
	                        options,
	                        num_input_programs, input_programs,
	                        pfn_notify, user_data,
	                        &errNo);

	if (ret_program) {
		*ret_program = program;
	} else {
		if (program)
			clReleaseProgram(program);
	}

	if(!piglit_cl_check_error(errNo, expected_error)) {
		fprintf(stderr, "Failed (error code: %s): %s.\n",
		        piglit_cl_get_error_name(errNo), test_str);
		piglit_merge_result(result, PIGLIT_FAIL);
		return false;
	}

	return true;
}
#endif

enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_api_test_config* config,
               const struct piglit_cl_api_test_env* env)
{
#if defined(CL_VERSION_1_2)
	enum piglit_result result = PIGLIT_PASS;

	bool linker_available;
	int i;
	cl_program_binary_type* binary_type;
	cl_program compiled_programs[2];
	cl_program function_prog;
	cl_program kernel_prog;
	cl_program linked_prog;

	linker_available = false;
	for(i = 0; i < env->context->num_devices; ++i) {
		cl_bool* dev_linker =
			piglit_cl_get_device_info(env->context->device_ids[i],
			                          CL_DEVICE_LINKER_AVAILABLE);

		if (*dev_linker)
			linker_available |= true;

		free(dev_linker);
	}

	if (!linker_available)
		return PIGLIT_SKIP;

	/* Create compiled program */
	function_prog = compile_program(env->context->cl_ctx,
	                                env->context->num_devices, env->context->device_ids,
	                                1, &strings[0],
	                                "function program");
	kernel_prog = compile_program(env->context->cl_ctx,
	                              env->context->num_devices, env->context->device_ids,
	                              2, &strings[1],
                                 "kernel program");

	if (!function_prog || !kernel_prog) {
		clReleaseProgram(function_prog);
		clReleaseProgram(kernel_prog);
		return PIGLIT_FAIL;
	}

	compiled_programs[0] = function_prog;
	compiled_programs[1] = kernel_prog;

	/*** Normal usage ***/
	test(env->context->cl_ctx,
	     env->context->num_devices, env->context->device_ids,
	     "-create-library",
	     1, compiled_programs,
	     NULL, NULL,
	     &linked_prog,
	     CL_SUCCESS, &result, "Link program as library");

	for(i = 0; i < env->context->num_devices; ++i) {
		binary_type = piglit_cl_get_program_build_info(linked_prog,
		                                               env->context->device_ids[i],
		                                               CL_PROGRAM_BINARY_TYPE);
		if (*binary_type != CL_PROGRAM_BINARY_TYPE_LIBRARY) {
			piglit_merge_result(&result, PIGLIT_FAIL);
			fprintf(stderr,
		           "Failed: binary is not of type CL_PROGRAM_BINARY_TYPE_LIBRARY.\n");
		}
		free(binary_type);
	}

	clReleaseProgram(linked_prog);

	test(env->context->cl_ctx,
	     env->context->num_devices, env->context->device_ids,
	     "",
	     2, compiled_programs,
	     NULL, NULL,
	     &linked_prog,
	     CL_SUCCESS, &result, "Link program as executable");

	for(i = 0; i < env->context->num_devices; ++i) {
		binary_type = piglit_cl_get_program_build_info(linked_prog,
		                                               env->context->device_ids[i],
		                                               CL_PROGRAM_BINARY_TYPE);
		if (*binary_type != CL_PROGRAM_BINARY_TYPE_EXECUTABLE) {
			piglit_merge_result(&result, PIGLIT_FAIL);
			fprintf(stderr,
		           "Failed: binary is not of type CL_PROGRAM_BINARY_TYPE_EXECUTABLE.\n");
		}
		free(binary_type);
	}


	/*** Errors ***/

	/*
	 * CL_INVALID_VALUE if device_list is NULL and num_devices is greater than
	 * zero, or if device_list is not NULL and num_devices is zero
	 */
	test(env->context->cl_ctx,
	     env->context->num_devices, NULL,
	     "",
	     2, compiled_programs,
	     NULL, NULL,
	     NULL,
	     CL_INVALID_VALUE, &result,
	     "Trigger CL_INVALID_VALUE if device_list is NULL and num_devices is greater than zero");

	test(env->context->cl_ctx,
	     0, env->context->device_ids,
	     "",
	     2, compiled_programs,
	     NULL, NULL,
	     NULL,
	     CL_INVALID_VALUE, &result,
	     "Trigger CL_INVALID_VALUE if device_list is not NULL and num_devices is zero");

	/*
	 * CL_INVALID_VALUE if num_input_programs is zero and input_programs is NULL
	 * or if num_input_programs is zero and input_programs is not NULL
	 * or if num_input_programs is not zero and input_programs is NULL
	 */
	test(env->context->cl_ctx,
	     env->context->num_devices, env->context->device_ids,
	     "",
	     0, NULL,
	     NULL, NULL,
	     NULL,
	     CL_INVALID_VALUE, &result,
	     "Trigger CL_INVALID_VALUE if num_input_programs is zero and input_programs is NULL");

	test(env->context->cl_ctx,
	     env->context->num_devices, env->context->device_ids,
	     "",
	     0, compiled_programs,
	     NULL, NULL,
	     NULL,
	     CL_INVALID_VALUE, &result,
	     "Trigger CL_INVALID_VALUE if num_input_programs is zero and input_programs is not NULL");

	test(env->context->cl_ctx,
	     env->context->num_devices, env->context->device_ids,
	     "",
	     2, NULL,
	     NULL, NULL,
	     NULL,
	     CL_INVALID_VALUE, &result,
	     "Trigger CL_INVALID_VALUE if num_input_programs is not zero and input_programs is NULL");

	/*
	 * CL_INVALID_PROGRAM if programs specified in input_programs are not valid program objects
	 */


	/*
	 * CL_INVALID_VALUE if pfn_notify is NULL but user_data is not NULL.
	 */
	test(env->context->cl_ctx,
	     env->context->num_devices, env->context->device_ids,
	     "",
	     2, compiled_programs,
	     NULL, &i,
	     NULL,
	     CL_INVALID_VALUE, &result,
	     "Trigger CL_INVALID_VALUE if pfn_notify is NULL but user_data is not NULL");

	/*
	 * CL_INVALID_DEVICE if OpenCL devices listed in device_list are not in the
	 * list of devices associated with context
	 */


	/*
	 * CL_INVALID_LINKER_OPTIONS if the linker options specified by options are
	 * invalid
	 */
	test(env->context->cl_ctx,
	     env->context->num_devices, env->context->device_ids,
	     "-invalid- --link-- options",
	     2, compiled_programs,
	     NULL, NULL,
	     NULL,
	     CL_INVALID_LINKER_OPTIONS, &result,
	     "Trigger CL_INVALID_LINKER_OPTIONS if the linker options specified by options are invalid");

	/*
	 * CL_INVALID_OPERATION if the compilation or build of a program executable
	 * for any of the devices listed in device_list by a previous call to
	 * clCompileProgram or clBuildProgram for program has not completed
	 */


	/*
	 * CL_INVALID_OPERATION if the rules for devices containing compiled binaries
	 * or libraries as described in input_programs argument above are not followed
	 */
	compiled_programs[0] = linked_prog;
	test(env->context->cl_ctx,
	     env->context->num_devices, env->context->device_ids,
	     "",
	     2, compiled_programs,
	     NULL, NULL,
	     NULL,
	     CL_INVALID_OPERATION, &result,
	     "Trigger CL_INVALID_OPERATION if the rules for devices containing compiled binaries or libraries as described in input_programs argument above are not followed");

	/*
	 * CL_LINKER_NOT_AVAILABLE if a linker is not available
	 * i.e. CL_DEVICE_LINKER_AVAILABLE specified in the table of allowed values
	 * for param_name for clGetDeviceInfo is set to CL_FALSE.
	 */
	for(i = 0; i < env->context->num_devices; ++i) {
		cl_bool* dev_linker =
			piglit_cl_get_device_info(env->context->device_ids[i],
			                          CL_DEVICE_LINKER_AVAILABLE);
		if(!(*dev_linker)) {
			test(env->context->cl_ctx,
			     1, &env->context->device_ids[i],
			     "",
			     2, compiled_programs,
			     NULL, NULL,
			     NULL,
			     CL_LINKER_NOT_AVAILABLE, &result,
			     "Trigger CL_LINKER_NOT_AVAILABLE if a linker is not available");
		}
		free(dev_linker);
	}


	/* Release programs */
	clReleaseProgram(function_prog);
	clReleaseProgram(kernel_prog);
	clReleaseProgram(linked_prog);

	/*
	 * CL_LINK_PROGRAM_FAILURE if there is a failure to link the compiled binaries
	 * and/or libraries.
	 */
	function_prog = compile_program(env->context->cl_ctx,
	                                env->context->num_devices, env->context->device_ids,
	                                1, &strings[0],
	                                "2nd function program");
	kernel_prog = compile_program(env->context->cl_ctx,
	                              env->context->num_devices, env->context->device_ids,
	                              3, &strings[1],
	                              "2nd kernel program");

	if (!function_prog || !kernel_prog) {
		result = PIGLIT_FAIL;
	} else {
		compiled_programs[0] = function_prog;
		compiled_programs[1] = kernel_prog;

		test(env->context->cl_ctx,
		     env->context->num_devices, env->context->device_ids,
		     "",
		     2, compiled_programs,
		     NULL, NULL,
		     NULL,
		     CL_LINK_PROGRAM_FAILURE, &result,
		     "Trigger CL_LINK_PROGRAM_FAILURE if there is a failure to link the compiled binaries and/or libraries");
	}

	/* Release programs */
	clReleaseProgram(function_prog);
	clReleaseProgram(kernel_prog);

	return result;
#else
	return PIGLIT_SKIP;
#endif
}
