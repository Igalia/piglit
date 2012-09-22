/*
 * Copyright © 2012 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <string.h>

#include "piglit-util-gl-common.h"
#include "piglit_gl_framework.h"

#ifdef PIGLIT_USE_WAFFLE
#	include "piglit_fbo_framework.h"
#	include "piglit_winsys_framework.h"
#else
#	include "piglit_glut_framework.h"
#endif

struct piglit_gl_framework*
piglit_gl_framework_factory(const struct piglit_gl_test_config *test_config)
{
#ifdef PIGLIT_USE_WAFFLE
	struct piglit_gl_framework *gl_fw = NULL;

	if (piglit_use_fbo) {
		gl_fw = piglit_fbo_framework_create(test_config);
	}

	if (gl_fw == NULL) {
		piglit_use_fbo = false;
		gl_fw = piglit_winsys_framework_factory(test_config);
	}

	return gl_fw;
#else
	return piglit_glut_framework_create(test_config);
#endif
}

bool
piglit_gl_framework_init(struct piglit_gl_framework *gl_fw,
                         const struct piglit_gl_test_config *test_config)
{
	memset(gl_fw, 0, sizeof(*gl_fw));
	gl_fw->test_config = test_config;
	return true;
}

void
piglit_gl_framework_teardown(struct piglit_gl_framework *gl_fw)
{
	return;
}
