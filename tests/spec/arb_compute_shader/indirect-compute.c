/*
 * Copyright (c) 2015 Intel Corporation
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
 * Tests indirect dispatch of a compute shader
 */

#include "cs-ids-common.h"

static struct piglit_gl_test_config *piglit_config;

PIGLIT_GL_TEST_CONFIG_BEGIN
	piglit_config = &config;
	config.supports_gl_compat_version = 33;
	config.supports_gl_core_version = 33;
PIGLIT_GL_TEST_CONFIG_END


enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}


static struct {
	uint32_t local[3];
	uint32_t global[3];
} scenarios[] = {
	{ { 2, 4, 8 }, { 8, 4, 2 } },
	{ { 4, 4, 4 }, { 4, 4, 4 } },
	{ { 8, 8, 8 }, { 8, 8, 8 } },
};


void
piglit_init(int argc, char **argv)
{
	enum piglit_result result = PIGLIT_PASS;
	int i;

	cs_ids_common_init();
	cs_ids_set_local_id_test();
	cs_ids_use_indirect_dispatch();

	for (i = 0; i < ARRAY_SIZE(scenarios); i++) {
		uint32_t *local = scenarios[i].local;
		uint32_t *global = scenarios[i].global;

		cs_ids_set_local_size(local[0], local[1], local[2]);
		cs_ids_set_global_size(global[0], global[1], global[2]);

		cs_ids_set_local_id_test();
		result = cs_ids_run_test();
		if (result != PIGLIT_PASS)
			piglit_report_result(result);

		cs_ids_set_global_id_test();
		result = cs_ids_run_test();
		if (result != PIGLIT_PASS)
			piglit_report_result(result);
	}

	cs_ids_common_destroy();

	piglit_report_result(result);
}
