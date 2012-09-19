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
#ifndef PIGLIT_FRAMEWORK_CL_PROGRAM_H
#define PIGLIT_FRAMEWORK_CL_PROGRAM_H

#include "piglit-framework-cl.h"


typedef const struct piglit_cl_program_test_config
                     piglit_cl_program_test_config_t;
typedef const struct piglit_cl_program_test_env
                     piglit_cl_program_test_env_t;

/**
 * \brief Definition of PROGRAM test function.
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
piglit_cl_program_test_t(const int argc,
                         const char** argv,
                         piglit_cl_program_test_config_t* config,
                         piglit_cl_program_test_env_t* env);

/**
 * \struct piglit_cl_program_test_config
 *
 * \brief Test configuration for PROGRAM tests.
 *
 * \note One of \c run_per_platform or \c run_per_device must be \c true.
 * \note One of \c program_* must be true.
 */
PIGLIT_CL_DEFINE_TEST_CONFIG_BEGIN(struct piglit_cl_program_test_config)

	piglit_cl_program_test_t* _program_test; /**< Program test function.
	                                              (internal) */
	piglit_cl_test_init_t* _init_test; /**< Program test init function.
	                                        (internal) */

	int clc_version_min; /**< Minimum OpenCL C version required. (optional) */
	int clc_version_max; /**< Maximum OpenCL C version supported. (optional) */

	char* program_source; /**< Source to create and build a program on each run.
	                           Conflicts with other \c program_*. (optional) */
	char* program_source_file; /**< Source file from which to read, create and
	                                build a program on each run. Conflicts with
	                                other \c program_*. (optional) */
	unsigned char* program_binary; /**< Binary to create and build a program on
	                                    each run. Conflicts with other
	                                    \c program_*. (optional) */
	char* program_binary_file; /**< Binary file from which to read, create and
	                                build a program on each run. Conflicts with
	                                other \c program_*. (optional) */

	char* build_options; /**< Build options for program. (optional) */
	bool expect_build_fail; /**< Expect building of a program to fail.
	                             (optional) */

	char* kernel_name; /**< Create kernel(s) for program.
	                        Conflicts with both \c expect_build_fail==TRUE and
	                        \c build_only==TRUE. (optional) */

PIGLIT_CL_DEFINE_TEST_CONFIG_END

piglit_cl_get_empty_test_config_t piglit_cl_get_empty_program_test_config;
piglit_cl_test_init_t piglit_cl_program_test_init;
piglit_cl_test_run_t piglit_cl_program_test_run;

/**
 * \def PIGLIT_CL_PROGRAM_TEST_CONFIG_BEGIN
 *
 * Extension of \c PIGLIT_CL_TEST_CONFIG_BEGIN macro to be used by
 * PROGRAM tests.
 * This macro must be used to create a PROGRAM test configuration
 * instance and must be followed by \c PIGLIT_CL_TEST_PROGRAM_CONFIG_END macro.
 *
 * In beetween \c PIGLIT_CL_PROGRAM_TEST_CONFIG_BEGIN and
 * \c PIGLIT_CL_PROGRAM_TEST_CONFIG_END macros you can set the test
 * configuration values.
 *
 */
/**
 * \def PIGLIT_CL_PROGRAM_TEST_CONFIG_END
 *
 * Extension of \c PIGLIT_CL_TEST_CONFIG_END macro to be used by
 * PROGRAM tests. It defines function prototypes for functions used by
 * a PROGRAM test.
 * This macro must be used to create a test configuration instance
 * and must follow \c PIGLIT_CL_PROGRAM_TEST_CONFIG_BEGIN macro.
 *
 */
#define PIGLIT_CL_PROGRAM_TEST_CONFIG_BEGIN                                  \
        piglit_cl_program_test_t piglit_cl_test;                             \
                                                                             \
        PIGLIT_CL_TEST_CONFIG_BEGIN(struct piglit_cl_program_test_config,    \
                                    piglit_cl_get_empty_program_test_config, \
                                    piglit_cl_program_test_run)

#define PIGLIT_CL_PROGRAM_TEST_CONFIG_END                                    \
        config._program_test = piglit_cl_test;                               \
        config._init_test = config.init_func;                                \
        config.init_func = piglit_cl_program_test_init;                      \
                                                                             \
        PIGLIT_CL_TEST_CONFIG_END

/**
* \brief Environment for PROGRAM tests.
*
* Defines environment used by PROGRAM tests.
*/
struct piglit_cl_program_test_env {
	int version; /**< Version of OpenCL to test against. */
	int clc_version; /**< Version of OpenCL C to test against. */

	cl_platform_id platform_id; /**< OpenCL platform id. */
	cl_device_id device_id; /**< OpenCL device id.
	                             Holds valid device id if \c run_per_device is
	                             \c true. */

	piglit_cl_context context; /**< Generated helper context. */

	cl_program program; /**< OpenCL program. */

	cl_kernel kernel; /**< OpenCL kernel.
	                       Holds valid kernel if \c kernel_name is a
	                       NULL-terminated string, \c run_per_device is \c true,
	                       and \c expect_build_fail is \c false.*/
};


#endif //PIGLIT_FRAMEWORK_CL_PROGRAM_H
