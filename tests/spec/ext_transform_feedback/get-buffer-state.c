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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file get-buffer-state.c
 *
 * Test that "Get" functions can be used to query the state of
 * transform feedback buffers.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

#define XFB_BUFFER_SIZE 12

enum test_mode {
	MAIN,
	INDEXED,
};

static struct test_desc
{
	const char *name;
	enum test_mode mode;
	GLenum param;
} tests[] = {
	/* name              mode     param */
	{ "main_binding",    MAIN,    GL_TRANSFORM_FEEDBACK_BUFFER_BINDING },
	{ "indexed_binding", INDEXED, GL_TRANSFORM_FEEDBACK_BUFFER_BINDING },
	{ "buffer_start",    INDEXED, GL_TRANSFORM_FEEDBACK_BUFFER_START },
	{ "buffer_size",     INDEXED, GL_TRANSFORM_FEEDBACK_BUFFER_SIZE },
};

static GLboolean
check_integer(const struct test_desc *test, GLenum param,
	      const char *param_string, GLint expected)
{
	GLint get_result;

	if (test->mode == MAIN && test->param == param) {
		glGetIntegerv(param, &get_result);
		if (get_result != expected) {
			printf("%s == %i, expected %i\n", param_string,
			       get_result, expected);
			return GL_FALSE;
		}
	}
	return GL_TRUE;
}

#define CHECK_INTEGER(param, expected) \
	pass = check_integer(test, param, #param, expected) && pass

static GLboolean
check_indexed(const struct test_desc *test, GLenum param,
	      const char *param_string, GLuint index, GLint expected)
{
	GLint get_result;

	if (test->mode == INDEXED && test->param == param) {
		glGetIntegeri_v(param, index, &get_result);
		if (get_result != expected) {
			printf("%s[%u] == %i, expected %i\n", param_string,
			       index, get_result, expected);
			return GL_FALSE;
		}
	}
	return GL_TRUE;
}

#define CHECK_INDEXED(param, index, expected) \
	pass = check_indexed(test, param, #param, index, expected) && pass

static GLboolean
do_test(const struct test_desc *test)
{
	GLuint *bufs;
	GLboolean pass = GL_TRUE;
	GLint max_separate_attribs;
	float initial_xfb_buffer_contents[XFB_BUFFER_SIZE];
	int i;

	glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS,
		      &max_separate_attribs);
	printf("MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTIBS=%i\n",
	       max_separate_attribs);

	bufs = malloc(max_separate_attribs * sizeof(GLuint));
	glGenBuffers(max_separate_attribs, bufs);
	memset(initial_xfb_buffer_contents, 0,
	       sizeof(initial_xfb_buffer_contents));

	/* Main GL_TRANSFORM_FEEDBACK_BUFFER_BINDING should still be
	 * set to its default value.
	 */
	CHECK_INTEGER(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, 0);

	/* Set up buffers. */
	for (i = 0; i < max_separate_attribs; ++i) {
		printf("BindBuffer(TRANSFORM_FEEDBACK_BUFFER, %u)\n", bufs[i]);
		glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, bufs[i]);
		CHECK_INTEGER(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, bufs[i]);
		glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
			     sizeof(initial_xfb_buffer_contents),
			     initial_xfb_buffer_contents, GL_STREAM_READ);
	}

	/* Indexed bindings should still be set to their default values. */
	for (i = 0; i < max_separate_attribs; ++i) {
		CHECK_INDEXED(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, i, 0);
		CHECK_INDEXED(GL_TRANSFORM_FEEDBACK_BUFFER_START, i, 0);
		CHECK_INDEXED(GL_TRANSFORM_FEEDBACK_BUFFER_SIZE, i, 0);
	}

	/* Bind buffers, setting various offsets and sizes. */
	for (i = 0; i < max_separate_attribs; ++i) {
		int offset = 4 * (i % 4);
		int size = 4 * ((i % 3) + 1);
		printf("BindBufferRange(TRANSFORM_FEEDBACK_BUFFER, %i, %u, %i, %i)\n",
		       i, bufs[i], offset, size);
		glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, i,
				  bufs[i], offset, size);
		CHECK_INTEGER(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, bufs[i]);
	}

	/* Check indexed bindings. */
	for (i = 0; i < max_separate_attribs; ++i) {
		int offset = 4 * (i % 4);
		int size = 4 * ((i % 3) + 1);
		CHECK_INDEXED(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, i, bufs[i]);
		CHECK_INDEXED(GL_TRANSFORM_FEEDBACK_BUFFER_START, i, offset);
		CHECK_INDEXED(GL_TRANSFORM_FEEDBACK_BUFFER_SIZE, i, size);
	}

	return pass;
}

void
print_usage_and_exit(const char *prog_name)
{
	int i;

	printf("Usage: %s <test_name>\n"
	       "  where <test_name> is one of:\n", prog_name);
	for (i = 0; i < ARRAY_SIZE(tests); ++i)
		printf("    %s\n", tests[i].name);
	exit(0);
}

const struct test_desc *
find_matching_test(const char *prog_name, const char *test_name)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(tests); ++i) {
		if (strcmp(tests[i].name, test_name) == 0)
			return &tests[i];
	}
	print_usage_and_exit(prog_name);
	return NULL; /* won't actually be reached */
}

void
piglit_init(int argc, char **argv)
{
	const struct test_desc *test;

	/* Parse params. */
	if (argc != 2)
		print_usage_and_exit(argv[0]);
	test = find_matching_test(argv[0], argv[1]);

	piglit_require_GLSL();
	piglit_require_transform_feedback();

	piglit_report_result(do_test(test) ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
