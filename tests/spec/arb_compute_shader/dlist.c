/*
 * Copyright (c) 2018 Timothy Arceri
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

/** \file
 *
 * Tests dispatch of a compute shader via display lists
 */

#include "cs-ids-common.h"

static struct piglit_gl_test_config *piglit_config;

PIGLIT_GL_TEST_CONFIG_BEGIN
	piglit_config = &config;
	config.supports_gl_compat_version = 33;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END


static struct {
	uint32_t local[3];
	uint32_t global[3];
} scenarios[] = {
	{ { 2, 4, 8 }, { 8, 4, 2 } },
	{ { 4, 4, 4 }, { 4, 4, 10 } },
};


void
piglit_init(int argc, char **argv)
{
	enum piglit_result result = PIGLIT_PASS;

	GLuint list = glGenLists(1);

	cs_ids_common_init();

	uint32_t *local = scenarios[0].local;
	uint32_t *global = scenarios[0].global;

	cs_ids_set_local_size(local[0], local[1], local[2]);
	cs_ids_set_global_size(global[0], global[1], global[2]);

	cs_ids_set_local_id_test();

	/* -----------------------------------------
	 * Test dispatch with display lists
	 * -----------------------------------------
	 */

	cs_ids_setup_atomics_for_test();

	glNewList(list, GL_COMPILE);
	cs_ids_run_test_without_check();
	glEndList();

	/* Confirm atomic counters were not updated while compiling
	 * the display list.
	 */
	result = cs_ids_confirm_initial_atomic_counters();
	if (result != PIGLIT_PASS) {
		printf("Compute dispatch shouldn't have been called at "
		       "display list compilation time\n");
		piglit_report_result(result);
	}

	glCallList(list);

	/* Confirm dispatch compute worked correctly */
	result = cs_ids_confirm_size();
	if (result != PIGLIT_PASS) {
		printf("Compute dispatch - unexpected results");
		piglit_report_result(result);
	}

	/* Reset atomic counters */
	cs_ids_setup_atomics_for_test();
	result = cs_ids_confirm_initial_atomic_counters();
	if (result != PIGLIT_PASS)
		piglit_report_result(result);

	glNewList(list, GL_COMPILE_AND_EXECUTE);
	cs_ids_run_test_without_check();
	glEndList();

	/* Confirm dispatch compute worked correctly */
	result = cs_ids_confirm_size();
	if (result != PIGLIT_PASS) {
		printf("Compute dispatch should have been called at "
		       "display list compilation time\n");
		piglit_report_result(result);
	}

	/* -----------------------------------------
	 * Test indirect dispatch with display lists
	 * -----------------------------------------
	 */
	cs_ids_use_indirect_dispatch();

	/* Reset atomic counters */
	cs_ids_setup_atomics_for_test();
	result = cs_ids_confirm_initial_atomic_counters();
	if (result != PIGLIT_PASS)
		piglit_report_result(result);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glNewList(list, GL_COMPILE);
	cs_ids_run_test_without_check();
	glEndList();

	if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
		printf("Failed to generate error when calling "
		       "glDispatchComputeIndirect() in display list.");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Confirm atomic counters were not updated while compiling
	 * the display list.
	 */
	result = cs_ids_confirm_initial_atomic_counters();
	if (result != PIGLIT_PASS) {
		printf("Indirect compute dispatch shouldn't have been called "
		       "at display list compilation time\n");
		piglit_report_result(result);
	}

	/* Reset atomic counters */
	cs_ids_setup_atomics_for_test();
	result = cs_ids_confirm_initial_atomic_counters();
	if (result != PIGLIT_PASS)
		piglit_report_result(result);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glNewList(list, GL_COMPILE_AND_EXECUTE);
	cs_ids_run_test_without_check();
	glEndList();

	if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
		printf("Failed to generate error when calling "
		       "glDispatchComputeIndirect() in display list.");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Confirm atomic counters were not updated while compiling
	 * the display list.
	 */
	result = cs_ids_confirm_initial_atomic_counters();
	if (result != PIGLIT_PASS) {
		printf("Indirect compute dispatch shouldn't have been called "
		       "at display list compilation time\n");
		piglit_report_result(result);
	}

	/* We are done start teardown */
	glDeleteLists(list, 1);
	cs_ids_common_destroy();

	piglit_report_result(result);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
