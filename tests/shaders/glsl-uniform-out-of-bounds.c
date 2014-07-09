/*
 * Copyright (c) 2009 Nicolai Hähnle
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
 *
 * Authors:
 *    Nicolai Hähnle <nhaehnle@gmail.com>
 *
 */

/**
 * \file
 * Test that out-of-bound writes to uniform locations are caught properly.
 */

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static void expect_error(GLenum expect, const char * where, ...)
{
	GLenum error = glGetError();
	if (error != expect) {
		va_list va;

		fprintf(stderr, "Expected OpenGL error 0x%04x, got 0x%04x\nat: ", expect, error);

		va_start(va, where);
		vfprintf(stderr, where, va);
		va_end(va);
		fprintf(stderr, "\n");

		piglit_report_result(PIGLIT_FAIL);
	}
}

static GLuint compile_shader(GLenum shaderType, const char * text)
{
	GLuint shader;
	GLint status;

	shader = glCreateShaderObjectARB(shaderType);
	glShaderSourceARB(shader, 1, (const GLchar **)&text, NULL);
	glCompileShaderARB(shader);

	glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &status);
	if (!status) {
		GLchar log[1000];
		GLsizei len;
		glGetInfoLogARB(shader, 1000, &len, log);
		fprintf(stderr, "Error: problem compiling shader: %s\n", log);
		piglit_report_result(PIGLIT_FAIL);
	}
	return shader;
}

static GLuint link_program(GLuint vs, GLuint fs)
{
	GLuint program;
	GLint status;

	program = glCreateProgramObjectARB();
	if (vs)
		glAttachObjectARB(program, vs);
	if (fs)
		glAttachObjectARB(program, fs);

	glLinkProgramARB(program);
	glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &status);
	if (!status) {
		GLchar log[1000];
		GLsizei len;
		glGetInfoLogARB(program, 1000, &len, log);
		fprintf(stderr, "Error: problem linking program: %s\n", log);
		piglit_report_result(PIGLIT_FAIL);
	}

	return program;
}

static const GLfloat lots_of_zeros[16*1024] = { 0.0f, };

static const char vs_vector_template[] =
"uniform %s a;\n"
"uniform %s b[4];\n"
"uniform %s c[4];\n"
"varying %s v;\n"
"void main() {\n"
"   v = a + b[3] + c[0] + c[1] + c[2] + c[3];\n"
"   gl_Position = vec4(0,0,0,1);\n"
"}\n";

static const char fs_vector_template[] =
"varying %s v;\n"
"void main() {\n"
"   gl_FragColor = vec4(v%s);\n"
"}\n";


