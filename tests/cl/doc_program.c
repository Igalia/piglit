#include "piglit-framework-cl-program.h"

/*
 * This is a file for program tests.
 * This type of test should be used for testing program building and
 * kernel execution.
 * Empty template is in template_program.c
 */

/* Function prototypes for config.init_func and config.clean_func */
void init(const int argc, const char** argv, void* config);
void clean(const int argc, const char** argv,void* config);

/* Test configuration */
PIGLIT_CL_PROGRAM_TEST_CONFIG_BEGIN

	/* Common */
	config.name = "Add and subtract";       // Name of the test
	config.run_per_platform = true;         // Run piglit_cl_test per each platform
	config.run_per_device   = true;         // Run piglit_cl_test per each device
	config.platform_regex = ".*Gallium.*";  // Only run on platforms that match this POSIX
	                                          // regex (if run_per_platform or run_per_device is true)
	config.device_regex = ".*RV300.*";      // Only run on device that match this POSIX
	                                          // regex (if run_per_device is true)
	config.require_platform_extensions = "ext1 ext2";  //Space-delimited required platform extensions
	config.require_device_extensions = "ext1 ext2";    //Space-delimited required device extensions
	init_func = init;                       // Function called before all the test calls
	clean_func = clean;                     // Function called after all the test calls

	/* Program */
	config.clc_version_min = 10;          // Minimum required OpenCL C version
	config.clc_version_max = 12;          // Maximum required OpenCL C version
	config.program_source = "kernel void test(){}";  // Create a program from string for each test
	config.program_source_file = "test.clc";         // Create a program from file for each test
	config.program_binary = "kernel void test(){}";  // Create a program from string for each test (binary)
	config.program_binary_file = "test.bin";         // Create a program from file for each test (binary)
	config.build_options = "-D DEF";      // Build options for the program
	config.expect_build_fail = true;      // Expect that build will fail
	config.kernel_name = "test";          // Create a kernel

PIGLIT_CL_PROGRAM_TEST_CONFIG_END


/* Test function */
enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_program_test_config* config,
               const struct piglit_cl_program_test_env* env) // look at piglit-framework-cl-program.h
{
	enum piglit_result result = PIGLIT_PASS;

	/* Code for test */

	return result;
}
