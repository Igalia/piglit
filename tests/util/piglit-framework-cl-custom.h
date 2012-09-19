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
#ifndef PIGLIT_FRAMEWORK_CL_CUSTOM_H
#define PIGLIT_FRAMEWORK_CL_CUSTOM_H

#include "piglit-framework-cl.h"


typedef const struct piglit_cl_custom_test_config
                     piglit_cl_custom_test_config_t;
typedef const struct piglit_cl_custom_test_env
                     piglit_cl_custom_test_env_t;

/**
 * \brief Definition of CUSTOM test function.
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
        piglit_cl_custom_test_t(const int argc,
                                const char** argv,
                                piglit_cl_custom_test_config_t* config,
                                piglit_cl_custom_test_env_t* env);

/**
 * \struct piglit_cl_custom_test_config
 *
 * \brief Test configuration for CUSTOM tests.
 */
PIGLIT_CL_DEFINE_TEST_CONFIG_BEGIN(struct piglit_cl_custom_test_config)

	piglit_cl_custom_test_t* _custom_test; /**< CUSTOM test function.
	                                            (internal) */

PIGLIT_CL_DEFINE_TEST_CONFIG_END

piglit_cl_get_empty_test_config_t piglit_cl_get_empty_custom_test_config;
piglit_cl_test_run_t piglit_cl_custom_test_run;

/**
 * \def PIGLIT_CL_CUSTOM_TEST_CONFIG_BEGIN
 *
 * Extension of \c PIGLIT_CL_TEST_CONFIG_BEGIN macro to be used by
 * CUSTOM tests.
 * This macro must be used to create an CUSTOM test configuration
 * instance and must be followed by \c PIGLIT_CL_TEST_CUSTOM_CONFIG_END macro.
 *
 * In beetween \c PIGLIT_CL_CUSTOM_TEST_CONFIG_BEGIN and
 * \c PIGLIT_CL_CUSTOM_TEST_CONFIG_END macros you can set the test
 * configuration values.
 *
 */
/**
 * \def PIGLIT_CL_CUSTOM_TEST_CONFIG_END
 *
 * Extension of \c PIGLIT_CL_TEST_CONFIG_END macro to be used by
 * CUSTOM tests. It defines function prototypes for functions used by
 * an CUSTOM test.
 * This macro must be used to create a test configuration instance
 * and must follow \c PIGLIT_CL_CUSTOM_TEST_CONFIG_BEGIN macro.
 *
 */
#define PIGLIT_CL_CUSTOM_TEST_CONFIG_BEGIN                                  \
        piglit_cl_custom_test_t piglit_cl_test;                             \
                                                                            \
        PIGLIT_CL_TEST_CONFIG_BEGIN(struct piglit_cl_custom_test_config,    \
                                    piglit_cl_get_empty_custom_test_config, \
                                    piglit_cl_custom_test_run)

#define PIGLIT_CL_CUSTOM_TEST_CONFIG_END                                    \
        config._custom_test = piglit_cl_test;                               \
                                                                            \
        PIGLIT_CL_TEST_CONFIG_END

/**
* \brief Environment for CUSTOM tests.
*
* Defines environment used by CUSTOM tests.
*/
struct piglit_cl_custom_test_env {
	int version; /**< Version of OpenCL to test against.
	                  Holds valid version if \c run_per_platform
	                  or \c run_per_device is \c true. */
	cl_platform_id platform_id; /**< OpenCL platform id.
	                                 Holds valid platform id if
	                                 \c run_per_platform or \c run_per_device
	                                 is \c true. */
	cl_device_id device_id; /**< OpenCL device id.
	                             Holds valid device id if \c run_per_device is
	                             \c true. */
};


#endif //PIGLIT_FRAMEWORK_CL_CUSTOM_H
