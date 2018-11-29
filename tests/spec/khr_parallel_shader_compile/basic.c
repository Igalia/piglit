/*
 * Copyright © 2018 Advanced Micro Devices, Inc.
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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_es_version = 30;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static void
create_program(float f)
{
	char vscode[4096], fscode[4096];

	snprintf(vscode, sizeof(vscode),
		 "void main() { gl_Position = vec4(%f); }", f);
	snprintf(fscode, sizeof(fscode),
		 "void main() { gl_FragColor = vec4(%f); }", f);

	piglit_build_simple_program(vscode, fscode);
}

static void
check_max_shader_compiler_threads(unsigned expected)
{
	GLint threads;
	glGetIntegerv(GL_MAX_SHADER_COMPILER_THREADS_KHR, &threads);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	if (threads != expected)
		piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_KHR_parallel_shader_compile");

	/* Test the query. */
	check_max_shader_compiler_threads(0xffffffff);

	/* Test the initial compilation completion status. */
	GLint status;
	GLuint shader = glCreateShader(GL_VERTEX_SHADER);
	glGetShaderiv(shader, GL_COMPLETION_STATUS_KHR, &status);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	if (status != GL_TRUE) {
		puts("glGetShaderiv incorrect initial completion status");
		piglit_report_result(PIGLIT_FAIL);
	}

	GLuint program = glCreateProgram();
	glGetProgramiv(program, GL_COMPLETION_STATUS_KHR, &status);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	if (status != GL_TRUE) {
		puts("glGetProgramiv incorrect initial completion status");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Change the thread count to test that the driver doesn't crash.
	 * Drivers are not required to obey this.
	 */
	unsigned counter = 0;
	for (unsigned i = 0; i < 40; i++)
		create_program(counter++);
	glMaxShaderCompilerThreadsKHR(1);
	check_max_shader_compiler_threads(1);

	for (unsigned i = 0; i < 40; i++)
		create_program(counter++);
	glMaxShaderCompilerThreadsKHR(20);
	check_max_shader_compiler_threads(20);

	for (unsigned i = 0; i < 40; i++)
		create_program(counter++);
	glMaxShaderCompilerThreadsKHR(2);
	check_max_shader_compiler_threads(2);

	piglit_report_result(PIGLIT_PASS);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
