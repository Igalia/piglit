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
 * GL_EXT_semaphore_fd extension.
 */

#include "piglit-util-gl.h"

static const struct piglit_gl_test_config * piglit_config;

static enum piglit_result
test_import_semaphore_fd_enum_errors(void * unused)
{
	GLuint sem;
	int fd = -1;

	glGenSemaphoresEXT(1, &sem);

	/**
	 * The spec does not define any errors for ImportSemaphoreFdEXT,
	 * but we should at least make sure this doesn't succeed.
	 */
	glImportSemaphoreFdEXT(sem, GL_NONE, fd);

	return piglit_check_gl_error(GL_INVALID_ENUM) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static const struct piglit_subtest tests[] = {
	{
		"import-semaphore-fd-bad-enum",
		"bad-enum",
		test_import_semaphore_fd_enum_errors,
		NULL,
	},
	{ NULL },

};

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

	result = piglit_run_selected_subtests(
		tests,
		piglit_config->selected_subtests,
		piglit_config->num_selected_subtests,
		result);

	return result;
}


void
piglit_init(int argc, char **argv)
{
	/* From the EXT_external_objects_fd spec:
	 *
	 *   "GL_EXT_semaphore_fd requires GL_EXT_semaphore"
	 */
	piglit_require_extension("GL_EXT_semaphore");
	piglit_require_extension("GL_EXT_semaphore_fd");
}
