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

#pragma once
#ifndef PIGLIT_FRAMEWORK_CL_API_H
#define PIGLIT_FRAMEWORK_CL_API_H

#include "piglit-framework-cl.h"


typedef const struct piglit_cl_api_test_config piglit_cl_api_test_config_t;
typedef const struct piglit_cl_api_test_env piglit_cl_api_test_env_t;

/**
 * \brief Definition of API test function.
 *
 * Every test must implement this function.
 *
 * @param argc         Argument count passed to \c main().
 * @param argv         Argument vector passed to \c main().
 * @param config       Test configuration.
 * @param env          Test environment.
 * @return             Result of test.
*/
typedef enum piglit_result
        piglit_cl_api_test_t(const int argc,
                             const char** argv,
                             piglit_cl_api_test_config_t* config,
                             piglit_cl_api_test_env_t* env);

/**
 * \struct piglit_cl_api_test_config
 *
 * \brief Test configuration for API tests.
 */
PIGLIT_CL_DEFINE_TEST_CONFIG_BEGIN(struct piglit_cl_api_test_config)

	piglit_cl_api_test_t* _api_test; /**< API test function. (internal) */
	piglit_cl_test_init_t* _init_test; /**< API test init function.
	                                       (internal) */

	int version_min; /**< Minimum version required. (optional) */
	int version_max; /**< Maximum version supported. (optional) */

	bool create_context; /**<  Create helper context on each run. Depends on
	                           \c run_per_device or \c run_per_platform.
	                           (optional)*/

	char* program_source; /**< Source to create and build a program on each run.
	                           Depends on \c run_per_device or
	                           \c run_per_platform and conflicts
	                           \c create_context=FALSE. (optional)*/
	char* build_options; /**< Build options for program. Depends on
	                          \c program_source. (optional)*/

PIGLIT_CL_DEFINE_TEST_CONFIG_END

piglit_cl_get_empty_test_config_t piglit_cl_get_empty_api_test_config;
piglit_cl_test_init_t piglit_cl_api_test_init;
piglit_cl_test_run_t piglit_cl_api_test_run;

/**
 * \def PIGLIT_CL_API_TEST_CONFIG_BEGIN
 *
 * Extension of \c PIGLIT_CL_TEST_CONFIG_BEGIN macro to be used by
 * API tests.
 * This macro must be used to create an API test configuration
 * instance and must be followed by \c PIGLIT_CL_TEST_API_CONFIG_END macro.
 *
 * In beetween \c PIGLIT_CL_API_TEST_CONFIG_BEGIN and
 * \c PIGLIT_CL_API_TEST_CONFIG_END macros you can set the test
 * configuration values.
 *
 */
/**
 * \def PIGLIT_CL_API_TEST_CONFIG_END
 *
 * Extension of \c PIGLIT_CL_TEST_CONFIG_END macro to be used by
 * API tests. It defines function prototypes for functions used by
 * an API test.
 * This macro must be used to create a test configuration instance
 * and must follow \c PIGLIT_CL_API_TEST_CONFIG_BEGIN macro.
 *
 */
#define PIGLIT_CL_API_TEST_CONFIG_BEGIN                                  \
        piglit_cl_api_test_t piglit_cl_test;                             \
                                                                         \
        PIGLIT_CL_TEST_CONFIG_BEGIN(struct piglit_cl_api_test_config,    \
                                    piglit_cl_get_empty_api_test_config, \
                                    piglit_cl_api_test_run)

#define PIGLIT_CL_API_TEST_CONFIG_END                                    \
        config._api_test = piglit_cl_test;                               \
        config._init_test = config.init_func;                            \
        config.init_func = piglit_cl_api_test_init;                      \
                                                                         \
        PIGLIT_CL_TEST_CONFIG_END

/**
* \brief Environment for API tests.
*
* Defines environment used by API tests.
*/
struct piglit_cl_api_test_env {
	int version; /**< Version of OpenCL to test against.
	                  Holds valid version if \c run_per_platform
	                  or \c run_per_device is \c true. */
	cl_platform_id platform_id; /**< OpenCL platform id.
	                                 Holds valid platform id if
	                                 \c run_per_platform or \c run_per_device is
	                                 \c true. */
	cl_device_id device_id; /**< OpenCL device id.
	                             Holds valid device id if \c run_per_device is
	                             \c true. */

	piglit_cl_context context; /**< Generated helper context.
	                                It is generated only if \c create_context
	                                and one of \c run_per_device or
	                                \c run_per_platform is \c true. Or if
	                                \c program_source is defined.*/

	cl_program program; /**< OpenCL program.
	                         Holds valid program if \c program_source is a
	                         NULL-terminated string and one of \c run_per_device
	                         or \c run_per_platform is \c true.*/
};


#endif //PIGLIT_FRAMEWORK_CL_API_H
