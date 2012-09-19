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

#include "piglit-framework-cl-custom.h"

/* Default test configuration values */
const struct piglit_cl_custom_test_config
             PIGLIT_CL_DEFAULT_CUSTOM_TEST_CONFIG = {
};

/* Return default values for test configuration */
const void*
piglit_cl_get_empty_custom_test_config()
{
	return &PIGLIT_CL_DEFAULT_CUSTOM_TEST_CONFIG;
}

/* Set environment and run test */
enum piglit_result
piglit_cl_custom_test_run(const int argc,
                       const char** argv,
                       void* void_config,
                       int version,
                       cl_platform_id platform_id,
                       cl_device_id device_id)
{
	enum piglit_result result;

	struct piglit_cl_custom_test_config* config = void_config;
	struct piglit_cl_custom_test_env env;

	/* Set environment */
	env.platform_id = platform_id;
	env.device_id = device_id;
	env.version = version;

	/* Run the actual test */
	result = config->_custom_test(argc, argv, config, &env);

	return result;
}
