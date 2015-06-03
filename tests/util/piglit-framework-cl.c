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

#include <stdlib.h>
#include <regex.h>

#include "piglit-framework-cl.h"


/* Default test header configuration values */
const struct piglit_cl_test_config_header
             PIGLIT_CL_DEFAULT_TEST_CONFIG_HEADER = {
	._filename = "",
	.name = NULL,

	.run_per_platform = false,
	.run_per_device = false,

	.platform_regex = NULL,
	.device_regex = NULL,

	.require_platform_extensions = NULL,
	.require_device_extensions = NULL,

	.init_func = NULL,
	.clean_func = NULL,
};


/* Print test configuration */
static void
print_test_info(const struct piglit_cl_test_config_header* config,
                int version,
                const cl_platform_id platform_id,
                const cl_device_id device_id) {
	if(config->run_per_platform || config->run_per_device) {
		char* platform_name;

		platform_name = piglit_cl_get_platform_info(platform_id,
		                                            CL_PLATFORM_NAME);

		printf("# Running on:\n"
		       "#   Platform: %s\n",
		       platform_name);

		if(config->run_per_device) {
			char* device_name = piglit_cl_get_device_info(device_id,
			                                              CL_DEVICE_NAME);

			printf("#   Device: %s\n", device_name);

			free(device_name);
		}

		printf("#   OpenCL version: %d.%d\n", version/10, version%10);

		free(platform_name);
	} else {
		// print nothing
	}
}

/* Check extensions */

bool check_platform_extensions(cl_platform_id platform_id, char* extensions)
{
	char* pch;

	if (!extensions)
		return true;

	pch = strtok(extensions, " ");
	while(pch != NULL) {
		if(   strlen(pch) > 0
		   && !piglit_cl_is_platform_extension_supported(platform_id, pch)) {
			char* platform_name = piglit_cl_get_platform_info(platform_id,
			                                                  CL_PLATFORM_NAME);
			printf("\n# Skipping platform %s because extension %s is not supported.\n\n",
			       platform_name,
			       pch);
			free(platform_name);
			return false;
		}
		pch = strtok(NULL, " ");
	}

	return true;
}

bool check_device_extensions(cl_device_id device_id, char* extensions)
{
	char* pch;

	if (!extensions)
		return true;

	pch = strtok(extensions, " ");
	while(pch != NULL) {
		if(   strlen(pch) > 0
		   && !piglit_cl_is_device_extension_supported(device_id, pch)) {
			char* device_name = piglit_cl_get_device_info(device_id,
			                                              CL_DEVICE_NAME);
			printf("\n# Skipping device %s because extension %s is not supported.\n\n",
			       device_name,
			       pch);
			free(device_name);
			return false;
		}
		pch = strtok(NULL, " ");
	}

	return true;
}

