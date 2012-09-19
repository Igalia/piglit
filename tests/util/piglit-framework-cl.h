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
#ifndef PIGLIT_FRAMEWORK_CL_H
#define PIGLIT_FRAMEWORK_CL_H

#include "piglit-util-cl.h"


struct piglit_cl_test_config_header;

/**
 * \brief Get an empty test configuration with default values.
 *
 * Every test runner must have this function defined.
 */
typedef const void* piglit_cl_get_empty_test_config_t();

/**
 * \brief Run the test with selected config and environment.
 *
 * Every test runner must have this function defined.
 *
 * \note This function can be called multiple times.
 *
 * @param argc         Argument count passed to \c main().
 * @param argv         Argument vector passed to \c main().
 * @param config       Test configuration.
 * @param platform_id  OpenCL platform id. Valid if \c config->run_per_platform
 *                     is \c true.
 * @param device_id    OpenCL device id. Valid if \c config->run_per_device is
 *                     \c true.
 * @return             Result of test.
 */
typedef enum piglit_result piglit_cl_test_run_t(const int argc,
                                                const char** argv,
                                                void* config,
                                                int version,
                                                cl_platform_id platform_id,
                                                cl_device_id device_id);

/**
 * \brief Initialize test configuration.
 *
 * Type definition for a function that is passed to \c config.init_func. This
 * function is intended to be used to fill the \c config when there needs to
 * be some input processing.
 *
 * \note This function is called once before running test(s).
 *
 * @param argc         Argument count passed to \c main().
 * @param argv         Argument vector passed to \c main().
 * @param config       Test configuration.
 */
typedef void piglit_cl_test_init_t(const int argc,
                                   const char** argv,
                                   void* config);
/**
 * \brief Clean environment.
 *
 * Type definition for a function that is passed to \c config.clean_func. This
 * function is intended to be used to clean memory after finishing all tests.
 *
 * \note This function is called once after running test(s).
 *
 * @param argc         Argument count passed to \c main().
 * @param argv         Argument vector passed to \c main().
 * @param config       Test configuration.
 */
typedef void piglit_cl_test_clean_t(const int argc,
                                    const char** argv,
                                    void* config);


#define PIGLIT_CL_TEST_CONFIG_HEADER                                         \
        char* _filename; /**< Read-only test filename. (internal) */         \
        piglit_cl_test_run_t* _test_run;                                     \
          /**< Function pointer to run the test. (internal) */               \
                                                                             \
        char* name; /**< Name of test. (optional) */                         \
                                                                             \
        bool run_per_platform; /**< Run test per platform. (optional) */     \
        bool run_per_device; /**< Run test per device. (optional) */         \
                                                                             \
        char* platform_regex;                                                \
          /**< Regex to filter platforms (optional) */                       \
        char* device_regex;                                                  \
          /**< Regex to filter devices (optional) */                         \
                                                                             \
        char* require_platform_extensions;                                   \
          /**< Space-separated list of required platform extensions
               (optional) */                                                 \
        char* require_device_extensions;                                     \
          /**< Space-separated list of required device extensions
               (optional) */                                                 \
                                                                             \
        piglit_cl_test_init_t* init_func;                                    \
          /**< Function pointer to initialize environment. (optional) */     \
        piglit_cl_test_clean_t* clean_func;                                  \
          /**< Function pointer to clean environment. (optional) */

/**
 * \brief OpenCL test configuration header.
 *
 * Every Piglit OpenCL test configuration has this fields. They are
 * defined at the beginning of the test configuration struct.
 */
struct piglit_cl_test_config_header {
	PIGLIT_CL_TEST_CONFIG_HEADER
};

/**
 * \brief Get test configuration.
 *
 * Every test has this function defined.
 */
extern void*
piglit_cl_get_test_config(const int argc,
                          const char** argv,
                          const struct piglit_cl_test_config_header* config_header);

/**
 * \def PIGLIT_CL_TEST_CONFIG_BEGIN(test_config_struct_t)
 *
 * This macro must be used to create a test configuration instance
 * and must be followed by \c PIGLIT_CL_TEST_CONFIG_END macro.
 * It defines the first part of \c piglit_cl_get_test_config function
 * that is used to retrieve the test configuration, set default
 * values to it and implement main function.
 *
 * In beetween \c PIGLIT_CL_TEST_CONFIG_BEGIN and
 * \c PIGLIT_CL_TEST_CONFIG_END macros you can set values of fields
 * defined in \c test_config_struct_t type.
 *
 * \pre \c test_config_struct_t must be defined by
 * \c PIGLIT_CL_DEFINE_TEST_CONFIG_BEGIN and
 * \c PIGLIT_CL_DEFINE_TEST_CONFIG_END
 *
 * \pre \c get_empty_test_config_f must have type
 * \c piglit_cl_get_empty_test_config_t
 *
 * \pre \c test_run_f must have type
 * \c piglit_cl_test_run_t
 *
 */
/**
 * \def PIGLIT_CL_TEST_CONFIG_END
 *
 * This macro must be used to create a test configuration instance
 * and must follow \c PIGLIT_CL_TEST_CONFIG_BEGIN macro.
 * It defines the last part of \c piglit_cl_get_test_config function
 * that is used to retrieve the test configuration, set default
 * values to it and implement main function.
 */
