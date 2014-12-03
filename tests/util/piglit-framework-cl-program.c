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

#include "piglit-framework-cl-program.h"


/* Default test configuration values */
const struct piglit_cl_program_test_config
             PIGLIT_CL_DEFAULT_PROGRAM_TEST_CONFIG = {
	.clc_version_min = 0,
	.clc_version_max = 0,

	.program_source = NULL,
	.program_source_file = NULL,
	.program_binary = NULL,
	.program_binary_file = NULL,

	.build_options = NULL,
	.expect_build_fail = false,

	.kernel_name = NULL,
};

/* Return default values for test configuration */
const void*
piglit_cl_get_empty_program_test_config()
{
	return &PIGLIT_CL_DEFAULT_PROGRAM_TEST_CONFIG;
}

/* Check configuration */
void piglit_cl_program_test_init(const int argc,
                                 const char** argv,
                                 void* void_config)
{
	struct piglit_cl_program_test_config* config = void_config;

	/* Run test's init */
	if(config->_init_test != NULL) {
		config->_init_test(argc, argv, void_config);
	}

	/* Check that config is valid */
	// run_per_device, run_per_platform
	if(!(config->run_per_device || config->run_per_platform)) {
		fprintf(stderr, "Invalid configuration, neither run_per_device nor run_per_platform is set to true.\n");
		piglit_report_result(PIGLIT_WARN);
	}
	// clc_version_min
	if(config->clc_version_min == 0) {
		config->clc_version_min = 10;
	}
	if(config->clc_version_min <= 0) {
		fprintf(stderr, "Invalid configuration, clc_version_min is %d.\n",
		        config->clc_version_min);
		piglit_report_result(PIGLIT_WARN);
	}
	// clc_version_max
	if(config->clc_version_max < 0) {
		fprintf(stderr, "Invalid configuration, clc_version_max is %d.\n",
		        config->clc_version_max);
		piglit_report_result(PIGLIT_WARN);
	}
	if(   config->clc_version_max > 0
	   && config->clc_version_max < config->clc_version_min) {
		fprintf(stderr, "Invalid configuration, clc_version_max (%d) is lower than clc_version_min (%d).\n",
		        config->clc_version_max, config->clc_version_min);
		piglit_report_result(PIGLIT_WARN);
	}
	// program_*
	if(!( /* one must be different than NULL */
	     (   config->program_source        != NULL
	      || config->program_source_file   != NULL
	      || config->program_binary        != NULL
	      || config->program_binary_file   != NULL
	     )
	     && /* the other three must be NULL */
	     (   (   config->program_source_file == NULL
	          && config->program_binary      == NULL
	          && config->program_binary_file == NULL)
	      || (   config->program_source      == NULL
	          && config->program_binary      == NULL
	          && config->program_binary_file == NULL)
	      || (   config->program_source      == NULL
	          && config->program_source_file == NULL
	          && config->program_binary_file == NULL)
	      || (   config->program_source      == NULL
	          && config->program_source_file == NULL
	          && config->program_binary      == NULL)
	     )
	    )) {
		fprintf(stderr, "Invalid configuration, one and only one of program_* must be defined.\n");
		piglit_report_result(PIGLIT_WARN);
	}
	// build_fail and kernel_name
	if(config->expect_build_fail && config->kernel_name != NULL) {
		fprintf(stderr, "Invalid configuration, kernel_name cannot be defined when build_fail is true.\n");
		piglit_report_result(PIGLIT_WARN);
	}
}