/* Run the test(s) */
int piglit_cl_framework_run(int argc, char** argv)
{
	enum piglit_result result = PIGLIT_SKIP;

	int version = 0;
	cl_platform_id platform_id = NULL;
	cl_device_id device_id = NULL;

	/* Get test configuration */
	struct piglit_cl_test_config_header *config =
		piglit_cl_get_test_config(argc,
		                          (const char**)argv,
		                          &PIGLIT_CL_DEFAULT_TEST_CONFIG_HEADER);

	/* Check that config is valid */
	// run_per_platform, run_per_device
	if(config->run_per_platform && config->run_per_device) {
		fprintf(stderr,
		        "Invalid configuration, only one of run_per_platform and run_per_device can be true.\n");
		piglit_report_result(PIGLIT_WARN);
	}

	/* Init */
	if(config->init_func != NULL) {
		config->init_func(argc, (const char**)argv, config);
	}

	/* Print test name and file */
	printf("## Test: %s (%s) ##\n\n", config->name != NULL ? config->name : "",
	       config->_filename);

	/* Get version to test against */
	version = piglit_cl_get_version_arg(argc, (const char **)argv);
	if(version > 0) {
		if(version > PIGLIT_CL_VERSION) {
			printf("Piglit was compiled with lower OpenCL version (%d.%d) than version argument: %d.%d.\n",
			       PIGLIT_CL_VERSION/10, PIGLIT_CL_VERSION%10,
			       version/10, version%10);
			piglit_report_result(PIGLIT_SKIP);
		}
	} else {
		/*
		 * If version was not provided on the command line, set it to
		 * the version against which Piglit was compiled (PIGLIT_CL_VERSION)
		 */
		version = PIGLIT_CL_VERSION;
	}

	/* Run the actual test */
	if(!(config->run_per_platform || config->run_per_device)) {
		print_test_info(config, version, NULL, NULL);
		result = config->_test_run(argc, (const char**)argv, (void*)config,
		                           version, NULL, NULL);
	} else {
		/* Run tests per platform or device */
		int i;
		regex_t platform_regex;
		regex_t device_regex;

		bool platform_defined;
		unsigned int num_platforms;
		cl_platform_id* platform_ids;

		/* Create regexes */
		if(   config->platform_regex != NULL
		   && regcomp(&platform_regex, config->platform_regex, REG_EXTENDED | REG_NEWLINE)) {
			fprintf(stderr,
			        "Regex to filter platforms is invalid, ignoring it.\n");
			regcomp(&platform_regex, "", REG_EXTENDED | REG_NEWLINE);
			piglit_merge_result(&result, PIGLIT_WARN);
		}
		if(   config->device_regex != NULL
		   && regcomp(&device_regex, config->device_regex, REG_EXTENDED | REG_NEWLINE)) {
			fprintf(stderr,
			        "Regex to filter devices is invalid, ignoring it.\n");
			regcomp(&device_regex, "", REG_EXTENDED | REG_NEWLINE);
			piglit_merge_result(&result, PIGLIT_WARN);
		}

		/* check for command-line/environment platform */
		platform_defined = piglit_cl_get_platform_arg(argc, (const char**)argv,
		                                              &platform_id);

		/* generate platforms list */
		if(platform_defined) {
			/* use platform defined by command-line/environment */
			num_platforms = 1;
			platform_ids = malloc(sizeof(cl_platform_id));
			platform_ids[0] = platform_id;
		} else {
			/* use all available platforms */
			num_platforms = piglit_cl_get_platform_ids(&platform_ids);
		}

		/* execute test for each platform in platforms list */
		for(i = 0; i < num_platforms; i++) {
			int final_version = version;
			int platform_version;

			platform_id = platform_ids[i];

			/* Filter platform */
			if(config->platform_regex != NULL) {
				char* platform_name;

				platform_name = piglit_cl_get_platform_info(platform_id,
				                                            CL_PLATFORM_NAME);
				if(regexec(&platform_regex, platform_name, 0, NULL, 0)) {
					printf("\n# Skipping platform %s because it does not match platform_regex.\n\n",
					       platform_name);
					free(platform_name);
					continue;
				}
				free(platform_name);
			}

			/* Check platform extensions */
			if(!check_platform_extensions(platform_id, config->require_platform_extensions)) {
				continue;
			}

			/* Get platform version */
			platform_version = piglit_cl_get_platform_version(platform_id);

			if(config->run_per_platform) {
				/* Check platform version */
				if(platform_version < final_version) {
					printf("# Platform supporting only version %d.%d. Running test on that version.\n",
					       platform_version/10, platform_version%10);
					final_version = platform_version;
				}

				/* run test on platform */
				print_test_info(config, final_version, platform_id, NULL);
				piglit_merge_result(&result,
				                    config->_test_run(argc,
				                                      (const char**)argv,
				                                      (void*)config,
				                                      final_version,
				                                      platform_id,
				                                      NULL));
			} else { //config->run_per_device
				int j;

				bool device_defined;
				unsigned int num_devices;
				cl_device_id* device_ids;

				/* check for command-line/environment device */
				device_defined = piglit_cl_get_device_arg(argc,
				                                          (const char**)argv,
				                                          platform_id,
				                                          &device_id);

				/* generate devices list */
				if(device_defined) {
					/* use device defined by command-line/environment */
					num_devices = 1;
					device_ids = malloc(sizeof(cl_device_id));
					device_ids[0] = device_id;
				} else {
					/* use all available devices */
					num_devices = piglit_cl_get_device_ids(platform_id,
					                                       CL_DEVICE_TYPE_ALL,
					                                       &device_ids);
				}

				/* run tests per each device */
				for(j = 0; j < num_devices; j++) {
					int device_version;

					device_id = device_ids[j];

					/* Filter device */
					if(config->device_regex != NULL) {
						char* device_name;

						device_name = piglit_cl_get_device_info(device_id,
						                                        CL_DEVICE_NAME);
						if(regexec(&device_regex, device_name, 0, NULL, 0)) {
							printf("\n# Skipping device %s because it does not match device_regex.\n\n",
								   device_name);
							free(device_name);
							continue;
						}
						free(device_name);
					}

					/* Check device extensions */
					if(!check_device_extensions(device_id, config->require_device_extensions)) {
						continue;
					}

					/* Check platform version */
					if(platform_version < final_version) {
						printf("# Platform supporting only version %d.%d. Running test on that version.\n",
						       platform_version/10, platform_version%10);
						final_version = platform_version;
					}
					/* Check device version */
					device_version = piglit_cl_get_device_version(device_id);
					if(device_version < final_version) {
						printf("# Device supporting only version %d.%d. Running test on that version.\n",
						       device_version/10, device_version%10);
						final_version = device_version;
					}

					print_test_info(config, version, platform_id, device_id);
					piglit_merge_result(&result,
					                    config->_test_run(argc,
					                                      (const char**)argv,
					                                      (void*)config,
					                                      final_version,
					                                      platform_id,
					                                      device_id));
				}

				free(device_ids);
			}
		}

		if(config->platform_regex != NULL) {
			regfree(&platform_regex);
		}
		if(config->device_regex != NULL) {
			regfree(&device_regex);
		}

		free(platform_ids);
	}

	/* Clean */
	if(config->clean_func != NULL) {
		config->clean_func(argc, (const char**)argv, config);
	}

	/* Report merged result */
	printf("# Result:\n");
	piglit_report_result(result);

	/* UNREACHED */
	return 1;
}

