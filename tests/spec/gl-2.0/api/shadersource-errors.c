/* Copyright © 2021 Valve Corporation
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

	config.supports_gl_compat_version = 20;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char v_shader[] =
	"varying vec4 color;\n"
	"void main() {\n"
	"   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"   color = vec4(0.0, 0.4, 0.0, 1.0);\n"
	"}\n";

static GLuint vs;
static GLuint prog;

static bool
test_api_errors(void)
{
	/* A count of zero is not considered an error by the spec */
	glShaderSourceARB(vs, 0, (const GLchar **)&v_shader, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	/* An INVALID_VALUE error is generated if count is negative. */
	glShaderSourceARB(vs, -1, (const GLchar **)&v_shader, NULL);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		return false;

	/* An INVALID_VALUE error is generated if shader is not the name of
	 * either a program or shader object.
	 */
	glShaderSourceARB(99, 1, (const GLchar **)&v_shader, NULL);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		return false;

	/* An INVALID_OPERATION error is generated if shader is the name of a
	 * program object.
	 */
	glShaderSourceARB(prog, 1, (const GLchar **)&v_shader, NULL);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		return false;

	return true;
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	bool pass;

	piglit_require_gl_version(20);

	vs = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	prog = glCreateProgramObjectARB();

	pass = test_api_errors();

	glDeleteShader(vs);
	glDeleteProgram(prog);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