static void test_vector(const char *glsl_type, const char * suffix,
		void (GLAPIENTRY *uniform)(GLint, GLsizei, const GLfloat*))
{
	char buffer[2*sizeof(vs_vector_template)];
	GLuint vs, fs;
	GLuint program;
	GLint loc_a, loc_b, loc_c;
	GLint loc_b2;

	snprintf(buffer, sizeof(buffer), vs_vector_template,
		glsl_type, glsl_type, glsl_type, glsl_type);
	vs = compile_shader(GL_VERTEX_SHADER_ARB, buffer);

	snprintf(buffer, sizeof(buffer), fs_vector_template,
		glsl_type, suffix);
	fs = compile_shader(GL_FRAGMENT_SHADER_ARB, buffer);

	program = link_program(vs, fs);

	glUseProgramObjectARB(program);
	loc_a = glGetUniformLocationARB(program, "a");
	loc_b = glGetUniformLocationARB(program, "b");
	loc_c = glGetUniformLocationARB(program, "c");
	loc_b2 = glGetUniformLocationARB(program, "b[2]");
	printf("locations: a: %i b: %i c: %i b[2]: %i\n", loc_a, loc_b, loc_c, loc_b);

	expect_error(GL_NO_ERROR, "Type %s: Sanity check", glsl_type);
	uniform(loc_a, 0, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 0 to a", glsl_type);
	uniform(loc_a, 1, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 1 to a", glsl_type);
	uniform(loc_a, 2, lots_of_zeros);
	expect_error(GL_INVALID_OPERATION, "Type %s: Write count = 2 to a", glsl_type);
	uniform(loc_a, 1024, lots_of_zeros);
	expect_error(GL_INVALID_OPERATION, "Type %s: Write count = 1024 to a", glsl_type);

	uniform(loc_b, 0, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 0 to b", glsl_type);
	uniform(loc_b, 1, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 1 to b", glsl_type);
	uniform(loc_b, 4, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 4 to b", glsl_type);

	/* Note: The following are out of bound for the array,
	 * but the spec situation is a bit unclear as to whether errors
	 * should be generated.
	 *
	 * Issue #32 of the ARB_shader_objects spec suggests errors
	 * should be generated when writing out-of-bounds on arrays,
	 * but this is not reflected in the OpenGL spec.
	 *
	 * The main point of these tests is to make sure the driver
	 * does not introduce memory errors by accessing internal arrays
	 * out of bounds.
	 */
	uniform(loc_b, 5, lots_of_zeros);
	(void) glGetError(); /* Eat generated error, if any */

	uniform(loc_c, 0, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 0 to c", glsl_type);
	uniform(loc_c, 1, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 1 to c", glsl_type);
	uniform(loc_c, 4, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 4 to c", glsl_type);

	/* Out of bounds; see comment above */
	uniform(loc_c, 5, lots_of_zeros);
	(void) glGetError(); /* Eat generated error, if any */

	uniform(loc_b2, 0, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 0 to b[2]", glsl_type);
	uniform(loc_b2, 2, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 2 to b[2]", glsl_type);

	/* Out of bounds; see comment above */
	uniform(loc_b2, 1024, lots_of_zeros);
	(void) glGetError(); /* Eat generated error, if any */

	glDeleteObjectARB(fs);
	glDeleteObjectARB(vs);
	glDeleteObjectARB(program);
}

static const char vs_matrix_template[] =
"uniform mat4 a;\n"
"uniform mat4 b[4];\n"
"uniform mat4 c[4];\n"
"varying vec4 v;\n"
"void main() {\n"
"   mat4 m = a + b[3] + c[0] + c[1] + c[2] + c[3];\n"
"   v = m * vec4(1.0, 1.0, 1.0, 1.0);\n"
"   gl_Position = vec4(0,0,0,1);\n"
"}\n";

static const char fs_matrix_template[] =
"varying vec4 v;\n"
"void main() {\n"
"   gl_FragColor = v;\n"
"}\n";


static void test_matrix(void)
{
	GLuint vs, fs;
	GLuint program;
	GLint loc_a, loc_b, loc_c;
	GLint loc_b2;
	const char * glsl_type = "mat4";

	vs = compile_shader(GL_VERTEX_SHADER_ARB, vs_matrix_template);
	fs = compile_shader(GL_FRAGMENT_SHADER_ARB, fs_matrix_template);
	program = link_program(vs, fs);

	glUseProgramObjectARB(program);
	loc_a = glGetUniformLocationARB(program, "a");
	loc_b = glGetUniformLocationARB(program, "b");
	loc_c = glGetUniformLocationARB(program, "c");
	loc_b2 = glGetUniformLocationARB(program, "b[2]");
	printf("locations: a: %i b: %i c: %i b[2]: %i\n", loc_a, loc_b, loc_c, loc_b);

	expect_error(GL_NO_ERROR, "Type %s: Sanity check", glsl_type);

	glUniformMatrix4fvARB(loc_b, 0, GL_FALSE, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 0 to b", glsl_type);
	glUniformMatrix4fvARB(loc_b, 1, GL_FALSE, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 1 to b", glsl_type);
	glUniformMatrix4fvARB(loc_b, 4, GL_FALSE, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 4 to b", glsl_type);

	/* Out of bounds; see comment above */
	glUniformMatrix4fvARB(loc_b, 5, GL_FALSE, lots_of_zeros);
	(void) glGetError(); /* Eat generated error, if any */

	glUniformMatrix4fvARB(loc_c, 0, GL_FALSE, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 0 to c", glsl_type);
	glUniformMatrix4fvARB(loc_c, 1, GL_FALSE, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 1 to c", glsl_type);
	glUniformMatrix4fvARB(loc_c, 4, GL_FALSE, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 4 to c", glsl_type);

	/* Out of bounds; see comment above */
	glUniformMatrix4fvARB(loc_c, 5, GL_FALSE, lots_of_zeros);
	(void) glGetError(); /* Eat generated error, if any */

	glUniformMatrix4fvARB(loc_b2, 0, GL_FALSE, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 0 to b[2]", glsl_type);
	glUniformMatrix4fvARB(loc_b2, 2, GL_FALSE, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 2 to b[2]", glsl_type);

	/* Out of bounds; see comment above */
	glUniformMatrix4fvARB(loc_b2, INT_MAX, GL_FALSE, lots_of_zeros);
	(void) glGetError(); /* Eat generated error, if any */

	glUniformMatrix4fvARB(loc_a, 0, GL_FALSE, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 0 to a", glsl_type);
	glUniformMatrix4fvARB(loc_a, 1, GL_FALSE, lots_of_zeros);
	expect_error(GL_NO_ERROR, "Type %s: Write count = 1 to a", glsl_type);
	glUniformMatrix4fvARB(loc_a, 2, GL_FALSE, lots_of_zeros);
	expect_error(GL_INVALID_OPERATION, "Type %s: Write count = 2 to a", glsl_type);
	glUniformMatrix4fvARB(loc_a, INT_MAX, GL_FALSE, lots_of_zeros);
	expect_error(GL_INVALID_OPERATION, "Type %s: Write count = INT_MAX to a", glsl_type);

	glDeleteObjectARB(fs);
	glDeleteObjectARB(vs);
	glDeleteObjectARB(program);
}

enum piglit_result
piglit_display(void)
{
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	test_matrix();

	test_vector("float", ", 0, 0, 0", glUniform1fvARB);
	test_vector("vec2", ", 0, 0", glUniform2fvARB);
	test_vector("vec3", ", 0", glUniform3fvARB);
	test_vector("vec4", "", glUniform4fvARB);

	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	if (!piglit_is_extension_supported("GL_ARB_shader_objects") || !piglit_is_extension_supported("GL_ARB_vertex_shader") || !piglit_is_extension_supported("GL_ARB_fragment_shader")) {
		printf("Requires ARB_shader_objects and ARB_{vertex,fragment}_shader\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}