/* Get command-line/environment variables */

const char*
piglit_cl_get_arg_value(const int argc, const char *argv[], const char* arg)
{
	int i;
	char* full_arg = calloc(strlen(arg) + 2, sizeof(char));
	full_arg = strcpy(full_arg, "-");
	full_arg = strcat(full_arg, arg);

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], full_arg)) {
			if ((i+1) >= argc) {
				fprintf(stderr,
				        "Argument error: %s requires a value\n",
				        full_arg);
				free(full_arg);
				piglit_report_result(PIGLIT_WARN);
			} else {
				free(full_arg);
				return argv[i+1];
			}
		}
	}

	free(full_arg);
	return NULL;
}

const char*
piglit_cl_get_unnamed_arg(const int argc, const char *argv[], int index)
{
	int i;
	int count = 0;

	for (i = 1; i < argc; i++) {
		if (strncmp(argv[i], "-", 1)) {
			count++;
			if((count - 1) == index) {
				return argv[i];
			}
		} else {
			i++;
		}
	}

	return NULL;
}

bool
piglit_cl_is_arg_defined(const int argc, const char *argv[], const char* arg)
{
	int i;
	char* full_arg = calloc(strlen(arg) + 2, sizeof(char));
	full_arg = strcpy(full_arg, "-");
	full_arg = strcat(full_arg, arg);

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], full_arg)) {
			free(full_arg);
			return true;
		}
	}

	free(full_arg);
	return false;
}