/* Run by piglit_cl_framework_run() */
enum piglit_result
piglit_cl_program_test_run(const int argc,
                           const char** argv,
                           void* void_config,
                           int version,
                           cl_platform_id platform_id,
                           cl_device_id device_id)
{
	enum piglit_result result;

	struct piglit_cl_program_test_config* config = void_config;
	struct piglit_cl_program_test_env env = {
		.version = version,
		.clc_version = 0,

		.platform_id = NULL,
		.device_id = NULL,

		.context = NULL,

		.program = NULL,

		.kernel = NULL,
	};

	int i;
	char* build_options = malloc(1 * sizeof(char));
	unsigned int num_devices;
	cl_device_id* device_ids;

	build_options[0] = '\0';

	/* Set environment */
	env.platform_id = platform_id;
	env.device_id = device_id;
	env.version = version;

	/* Get device ids */
	if(config->run_per_platform) {
		num_devices = piglit_cl_get_device_ids(platform_id,
		                                       CL_DEVICE_TYPE_ALL,
		                                       &device_ids);
	}

	/* Check OpenCL C version to test against */
	if(config->run_per_platform) {
		for(i = 0; i < num_devices; i++) {
			int device_clc_version =
				piglit_cl_get_device_cl_c_version(device_ids[i]);

			if(device_clc_version < env.clc_version || env.clc_version == 0) {
				env.clc_version = device_clc_version;
			}
		}
	} else { // config->run_per_device
		env.clc_version = piglit_cl_get_device_cl_c_version(device_id);
	}
	if(env.clc_version > version) {
		printf("#   Lowering OpenCL C version to %d.%d because of OpenCL version.\n",
		       version/10, version%10);
		env.clc_version = version;
	}
	if(   config->clc_version_max > 0
	   && env.clc_version > config->clc_version_max) {
		printf("#   Lowering OpenCL C version to %d.%d because of clc_version_max.\n",
		       config->clc_version_max/10, config->clc_version_max%10);
		env.clc_version = config->clc_version_max;
	}
	if(env.clc_version < config->clc_version_min) {
		printf("Trying to run test with OpenCL C version (%d.%d) ""lower than clc_version_min: %d\n",
		       env.clc_version/10, env.clc_version%10,
		       config->clc_version_min);
		return PIGLIT_SKIP;
	}

	printf("#   OpenCL C version: %d.%d\n",
	       env.clc_version/10, env.clc_version%10);

	/* Create context */
	if(config->run_per_platform) {
		env.context = piglit_cl_create_context(platform_id, device_ids,
		                                       num_devices);
	} else { // config->run_per_device
		env.context = piglit_cl_create_context(platform_id, &device_id, 1);
	}

	if(env.context == NULL) {
		return PIGLIT_FAIL;
	}

	/* Set build options */
	if(config->build_options != NULL) {
		char* old = build_options;
		build_options = malloc((strlen(old) + strlen(config->build_options) + 1) * sizeof(char));
		strcpy(build_options, old);
		sprintf(build_options+strlen(old), "%s", config->build_options);
		free(old);
	}

	if(env.clc_version > 10) {
		//If -cl-std was already in config->build_options, use what the test requested
		if (!strstr(build_options, "-cl-std")){
			char* template = " -cl-std=CL%d.%d";
			char* old = build_options;
			build_options = malloc((strlen(old) + strlen(template) + 1) * sizeof(char));
			strcpy(build_options, old);
			sprintf(build_options+strlen(old), template, env.clc_version/10,
								     env.clc_version%10);
			free(old);
		}
	}

	printf("#   Build options: %s\n", build_options);

	/* Create and build program */
	if(config->program_source != NULL) {
		if(!config->expect_build_fail) {
			env.program = piglit_cl_build_program_with_source(env.context,
			                                                  1,
			                                                  &config->program_source,
			                                                  build_options);
		} else {
			env.program = piglit_cl_fail_build_program_with_source(env.context,
			                                                       1,
			                                                       &config->program_source,
			                                                       build_options);
		}
	} else if(config->program_source_file != NULL) {
		unsigned int size;
		char* program_source;

		program_source = piglit_load_text_file(config->program_source_file, &size);
		if(program_source != NULL && size > 0) {
			if(!config->expect_build_fail) {
				env.program = piglit_cl_build_program_with_source(env.context,
				                                                  1,
				                                                  &program_source,
				                                                  build_options);
			} else {
				env.program = piglit_cl_fail_build_program_with_source(env.context,
				                                                       1,
				                                                       &program_source,
				                                                       build_options);
			}
		} else {
			fprintf(stderr, "Program source file %s does not exists or is empty\n",
			        config->program_source_file);
			return PIGLIT_WARN;
		}
		free(program_source);
	} else if(config->program_binary != NULL) {
		size_t length = strlen((char*)config->program_binary);

		if(!config->expect_build_fail) {
			env.program = piglit_cl_build_program_with_binary(env.context,
			                                                  &length,
			                                                  &config->program_binary,
			                                                  build_options);
		} else {
			env.program = piglit_cl_fail_build_program_with_binary(env.context,
			                                                       &length,
			                                                       &config->program_binary,
			                                                       build_options);
		}
	} else if(config->program_binary_file != NULL) {
		unsigned int length;
		size_t* lengths = malloc(sizeof(size_t) * env.context->num_devices);
		unsigned char** program_binaries = malloc(sizeof(unsigned char**) * env.context->num_devices);

		((char**)program_binaries)[0] =
			piglit_load_text_file(config->program_binary_file, &length);
		lengths[0] = length;
		for(i = 1; i < env.context->num_devices; i++) {
			lengths[i] = lengths[0];
			program_binaries[i] = program_binaries[0];
		}

		if(((char**)program_binaries)[0] != NULL && length > 0) {
			if(!config->expect_build_fail) {
				env.program = piglit_cl_build_program_with_binary(env.context,
				                                                  lengths,
				                                                  program_binaries,
				                                                  build_options);
			} else {
				env.program = piglit_cl_fail_build_program_with_binary(env.context,
				                                                       lengths,
				                                                       program_binaries,
				                                                       build_options);
			}
		} else {
			fprintf(stderr, "Program binary file %s does not exists or is empty\n",
			        config->program_source_file);
			return PIGLIT_WARN;
		}

		free(program_binaries[0]);
		free(program_binaries);
		free(lengths);
	}

	free(build_options);

	if(env.program == NULL) {
		return PIGLIT_FAIL;
	}

	/* Create kernel(s) */
	if(config->kernel_name != NULL) {
		env.kernel = piglit_cl_create_kernel(env.program, config->kernel_name);

		if(env.kernel == NULL) {
			return PIGLIT_FAIL;
		}
	}

	/* Release retrieved device IDs */
	if(config->run_per_platform) {
		free(device_ids);
	}


	/* Run the actual test */
	result = config->_program_test(argc, argv, config, &env);


	/* Release kernel(s) */
	if(env.kernel != NULL) {
		clReleaseKernel(env.kernel);
	}

	/* Release program */
	clReleaseProgram(env.program);

	/* Release context */
	piglit_cl_release_context(env.context);

	return result;
}
