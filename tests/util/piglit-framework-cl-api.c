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

#include "piglit-framework-cl-api.h"

/* Default test configuration values */
const struct piglit_cl_api_test_config PIGLIT_CL_DEFAULT_API_TEST_CONFIG = {
	.version_min = 0,
	.version_max = 0,

	.create_context = false,

	.program_source = NULL,
	.build_options = NULL
};

/* Return default values for test configuration */
const void*
piglit_cl_get_empty_api_test_config()
{
	return &PIGLIT_CL_DEFAULT_API_TEST_CONFIG;
}

/* Check configuration */
void piglit_cl_api_test_init(const int argc,
                             const char** argv,
                             void* void_config)
{
	struct piglit_cl_api_test_config* config = void_config;

	/* Run test's init */
	if(config->_init_test != NULL) {
		config->_init_test(argc, argv, void_config);
	}

	// version_min
	if(config->version_min == 0) {
		config->version_min = 10;
	}
	if(config->version_min <= 0) {
		fprintf(stderr, "Invalid configuration, version_min is %d.\n",
		        config->version_min);
		piglit_report_result(PIGLIT_WARN);
	}
	if(config->version_min > PIGLIT_CL_VERSION) {
		fprintf(stderr, "Piglit was compiled with lower OpenCL version (%d.%d) than version_min: %d.\n",
		        PIGLIT_CL_VERSION/10, PIGLIT_CL_VERSION%10,
		        config->version_min);
		piglit_report_result(PIGLIT_WARN);
	}
	// version_max
	if(config->version_max < 0) {
		fprintf(stderr, "Invalid configuration, version_max is %d.\n",
		        config->version_max);
		piglit_report_result(PIGLIT_WARN);
	}
	if(config->version_max > 0 && config->version_max < config->version_min) {
		fprintf(stderr, "Invalid configuration, version_max (%d) is lower than version_min (%d).\n",
		        config->version_max, config->version_min);
		piglit_report_result(PIGLIT_WARN);
	}
	// create_context
	if(   config->create_context
	   && !(config->run_per_device || config->run_per_platform)) {
		printf("Invalid configuration, create_context can only be used with run_per_platform or run_per_device.\n");
		piglit_report_result(PIGLIT_WARN);
	}
	// program_source
	if(   config->program_source != NULL
	   && !(config->run_per_device || config->run_per_platform)) {
		printf("Invalid configuration, program_source can only be used with run_per_platform or run_per_device.\n");
		piglit_report_result(PIGLIT_WARN);
	}
	if(config->program_source != NULL && !config->create_context) {
		config->create_context = true;
	}
	// build options
	if(config->build_options != NULL && config->program_source == NULL) {
		fprintf(stderr, "Invalid configuration, build_options can only be used with program_source.\n");
		piglit_report_result(PIGLIT_WARN);
	}
}

/* Set environment and run test */
enum piglit_result
piglit_cl_api_test_run(const int argc,
                       const char** argv,
                       void* void_config,
                       int version,
                       cl_platform_id platform_id,
                       cl_device_id device_id)
{
	enum piglit_result result;

	struct piglit_cl_api_test_config* config = void_config;
	struct piglit_cl_api_test_env env;

	piglit_cl_context context = NULL;
	cl_program program = NULL;

	/* Check version to test against */
	if(version < config->version_min) {
		printf("Trying to run test with version (%d.%d) lower than version_min: %d\n",
		       version/10, version%10,
		       config->version_min);
		return PIGLIT_SKIP;
	}
	if(config->version_max > 0 && version > config->version_max) {
		/*
		 * If version was not provided on the command line
		 * lower it to version_max.
		 */
		if(piglit_cl_get_version_arg(argc, argv) == 0) {
			printf("#   Lowering version to %d.%d because of version_max.\n",
			       config->version_max/10, config->version_max%10);
			version = config->version_max;
		} else {
			printf("Trying to run test with version (%d.%d) higher than version_max: %d\n",
			       version/10, version%10,
			       config->version_max);
			return PIGLIT_SKIP;
		}
	}

	/* Create context */
	if(config->create_context) {
		if(config->run_per_platform) {
			unsigned int num_devices;
			cl_device_id* device_ids;

			num_devices = piglit_cl_get_device_ids(platform_id,
			                                       CL_DEVICE_TYPE_ALL,
			                                       &device_ids);

			context = piglit_cl_create_context(platform_id,
			                                   device_ids,
			                                   num_devices);

			free(device_ids);
		} else { // config->run_per_device
			context = piglit_cl_create_context(platform_id, &device_id, 1);
		}

		if(context == NULL) {
			return PIGLIT_FAIL;
		}
	}
	
	/* Create and build program */
	if(config->program_source != NULL) {
		if(config->build_options != NULL) {
			program = piglit_cl_build_program_with_source(context,
			                                              1,
			                                              &config->program_source,
			                                              config->build_options);
		} else {
			program = piglit_cl_build_program_with_source(context,
			                                              1,
			                                              &config->program_source,
			                                              "");
		}

		if(program == NULL) {
			return PIGLIT_FAIL;
		}
	}


	/* Set environment */
	env.platform_id = platform_id;
	env.device_id = device_id;
	env.context = context;
	env.version = version;
	env.program = program;


	/* Run the actual test */
	result = config->_api_test(argc, argv, config, &env);


	/* Release program */
	if(config->program_source != NULL) {
		clReleaseProgram(program);
	}
	
	/* Release context */
	if(config->create_context) {
		piglit_cl_release_context(context);
	}

	return result;
}