int
piglit_cl_get_version_arg(int argc, const char** argv)
{
	int version_major = 0;
	int version_minor = 0;

	const char* version_str;

	/* First check argument then environment */
	version_str = piglit_cl_get_arg_value(argc, argv, "version");
	if(version_str == NULL) {
		version_str = getenv("PIGLIT_CL_VERSION");
	}

	if(version_str != NULL) {
		if(sscanf(version_str, "%i.%i", &version_major, &version_minor) != 2) {
			version_major = 0;
			version_minor = 0;
		}
	}

	return version_major*10 + version_minor;
}

bool
piglit_cl_get_platform_arg(const int argc, const char** argv,
                           cl_platform_id* platform_id)
{
	int i;
	const char* arg_value;

	/* First check argument then environment */
	arg_value = piglit_cl_get_arg_value(argc, argv, "platform");
	if(arg_value == NULL) {
		arg_value = getenv("PIGLIT_CL_PLATFORM");
	}

	if(arg_value != NULL) {
		unsigned int num_platforms;
		cl_platform_id* platform_ids;

		num_platforms = piglit_cl_get_platform_ids(&platform_ids);

		for(i = 0; i < num_platforms; i++) {
			char* platform_name = piglit_cl_get_platform_info(platform_ids[i],
			                                                  CL_PLATFORM_NAME);

			if(!strncmp(arg_value, platform_name, strlen(arg_value))) {
				*platform_id = platform_ids[i];

				free(platform_ids);
				free(platform_name);
				return true;
			}

			free(platform_name);
		}

		free(platform_ids);
		fprintf(stderr,
		        "Could not find platform: %s\n",
		        arg_value);
		piglit_report_result(PIGLIT_WARN);
	}

	return false;
}

bool
piglit_cl_get_device_arg(const int argc, const char** argv,
                         cl_platform_id platform_id, cl_device_id* device_id)
{
	int i;
	const char* arg_value;

	/* First check argument then environment */
	arg_value = piglit_cl_get_arg_value(argc, argv, "device");
	if(arg_value == NULL) {
		arg_value = getenv("PIGLIT_CL_DEVICE");
	}

	if(arg_value != NULL) {
		unsigned int num_devices;
		cl_device_id* device_ids;

		num_devices = piglit_cl_get_device_ids(platform_id,
		                                       CL_DEVICE_TYPE_ALL,
		                                       &device_ids);

		for(i = 0; i < num_devices; i++) {
			char* device_name = piglit_cl_get_device_info(device_ids[i],
			                                              CL_DEVICE_NAME);

			if(!strncmp(arg_value, device_name, strlen(arg_value))) {
				*device_id = device_ids[i];

				free(device_ids);
				free(device_name);
				return true;
			}

			free(device_name);
		}

		free(device_ids);
		fprintf(stderr,
		        "Could not find device: %s\n",
		        arg_value);
		piglit_report_result(PIGLIT_WARN);
	}

	return false;
}

bool piglit_cl_framework_check_local_work_size(
	cl_device_id device_id,
	size_t *local_work_size)
{
	unsigned i;
	size_t workgroup_size = 1;
	size_t *max_workgroup_size = piglit_cl_get_device_info(device_id,
						CL_DEVICE_MAX_WORK_GROUP_SIZE);
	size_t *max_workitem_sizes = piglit_cl_get_device_info(device_id,
						CL_DEVICE_MAX_WORK_ITEM_SIZES);
	bool ret = true;

	if (!local_work_size) {
		goto out;
	}

	if (!max_workgroup_size || !max_workitem_sizes) {
		ret = false;
		goto out;
	}

	for (i = 0; i < 3; i++) {
		size_t local_size = local_work_size[i];
		if (local_size > max_workitem_sizes[i]) {
			ret = false;
			goto out;
		}
		if (local_size > 0) {
			workgroup_size *= local_size;
		}
	}

	if (workgroup_size > *max_workgroup_size) {
		ret = false;
	}
out:
	free(max_workgroup_size);
	free(max_workitem_sizes);
	return ret;
}
