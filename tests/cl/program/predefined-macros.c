/*
 * Copyright © 2016 Niels Ole Salscheider <niels_ole@salscheider-online.de>
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

#include "piglit-framework-cl-program.h"

char *program_source =
"kernel void test(global int* file_defined, global int* line_defined, \n"
"                 global int* opencl_version_defined, global int* opencl_version, \n"
"                 global int* opencl_c_version_defined, global int* opencl_c_version, \n"
"                 global int* cl_version_defined, global int* cl_version, \n"
"                 global int* endian_little_defined, global int* endian_little, \n"
"                 global int* image_support_defined, global int* image_support) \n"
"{ \n"
"#ifdef __FILE__ \n"
"	*file_defined = 1; \n"
"#else \n"
"	*file_defined = 0; \n"
"#endif \n"
"\n"
"#ifdef __LINE__ \n"
"	*line_defined = 1; \n"
"#else \n"
"	*line_defined = 0; \n"
"#endif \n"
"\n"
"#ifdef __OPENCL_VERSION__ \n"
"	*opencl_version_defined = 1; \n"
"	*opencl_version = __OPENCL_VERSION__; \n"
"#else \n"
"	*opencl_version_defined = 0; \n"
"#endif \n"
"\n"
"#ifdef __OPENCL_C_VERSION__ \n"
"	*opencl_c_version_defined = 1; \n"
"	*opencl_c_version = __OPENCL_C_VERSION__; \n"
"#else \n"
"	*opencl_c_version_defined = 0; \n"
"#endif \n"
"\n"
"#ifdef CL_VERSION_1_0 \n"
"	cl_version_defined[0] = 1; \n"
"	cl_version[0] = CL_VERSION_1_0; \n"
"#else \n"
"	cl_version_defined[0] = 0; \n"
"#endif \n"
"\n"
"#ifdef CL_VERSION_1_1 \n"
"	cl_version_defined[1] = 1; \n"
"	cl_version[1] = CL_VERSION_1_1; \n"
"#else \n"
"	cl_version_defined[1] = 0; \n"
"#endif \n"
"\n"
"#ifdef CL_VERSION_1_2 \n"
"	cl_version_defined[2] = 1; \n"
"	cl_version[2] = CL_VERSION_1_2; \n"
"#else \n"
"	cl_version_defined[2] = 0; \n"
"#endif \n"
"\n"
"#ifdef CL_VERSION_2_0 \n"
"	cl_version_defined[3] = 1; \n"
"	cl_version[3] = CL_VERSION_2_0; \n"
"#else \n"
"	cl_version_defined[3] = 0; \n"
"#endif \n"
"\n"
"#ifdef __ENDIAN_LITTLE__ \n"
"	*endian_little_defined = 1; \n"
"	*endian_little = __ENDIAN_LITTLE__; \n"
"#else \n"
"	*endian_little_defined = 0; \n"
"#endif \n"
"#ifdef __IMAGE_SUPPORT__ \n"
"	*image_support_defined = 1; \n"
"	*image_support = __IMAGE_SUPPORT__; \n"
"#else \n"
"	*image_support_defined = 0; \n"
"#endif \n"
"}";

PIGLIT_CL_PROGRAM_TEST_CONFIG_BEGIN

	config.name = "Preprocessor Macros";
	config.clc_version_min = 10;
	config.run_per_device = true;

	config.program_source = program_source;
	config.kernel_name = "test";

PIGLIT_CL_PROGRAM_TEST_CONFIG_END

int
version_from_string(char* string, int *version)
{
	int major, minor;
	if (!string || sscanf(string, "%*[a-zA-Z ] %d.%d", &major, &minor) != 2) {
		return 0;
	}
	*version = 100 * major + 10 * minor;
	return 1;
}

#define NUM_CL_VERSION 4

enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_program_test_config* config,
               const struct piglit_cl_program_test_env* env)
{
	enum piglit_result result = PIGLIT_SKIP;
	enum piglit_result result_file_defined = PIGLIT_SKIP;
	enum piglit_result result_line_defined = PIGLIT_SKIP;
	enum piglit_result result_opencl_version_defined = PIGLIT_SKIP;
	enum piglit_result result_opencl_version = PIGLIT_SKIP;
	enum piglit_result result_opencl_c_version_defined = PIGLIT_SKIP;
	enum piglit_result result_opencl_c_version = PIGLIT_SKIP;
	enum piglit_result result_cl_version_defined[NUM_CL_VERSION];
	enum piglit_result result_cl_version[NUM_CL_VERSION];
	enum piglit_result result_endian_little_defined = PIGLIT_SKIP;
	enum piglit_result result_endian_little = PIGLIT_SKIP;
	enum piglit_result result_image_support_defined = PIGLIT_SKIP;
	enum piglit_result result_image_support = PIGLIT_SKIP;

	size_t work_size = 1;

	cl_int file_defined;
	cl_mem file_defined_mem = NULL;
	cl_int line_defined;
	cl_mem line_defined_mem = NULL;
	cl_int opencl_version_defined;
	cl_mem opencl_version_defined_mem = NULL;
	cl_int opencl_version;
	cl_mem opencl_version_mem = NULL;
	cl_int opencl_c_version_defined;
	cl_mem opencl_c_version_defined_mem = NULL;
	cl_int opencl_c_version;
	cl_mem opencl_c_version_mem = NULL;
	cl_int cl_version_defined[NUM_CL_VERSION];
	cl_mem cl_version_defined_mem = NULL;
	cl_int cl_version[NUM_CL_VERSION];
	cl_mem cl_version_mem = NULL;
	cl_int endian_little_defined;
	cl_mem endian_little_defined_mem = NULL;
	cl_int endian_little;
	cl_mem endian_little_mem = NULL;
	cl_int image_support_defined;
	cl_mem image_support_defined_mem = NULL;
	cl_int image_support;
	cl_mem image_support_mem = NULL;

	char* opencl_version_str_host = NULL;
	char* opencl_c_version_str_host = NULL;
	cl_bool* endian_little_host = NULL;
	cl_bool* image_support_host = NULL;

	int i;
	for (i = 0; i < NUM_CL_VERSION; ++i) {
		result_cl_version_defined[i] = PIGLIT_SKIP;
		result_cl_version[i] = PIGLIT_SKIP;
	}

	opencl_version_str_host = piglit_cl_get_device_info(env->device_id,
	                                                    CL_DEVICE_VERSION);
#ifdef CL_VERSION_1_1
	opencl_c_version_str_host = piglit_cl_get_device_info(env->device_id,
	                                                      CL_DEVICE_OPENCL_C_VERSION);
#endif
	endian_little_host = piglit_cl_get_device_info(env->device_id,
	                                               CL_DEVICE_ENDIAN_LITTLE);
	image_support_host = piglit_cl_get_device_info(env->device_id,
	                                               CL_DEVICE_IMAGE_SUPPORT);

	/* create buffers for the results */
	file_defined_mem = piglit_cl_create_buffer(env->context,
	                                           CL_MEM_WRITE_ONLY,
	                                           sizeof(cl_int));
	line_defined_mem = piglit_cl_create_buffer(env->context,
	                                           CL_MEM_WRITE_ONLY,
	                                           sizeof(cl_int));
	opencl_version_defined_mem = piglit_cl_create_buffer(env->context,
	                                                     CL_MEM_WRITE_ONLY,
	                                                     sizeof(cl_int));
	opencl_version_mem = piglit_cl_create_buffer(env->context,
	                                             CL_MEM_WRITE_ONLY,
	                                             sizeof(cl_int));
	opencl_c_version_defined_mem = piglit_cl_create_buffer(env->context,
	                                                       CL_MEM_WRITE_ONLY,
	                                                       sizeof(cl_int));
	opencl_c_version_mem = piglit_cl_create_buffer(env->context,
	                                               CL_MEM_WRITE_ONLY,
	                                               sizeof(cl_int));
	cl_version_defined_mem = piglit_cl_create_buffer(env->context,
	                                                 CL_MEM_WRITE_ONLY,
	                                                 NUM_CL_VERSION * sizeof(cl_int));
	cl_version_mem = piglit_cl_create_buffer(env->context,
	                                         CL_MEM_WRITE_ONLY,
	                                         NUM_CL_VERSION * sizeof(cl_int));
	endian_little_defined_mem = piglit_cl_create_buffer(env->context,
	                                                    CL_MEM_WRITE_ONLY,
	                                                    sizeof(cl_int));
	endian_little_mem = piglit_cl_create_buffer(env->context,
	                                            CL_MEM_WRITE_ONLY,
	                                            sizeof(cl_int));
	image_support_defined_mem = piglit_cl_create_buffer(env->context,
	                                                    CL_MEM_WRITE_ONLY,
	                                                    sizeof(cl_int));
	image_support_mem = piglit_cl_create_buffer(env->context,
	                                            CL_MEM_WRITE_ONLY,
	                                            sizeof(cl_int));

	/* set kernel args and run the kernel */
	piglit_cl_set_kernel_buffer_arg(env->kernel, 0, &file_defined_mem);
	piglit_cl_set_kernel_buffer_arg(env->kernel, 1, &line_defined_mem);
	piglit_cl_set_kernel_buffer_arg(env->kernel, 2,
	                                &opencl_version_defined_mem);
	piglit_cl_set_kernel_buffer_arg(env->kernel, 3, &opencl_version_mem);
	piglit_cl_set_kernel_buffer_arg(env->kernel, 4,
	                                &opencl_c_version_defined_mem);
	piglit_cl_set_kernel_buffer_arg(env->kernel, 5, &opencl_c_version_mem);
	piglit_cl_set_kernel_buffer_arg(env->kernel, 6,
	                                &cl_version_defined_mem);
	piglit_cl_set_kernel_buffer_arg(env->kernel, 7, &cl_version_mem);
	piglit_cl_set_kernel_buffer_arg(env->kernel, 8,
	                                &endian_little_defined_mem);
	piglit_cl_set_kernel_buffer_arg(env->kernel, 9, &endian_little_mem);
	piglit_cl_set_kernel_buffer_arg(env->kernel, 10,
	                                &image_support_defined_mem);
	piglit_cl_set_kernel_buffer_arg(env->kernel, 11, &image_support_mem);
	piglit_cl_execute_ND_range_kernel(env->context->command_queues[0],
	                                  env->kernel,
	                                  1,
	                                  NULL,
	                                  &work_size,
	                                  &work_size);

	/* read the buffers */
	piglit_cl_read_buffer(env->context->command_queues[0], file_defined_mem,
	                      0, sizeof(cl_int), &file_defined);
	piglit_cl_read_buffer(env->context->command_queues[0], line_defined_mem,
	                      0, sizeof(cl_int), &line_defined);
	piglit_cl_read_buffer(env->context->command_queues[0],
	                      opencl_version_defined_mem, 0, sizeof(cl_int),
	                      &opencl_version_defined);
	piglit_cl_read_buffer(env->context->command_queues[0],
	                      opencl_version_mem, 0, sizeof(cl_int),
	                      &opencl_version);
	piglit_cl_read_buffer(env->context->command_queues[0],
	                      opencl_c_version_defined_mem, 0, sizeof(cl_int),
	                      &opencl_c_version_defined);
	piglit_cl_read_buffer(env->context->command_queues[0],
	                      opencl_c_version_mem, 0, sizeof(cl_int),
	                      &opencl_c_version);
	piglit_cl_read_buffer(env->context->command_queues[0],
	                      cl_version_defined_mem, 0,
	                      NUM_CL_VERSION * sizeof(cl_int),
	                      cl_version_defined);
	piglit_cl_read_buffer(env->context->command_queues[0],
	                      cl_version_mem, 0,
	                      NUM_CL_VERSION * sizeof(cl_int),
	                      cl_version);
	piglit_cl_read_buffer(env->context->command_queues[0],
	                      endian_little_defined_mem, 0, sizeof(cl_int),
	                      &endian_little_defined);
	piglit_cl_read_buffer(env->context->command_queues[0],
	                      endian_little_mem, 0, sizeof(cl_int),
	                      &endian_little);
	piglit_cl_read_buffer(env->context->command_queues[0],
	                      image_support_defined_mem, 0, sizeof(cl_int),
	                      &image_support_defined);
	piglit_cl_read_buffer(env->context->command_queues[0],
	                      image_support_mem, 0, sizeof(cl_int),
	                      &image_support);

	/* check the values */
	result_file_defined = (file_defined == 1) ? PIGLIT_PASS : PIGLIT_FAIL;
	result_line_defined = (line_defined == 1) ? PIGLIT_PASS : PIGLIT_FAIL;

	if (!opencl_version_defined) {
		result_opencl_version_defined = PIGLIT_FAIL;
	} else {
		int opencl_version_host;
		result_opencl_version_defined = PIGLIT_PASS;
		if (!version_from_string(opencl_version_str_host,
		                         &opencl_version_host)) {
			printf("Could not determine host OpenCL version \"%s\".\n",
			       opencl_version_str_host);
			result_opencl_version = PIGLIT_FAIL;
		} else {
			result_opencl_version = (opencl_version == opencl_version_host) ?
			                        PIGLIT_PASS : PIGLIT_FAIL;
		}
	}

	if (env->version >= 11) {
		result_cl_version_defined[0] = cl_version_defined[0] ?
		                               PIGLIT_PASS : PIGLIT_FAIL;
		result_cl_version_defined[1] = cl_version_defined[1] ?
		                               PIGLIT_PASS : PIGLIT_FAIL;
	}
	if (env->version >= 12) {
		result_cl_version_defined[2] = cl_version_defined[2] ?
		                               PIGLIT_PASS : PIGLIT_FAIL;
	}
	if (env->version >= 20) {
		result_cl_version_defined[3] = cl_version_defined[3] ?
		                               PIGLIT_PASS : PIGLIT_FAIL;
	}

	if (env->version >= 12) {
		if (!opencl_c_version_defined) {
			result_opencl_c_version_defined = PIGLIT_FAIL;
		} else {
#ifdef CL_VERSION_1_1
			int opencl_c_version_host;
			result_opencl_c_version_defined = PIGLIT_PASS;
			if (!version_from_string(opencl_c_version_str_host,
			                         &opencl_c_version_host)) {
				printf("Could not determine host OpenCL C version \"%s\".\n",
				       opencl_c_version_str_host);
				result_opencl_c_version = PIGLIT_FAIL;
			} else {
				result_opencl_c_version = (opencl_c_version == opencl_c_version_host) ?
				                          PIGLIT_PASS : PIGLIT_FAIL;
			}
#endif
		}
	}

	if (cl_version_defined[0]) {
		result_cl_version[0] = (cl_version[0] == 100) ?
		                       PIGLIT_PASS : PIGLIT_FAIL;
	}
	if (cl_version_defined[1]) {
		result_cl_version[1] = (cl_version[1] == 110) ?
		                       PIGLIT_PASS : PIGLIT_FAIL;
	}
	if (cl_version_defined[2]) {
		result_cl_version[2] = (cl_version[2] == 120) ?
		                       PIGLIT_PASS : PIGLIT_FAIL;
	}
	if (cl_version_defined[3]) {
		result_cl_version[3] = (cl_version[3] == 200) ?
		                       PIGLIT_PASS : PIGLIT_FAIL;
	}

	result_endian_little_defined = (*endian_little_host == endian_little_defined) ?
	                               PIGLIT_PASS : PIGLIT_FAIL;
	if (endian_little_defined) {
		result_endian_little = (endian_little == 1) ?
		                       PIGLIT_PASS : PIGLIT_FAIL;
	} else if (!*endian_little_host) {
		result_endian_little = PIGLIT_PASS;
	}

	result_image_support_defined = (*image_support_host == image_support_defined) ?
	                               PIGLIT_PASS : PIGLIT_FAIL;
	if (image_support_defined) {
		result_image_support = (image_support == 1) ?
		                       PIGLIT_PASS : PIGLIT_FAIL;
	} else if (!*image_support_host) {
		result_image_support = PIGLIT_PASS;
	}

	/* report the results */
	piglit_report_subtest_result(result_file_defined,
	                             "__FILE__ must be defined");
	piglit_merge_result(&result, result_file_defined);
	piglit_report_subtest_result(result_line_defined,
	                             "__LINE__ must be defined");
	piglit_merge_result(&result, result_line_defined);
	piglit_report_subtest_result(result_opencl_version_defined,
	                             "__OPENCL_VERSION__ must be defined");
	piglit_merge_result(&result, result_opencl_version_defined);
	piglit_report_subtest_result(result_opencl_version,
	                             "__OPENCL_VERSION__ must be consistent with host");
	piglit_merge_result(&result, result_opencl_version);
	piglit_report_subtest_result(result_opencl_c_version_defined,
	                             "__OPENCL_C_VERSION__ must be defined for OpenCL 1.2 and later");
	piglit_merge_result(&result, result_opencl_c_version_defined);
	piglit_report_subtest_result(result_opencl_c_version,
	                             "__OPENCL_C_VERSION__ must be consistent with host (if defined)");
	piglit_merge_result(&result, result_opencl_c_version);
	piglit_report_subtest_result(result_cl_version_defined[0],
	                             "CL_VERSION_1_0 must be defined for OpenCL 1.1 and later");
	piglit_merge_result(&result, result_cl_version_defined[0]);
	piglit_report_subtest_result(result_cl_version_defined[1],
	                             "CL_VERSION_1_1 must be defined for OpenCL 1.1 and later");
	piglit_merge_result(&result, result_cl_version_defined[1]);
	piglit_report_subtest_result(result_cl_version_defined[2],
	                             "CL_VERSION_1_2 must be defined for OpenCL 1.2 and later");
	piglit_merge_result(&result, result_cl_version_defined[2]);
	piglit_report_subtest_result(result_cl_version_defined[3],
	                             "CL_VERSION_2_0 must be defined for OpenCL 2.0 and later");
	piglit_merge_result(&result, result_cl_version_defined[3]);
	piglit_report_subtest_result(result_cl_version[0],
	                             "CL_VERSION_1_0 must be 100 if defined");
	piglit_merge_result(&result, result_cl_version[0]);
	piglit_report_subtest_result(result_cl_version[1],
	                             "CL_VERSION_1_1 must be 110 if defined");
	piglit_merge_result(&result, result_cl_version[1]);
	piglit_report_subtest_result(result_cl_version[2],
	                             "CL_VERSION_1_2 must be 120 if defined");
	piglit_merge_result(&result, result_cl_version[2]);
	piglit_report_subtest_result(result_cl_version[3],
	                             "CL_VERSION_2_0 must be 200 if defined");
	piglit_merge_result(&result, result_cl_version[3]);
	piglit_report_subtest_result(result_endian_little_defined,
	                             "__ENDIAN_LITTLE__ must be consistent with host");
	piglit_merge_result(&result, result_endian_little_defined);
	piglit_report_subtest_result(result_endian_little,
	                             "__ENDIAN_LITTLE__ must be 1 if defined");
	piglit_merge_result(&result, result_endian_little);
	piglit_report_subtest_result(result_image_support_defined,
	                             "__IMAGE_SUPPORT__ must be consistent with host");
	piglit_merge_result(&result, result_image_support_defined);
	piglit_report_subtest_result(result_image_support,
	                             "__IMAGE_SUPPORT__ must be 1 if defined");
	piglit_merge_result(&result, result_image_support);

	/* free cl resources */
	clReleaseMemObject(file_defined_mem);
	clReleaseMemObject(line_defined_mem);
	clReleaseMemObject(opencl_version_defined_mem);
	clReleaseMemObject(opencl_version_mem);
	clReleaseMemObject(opencl_c_version_defined_mem);
	clReleaseMemObject(opencl_c_version_mem);
	clReleaseMemObject(cl_version_defined_mem);
	clReleaseMemObject(cl_version_mem);
	clReleaseMemObject(endian_little_defined_mem);
	clReleaseMemObject(endian_little_mem);
	clReleaseMemObject(image_support_defined_mem);
	clReleaseMemObject(image_support_mem);

	/* free host resources */
	free(opencl_version_str_host);
	free(opencl_c_version_str_host);
	free(endian_little_host);
	free(image_support_host);

	return result;
}
