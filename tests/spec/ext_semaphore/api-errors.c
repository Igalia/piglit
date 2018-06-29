/*
 * Copyright (c) 2017 Valve Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation on
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS AND/OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * Tests that api errors are thrown where expected for the
 * GL_EXT_semaphore extension.
 */

#include "piglit-util-gl.h"

static const struct piglit_gl_test_config * piglit_config;

#define RESULT(error) piglit_check_gl_error(error) ? PIGLIT_PASS : PIGLIT_FAIL

static enum piglit_result
test_get_unsigned_byte_v_enum_errors(void * unused)
{
	GLubyte data[GL_UUID_SIZE_EXT];

	glGetUnsignedBytevEXT(UINT32_MAX, data);

	return RESULT(GL_INVALID_ENUM);
}

static enum piglit_result
test_get_unsigned_byte_i_v_enum_errors(void * unused)
{
	GLubyte data[GL_UUID_SIZE_EXT];

	glGetUnsignedBytei_vEXT(UINT32_MAX, 0, data);

	return RESULT(GL_INVALID_ENUM);
}

static enum piglit_result
test_get_unsigned_byte_i_v_value_errors(void * unused)
{
	GLubyte data[GL_UUID_SIZE_EXT];
	GLint numDevices;

	glGetIntegerv(GL_NUM_DEVICE_UUIDS_EXT, &numDevices);

	glGetUnsignedBytei_vEXT(GL_DEVICE_UUID_EXT, numDevices + 1, data);

	return RESULT(GL_INVALID_VALUE);
}

static enum piglit_result
test_gen_semaphores_value_errors(void * unused)
{
	GLuint sem;

	glGenSemaphoresEXT(-1, &sem);

	return RESULT(GL_INVALID_VALUE);
}

static enum piglit_result
test_delete_semaphores_value_errors(void * unused)
{
	GLuint sem;

	glDeleteSemaphoresEXT(-1, &sem);

	return RESULT(GL_INVALID_VALUE);
}

static enum piglit_result
test_semaphore_parameter_enum_errors(void * unused)
{
	GLuint sem;
	GLuint64 param;

	glGenSemaphoresEXT(1, &sem);

	/**
	 * The spec does not define any valid parameters
	 * in EXT_external_objects or in EXT_external_objects_fd
	 */
	glSemaphoreParameterui64vEXT(0, sem, &param);

	return RESULT(GL_INVALID_ENUM);
}

static enum piglit_result
test_get_semaphore_parameter_enum_errors(void * unused)
{
	GLuint sem;
	GLuint64 param;

	glGenSemaphoresEXT(1, &sem);
	glGetSemaphoreParameterui64vEXT(0, sem, &param);

	return RESULT(GL_INVALID_ENUM);
}

#undef RESULT

#define ADD_TEST(func, name) \
	{                    \
		name,        \
		name,        \
		func,        \
		NULL         \
	}
static const struct piglit_subtest tests[] = {
	ADD_TEST(test_get_unsigned_byte_v_enum_errors, "usigned-byte-v-bad-enum"),
	ADD_TEST(test_get_unsigned_byte_i_v_enum_errors, "usigned-byte-i-v-bad-enum"),
	ADD_TEST(test_get_unsigned_byte_i_v_value_errors, "usigned-byte-i-v-bad-value"),

	ADD_TEST(test_gen_semaphores_value_errors, "gen-semaphores-bad-value"),
	ADD_TEST(test_delete_semaphores_value_errors, "gen-semaphores-bad-value"),
	ADD_TEST(test_delete_semaphores_value_errors, "gen-semaphores-bad-value"),

	ADD_TEST(test_semaphore_parameter_enum_errors, "semaphore-parameter-bad-enum"),
	ADD_TEST(test_get_semaphore_parameter_enum_errors, "get-semaphore-parameter-bad-enum"),
	{ NULL },
};
#undef ADD_TEST

PIGLIT_GL_TEST_CONFIG_BEGIN

	piglit_config = &config;
	config.subtests = tests;
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	enum piglit_result result = PIGLIT_PASS;

	piglit_run_selected_subtests(
		tests,
		piglit_config->selected_subtests,
		piglit_config->num_selected_subtests,
		result);

	return result;
}


void
piglit_init(int argc, char **argv)
{
	/* From the EXT_external_objects spec:
	 *
	 *   "GL_EXT_semaphore requires OpenGL 1.0."
	 */
	piglit_require_extension("GL_EXT_semaphore");
}
