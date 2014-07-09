/*
 * Copyright © 2011 Intel Corporation
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

/**
 * \file glsl-link-test.c
 * Try to link a set of shaders, check if the result matches the expectation
 *
 * \author Ian Romanick
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLint program;
	bool expect = false;
	bool result;
	unsigned i;
	const char *invalid_file = NULL;

	piglit_require_gl_version(20);

	piglit_require_GLSL();
	program = glCreateProgram();

	for (i = 1; i < argc; i++) {
		size_t len;
		GLint shader;
		GLenum target;

		if (strcmp(argv[i], "pass") == 0) {
			expect = true;
			break;
		}

		if (strcmp(argv[i], "fail") == 0) {
			expect = false;
			break;
		}

		/* Expect that the name is at least one character plus
		 * ".vert", ".geom", or ".frag"
		 */
		len = strlen(argv[i]);
		if (len < 6) {
			invalid_file = argv[i];
			break;
		}

		if (strcmp(argv[i] + len - 5, ".vert") == 0) {
			target = GL_VERTEX_SHADER;
		} else if (strcmp(argv[i] + len - 5, ".geom") == 0) {
			target = GL_GEOMETRY_SHADER;
			if (piglit_get_gl_version() < 32 && !piglit_is_extension_supported("GL_ARB_geometry_shader4")) {
				printf("Requires geometry shaders.\n");
				piglit_report_result(PIGLIT_SKIP);
			}
		} else if (strcmp(argv[i] + len - 5, ".frag") == 0) {
			target = GL_FRAGMENT_SHADER;
		} else {
			invalid_file = argv[i];
			break;
		}

		shader = piglit_compile_shader(target, argv[i]);
		glAttachShader(program, shader);
		glDeleteShader(shader);
	}

	/* The loop above will break when an option of either 'pass' or 'fail'
	 * is encountered.  If this happens at the last commandline argument,
	 * the loop counter will be (argc-1).  Any other value is an error.
	 */
	if (i != (argc - 1)) {
		fprintf(stderr, "Last command line option must be either "
			"\"pass\" or \"fail\".\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (invalid_file != NULL) {
		fprintf(stderr, "Invalid shader file name \"%s\".\n",
			invalid_file);
		piglit_report_result(PIGLIT_FAIL);
	}

	glLinkProgram(program);

	result = piglit_link_check_status_quiet(program);
	if (result != expect)
		fprintf(stderr,
			"Program should have %s linking, but "
			"it was (incorrectly) %s.\n",
			expect ? "succeeded" : "failed",
			expect ? "unsuccesful" : "succesful");

	piglit_report_result((result == expect) ? PIGLIT_PASS : PIGLIT_FAIL);
}

