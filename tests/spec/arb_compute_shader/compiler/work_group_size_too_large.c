/*
 * Copyright © 2014 Intel Corporation
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
 * Test that exceeding the implementation's size work group size
 * limits results in a compile error.
 *
 * From the ARB_compute_shader specification:
 *
 *     If the local size of the shader in any dimension is greater
 *     than the maximum size supported by the implementation for that
 *     dimension, a compile-time error results.
 *
 * It is not clear from the spec how the error should be reported if
 * the total size of the work group exceeds
 * MAX_COMPUTE_WORK_GROUP_INVOCATIONS, but it seems reasonable to
 * assume that this is reported at compile time as well.
 */

#include "piglit-util-gl.h"
#include <math.h>
#include <limits.h>

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 33;
	config.supports_gl_core_version = 33;

PIGLIT_GL_TEST_CONFIG_END


enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}


static const char *cs_template =
	"#version 330\n"
	"#extension GL_ARB_compute_shader: enable\n"
	"\n"
	"layout(local_size_x = %d, local_size_y = %d, local_size_z = %d) in;\n"
	"\n"
	"void main()\n"
	"{\n"
	"}\n";


static bool
test_work_group_size(GLint *size, bool expect_ok)
{
	char *shader_text;
	GLint shader;
	GLint ok;

	printf("Sizes %d, %d, %d should %s: ", size[0], size[1], size[2],
	       expect_ok ? "compile successfully" : "produce a compile error");

	asprintf(&shader_text, cs_template, size[0], size[1], size[2]);
	shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(shader, 1, (const GLchar **) &shader_text, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		/* Details of the error have already been printed. */
		printf("GL Error occurred.\n");
		return false;
	}
	if (ok)
		printf("Successful compile.\n");
	else
		printf("Compile error.\n");
	return ok == expect_ok;
}


void
piglit_init(int argc, char **argv)
{
	GLint max_dims[3];
	GLint size[3];
	GLint max_invocations;
	int dim, i;
	double max_dims_product;
	bool pass = true;

	piglit_require_extension("GL_ARB_compute_shader");

	max_dims_product = 1.0;
	for (dim = 0; dim < 3; dim++) {
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, dim,
				&max_dims[dim]);
		max_dims_product *= max_dims[dim];
	}

	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_invocations);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	for (dim = 0; dim < 3; dim++) {
		/* Constrain all dimensions except dim to be as small
		 * as possible.
		 */
		for (i = 0; i < 3; i++)
			if (i != dim)
				size[i] = 1;

		/* Subject to that constraint, make dim as large as
		 * the implementation allows.
		 */
		if (max_dims[dim] < max_invocations)
			size[dim] = max_dims[dim];
		else
			size[dim] = max_invocations;

		/* Test that this size is allowed. */
		pass = test_work_group_size(size, true) && pass;

		/* Increase dim by 1 and make sure that the resulting
		 * size is not allowed.
		 */
		if (size[dim] < INT_MAX) {
			size[dim]++;
			test_work_group_size(size, false);
		}
	}

	if (max_dims_product > (double) max_invocations) {
		/* Construct a size for which each dimension is in
		 * bounds but the product is greater than
		 * max_invocations.  We want to find a factor f we can
		 * multiply each of max_dims[] by so that the result
		 * has a product of max_invocations + 1.  That is, we
		 * need:
		 *
		 * product(f*max_dims[i]) == max_invocations + 1
		 *
		 * therefore:
		 *
		 * f^3 * product(max_dims[i]) == max_invocations + 1
		 */
		double f = pow((max_invocations + 1.0) / max_dims_product,
			       1.0/3.0);

		/* Now we multiply each dimension by f, rounding up so
		 * that rounding errors don't push us back into the
		 * allowed range.
		 */
		for (dim = 0; dim < 3; dim++)
			size[dim] = (int) ceil(max_dims[dim] * f);

		pass = test_work_group_size(size, false) && pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
