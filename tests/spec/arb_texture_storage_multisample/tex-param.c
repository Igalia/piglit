/*
 * Copyright © 2013 Chris Forbes
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
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/* Exercises GetTexParameter/TexParameter with multisample textures */

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

struct subtest
{
	GLenum param;
	GLint initial_value;
	GLint value;
	GLenum expected_error;
	const char *label;          // if null, the enum name is used as the label
} subtests[] =
{
	/* readonly */
	{ GL_TEXTURE_IMMUTABLE_FORMAT, GL_FALSE, GL_TRUE, GL_INVALID_ENUM },

	/* sampler state from GL4.2 core spec, table 6.18 -- readonly, and generate
	 * INVALID_OPERATION
	 */
	{ GL_TEXTURE_MAG_FILTER, GL_NEAREST, GL_LINEAR, GL_INVALID_OPERATION },
	{ GL_TEXTURE_MIN_FILTER, GL_NEAREST, GL_LINEAR, GL_INVALID_OPERATION },
	{ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE, GL_REPEAT, GL_INVALID_OPERATION },
	{ GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE, GL_REPEAT, GL_INVALID_OPERATION },
	{ GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE, GL_REPEAT, GL_INVALID_OPERATION },
	{ GL_TEXTURE_COMPARE_MODE, GL_NONE, GL_COMPARE_REF_TO_TEXTURE, GL_INVALID_OPERATION },
	{ GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL, GL_ALWAYS, GL_INVALID_OPERATION },
	{ GL_TEXTURE_MIN_LOD, -1000, 0, GL_INVALID_OPERATION },
	{ GL_TEXTURE_MAX_LOD, 1000, 0, GL_INVALID_OPERATION },

	/* setting TEXTURE_BASE_LEVEL to a nonzero value produces INVALID_OPERATION;
	 * setting to a zero value is allowed
	 */
	{ GL_TEXTURE_BASE_LEVEL, 0, 0, GL_NO_ERROR, "GL_TEXTURE_BASE_LEVEL zero" },
	{ GL_TEXTURE_BASE_LEVEL, 0, 1, GL_INVALID_OPERATION, "GL_TEXTURE_BASE_LEVEL nonzero" },

	{ 0 } /* sentinel */
};

void
check_subtest(struct subtest *t)
{
	GLint val;
	const char *test_name = t->label ? t->label : piglit_get_gl_enum_name(t->param);
	GLint expected_val;

	glGetTexParameteriv(GL_TEXTURE_2D_MULTISAMPLE, t->param, &val);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("GetTexParameteriv failed\n");
		piglit_report_subtest_result(PIGLIT_FAIL, "%s", test_name);
		return;
	}

	if (t->initial_value != val) {
		printf("parameter %s expected initially %d, got %d\n",
		       piglit_get_gl_enum_name(t->param),
		       t->initial_value,
		       val);
		piglit_report_subtest_result(PIGLIT_FAIL, "%s", test_name);
		return;
	}

	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, t->param, t->value);

	if (!piglit_check_gl_error(t->expected_error)) {
		printf("error setting parameter %s\n",
		       piglit_get_gl_enum_name(t->param));
		piglit_report_subtest_result(PIGLIT_FAIL, "%s", test_name);
		return;
	}

	/* verify that the new value stuck (or didnt, if we expected failure) */
	glGetTexParameteriv(GL_TEXTURE_2D_MULTISAMPLE, t->param, &val);
	expected_val = t->expected_error == GL_NO_ERROR ? t->value : t->initial_value;

	if (expected_val != val) {
		printf("after setting parameter %s expected %d, got %d\n",
		       piglit_get_gl_enum_name(t->param),
		       expected_val,
		       val);
		piglit_report_subtest_result(PIGLIT_FAIL, "%s", test_name);
	}

	piglit_report_subtest_result(PIGLIT_PASS, "%s", test_name);
}

void
piglit_init(int argc, char **argv)
{
	GLuint tex;
	struct subtest *t;

	piglit_require_extension("GL_ARB_texture_storage_multisample");

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
				4, GL_RGBA, 64, 64, GL_TRUE);

	for (t = subtests; t->param; t++)
		check_subtest(t);

	piglit_report_result(PIGLIT_PASS);
}
