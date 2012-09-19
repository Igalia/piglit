#include "piglit-framework-cl-api.h"

/*
 * This is a file for API tests.
 * This type of test should be used for testing the API functions.
 * For each test uncomment the appropriate options and delete the rest.
 * Empty template is in template_api.c
 */

/* Function prototypes for config.init_func and config.clean_func */
void init(const int argc, const char** argv, void* config);
void clean(const int argc, const char** argv,void* config);

/* Test configuration */
PIGLIT_CL_API_TEST_CONFIG_BEGIN

	/* Common */
	config.name = "clFunctionName";         // Name of the test
	config.run_per_platform = true;         // Run piglit_cl_test per each platform
	config.run_per_device   = true;         // Run piglit_cl_test per each device
	config.platform_regex = ".*Gallium.*";  // Only run on platforms that match this POSIX
	                                          // regex (if run_per_platform or run_per_device is true)
	config.device_regex = ".*RV300.*";      // Only run on device that match this POSIX
	                                          // regex (if run_per_device is true)
	config.require_platform_extensions = "ext1 ext2"  //Space-delimited required platform extensions
	config.require_device_extensions = "ext1 ext2"    //Space-delimited required device extensions
	init_func = init;                       // Function called before all the test calls
	clean_func = clean;                     // Function called after all the test calls

	/* API */
	config.version_min = 10;          // Minimum required OpenCL version
	config.version_max = 12;          // Maximum required OpenCL version
	config.create_context = true;     // Create struct piglit_cl_context for each test
	                                    // (if run_per_platform or run_per_device are defined)
	config.program_source = "kernel void test(){}";  // Create a program for each test
	                                                   // (if run_per_platform or run_per_device are true)
	config.build_options = "-D DEF";  // Build options for the program

PIGLIT_CL_API_TEST_CONFIG_END


/* Test function */
enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_api_test_config* config,
               const struct piglit_cl_api_test_env* env) // look at piglit-framework-cl-api.h
{
	enum piglit_result result = PIGLIT_PASS;

	/* Code for test */

	return result;
}