#define PIGLIT_CL_TEST_CONFIG_BEGIN(test_config_struct_t,                 \
                                    get_empty_test_config_f,              \
                                    test_run_f)                           \
                                                                          \
        test_config_struct_t config;                                      \
                                                                          \
        void*                                                             \
        piglit_cl_get_test_config(                                        \
            const int argc,                                               \
            const char** argv,                                            \
            const struct piglit_cl_test_config_header* config_header_ptr) \
        {                                                                 \
            piglit_cl_get_empty_test_config_t* _get_empty_config =        \
                get_empty_test_config_f; /*for compile time check*/       \
                                                                          \
            memcpy(&config,                                               \
                   _get_empty_config(),                                   \
                   sizeof(test_config_struct_t));                         \
            memcpy(&config,                                               \
                   config_header_ptr,                                     \
                   sizeof(struct piglit_cl_test_config_header));          \
                                                                          \
            config._test_run = test_run_f;                                \
            config._filename = __FILE__;

            /* Here goes the configuration of the test */

#define PIGLIT_CL_TEST_CONFIG_END                                         \
                                                                          \
            return &config;                                               \
        }                                                                 \
                                                                          \
        int main(int argc, char** argv)                                   \
        {                                                                 \
            return piglit_cl_framework_run(argc, argv);                   \
        }

/**
 * \def PIGLIT_CL_DEFINE_TEST_CONFIG_BEGIN(test_config_struct_t)
 *
 * This macro must be used by each test configuration definition
 * and must be followed by \c PIGLIT_CL_DEFINE_TEST_CONFIG_END macro.
 * It defines the first part \c test_config_struct_t type with first bytes
 * set to \c piglit_cl_text_config_header.
 *
 * In beetween \c PIGLIT_CL_DEFINE_TEST_CONFIG_BEGIN and
 * \c PIGLIT_CL_DEFINE_TEST_CONFIG_END macros you can define additional
 * fields used by the tests.
 *
 * The name of test configuration type is manipulated by
 * \c test_config_struct_t macro argument.
 *
 */
/**
 * \def PIGLIT_CL_DEFINE_TEST_CONFIG_END
 *
 * This macro must be used by each test configuration definition
 * and must follow \c PIGLIT_CL_DEFINE_TEST_CONFIG_BEGIN macro.
 * It defines the last part of \c test_config_struct_t type.
 *
 */
#define PIGLIT_CL_DEFINE_TEST_CONFIG_BEGIN(test_config_struct_t)  \
                                                                  \
        test_config_struct_t {                                    \
            PIGLIT_CL_TEST_CONFIG_HEADER

            /* Here goes the test configuration definition */

#define PIGLIT_CL_DEFINE_TEST_CONFIG_END                          \
                                                                  \
        };


/**
 * \brief Called from \c main function of each test.
 *
 * Each test has a definition of a \c main function
 * in its configuration. The only thing that \c main does
 * is to call this function.
 *
 * @param argc  Argument count passed to \c main().
 * @param argv  Argument vector passed to \c main().
 */
int
piglit_cl_framework_run(int argc, char** argv);


/* Get command-line arguments */

/**
 * \brief Check if argument \c arg was passed to program.
 *
 * @param argc  Argument count passed to \c main().
 * @param argv  Argument vector passed to \c main().
 * @param arg   String of argument to check.
 * @return      \c true if argument was passed to program.
 */
bool
piglit_cl_is_arg_defined(int argc, const char** argv, const char* arg);

/**
 * \brief Get argument value passed to program.
 *
 * Get value passed after argument \c "-arg".
 *
 * @param argc         Argument count passed to \c main().
 * @param argv         Argument vector passed to \c main().
 * @param arg          Argument to retrieve.
 * @return             Returns value passed by argument or NULL.
 */
const char*
piglit_cl_get_arg_value(const int argc, const char** argv, const char* arg);

/**
 * \brief Get unnamed argument value passed to program.
 *
 * Get value index-th (zero based) unnamed argument passed to program.
 *
 * @param argc         Argument count passed to \c main().
 * @param argv         Argument vector passed to \c main().
 * @param index        Index of argument to retrieve
 * @return             Returns value passed by argument or NULL.
 */
const char*
piglit_cl_get_unnamed_arg(const int argc, const char** argv, int index);

/**
 * \brief Get version passed to program.
 *
 * Get value passed after argument "-version". If version
 * argument is not defined use PIGLIT_CL_VERSION environment
 * variable.
 *
 * @param argc  Argument count passed to \c main().
 * @param argv  Argument vector passed to \c main().
 * @return      Version passed to program.
 */
int
piglit_cl_get_version_arg(int argc, const char** argv);

/**
 * \brief Get platform id passed to program.
 *
 * Return a platform id with its name starting with value
 * of argument passed after argument "-platform". If platform
 * argument is not defined use PIGLIT_CL_PLATFORM environment
 * variable.
 *
 * @param argc         Argument count passed to \c main().
 * @param argv         Argument vector passed to \c main().
 * @param platform_id  Returns platform id.
 * @return             \c true if platform was passed to program.
 */
bool
piglit_cl_get_platform_arg(const int argc,
                           const char** argv,
                           cl_platform_id* platform_id);

/**
 * \brief Get device id passed to program.
 *
 * Return a device id with its name starting with value
 * of argument passed after argument "-device". If device
 * argument is not defined use PIGLIT_CL_DEVICE environment
 * variable.
 *
 * @param argc         Argument count passed to \c main().
 * @param argv         Argument vector passed to \c main().
 * @param platform_id  Platform on which the device should be searched.
 * @param device_id    Returns device id.
 * @return             \c true if device was passed to program.
 */
bool
piglit_cl_get_device_arg(const int argc,
                         const char** argv,
                         cl_platform_id platform_id,
                         cl_device_id* device_id);

#endif //PIGLIT_FRAMEWORK_CL_H
