/*
 * Copyright 2017 VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL AUTHORS AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include "piglit-util-gl.h"
#include "minmax-test.h"


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_core_version = 43;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END


void
piglit_init(int argc, char *argv[])
{
	GLint num = -1;
	GLint i;

	glGetIntegerv(GL_NUM_SHADING_LANGUAGE_VERSIONS, &num);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}
	if (num < 1) {
		printf("Invalid number of shading language versions\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	for (i = 0; i < num; i++) {
		const GLubyte *v = glGetStringi(GL_SHADING_LANGUAGE_VERSION, i);
		if (!v) {
			printf("Invalid glGetStringi(index=%d) result\n", i);
			piglit_report_result(PIGLIT_FAIL);
		}
		if (v[0] != 0 && !(v[0] >= '1' && v[0] <= '9')) {
			printf("Invalid GLSL version string: %s\n", v);
			piglit_report_result(PIGLIT_FAIL);
		}
		//printf("%d: %s\n", i, (const char *) v);
	}

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}


enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}
