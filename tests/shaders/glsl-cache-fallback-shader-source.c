/*
 * Copyright (c) 2017 Timothy Arceri
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
 */

/**
 * \file
 * Tests setting shader source on an already compiled shader. If we don't
 * compile the new source we need to make sure the old source being used if
 * Mesa's on-disk shader cache is forced to fallback and recompile the shader.
 */

#include <stdio.h>

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_one[] =
	"varying vec4 color;\n"
	"void main() {\n"
	"   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"   color = vec4(0.0, 0.4, 0.0, 1.0);\n"
	"}\n";

static const char vs_two[] =
	"varying vec4 color;\n"
	"void main() {\n"
	"   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"   color = vec4(0.4, 0.4, 0.0, 1.0);\n"
	"}\n";

static const char fs_one[] =
	"varying vec4 color;\n"
	"void main() {\n"
	"   gl_FragColor = color;\n"
	"}\n";

static const char fs_two[] =
	"varying vec4 color;\n"
	"void main() {\n"
	"   gl_FragColor = color + vec4(0.4, 0.0, 0.4, 0.0);\n"
	"}\n";

static const GLfloat expect_one_one[3] = { 0.0, 0.4, 0.0 };
static const GLfloat expect_one_two[3] = { 0.4, 0.4, 0.4 };
static const GLfloat expect_two_one[3] = { 0.4, 0.4, 0.0 };
static const GLfloat expect_two_two[3] = { 0.8, 0.4, 0.4 };

static GLuint vs;
static GLuint fs;
static GLuint program;


static void compile_shader(GLuint shader)
{
	GLint status;

	glCompileShaderARB(shader);

	glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &status);
	if (!status) {
		GLchar log[1000];
		GLsizei len;
		glGetInfoLogARB(shader, 1000, &len, log);
		fprintf(stderr, "Error: problem compiling shader: %s\n", log);
		piglit_report_result(PIGLIT_FAIL);
	}
}

static void link_and_use_program()
{
	GLint status;

	glLinkProgramARB(program);
	glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &status);
	if (!status) {
		GLchar log[1000];
		GLsizei len;
		glGetInfoLogARB(program, 1000, &len, log);
		fprintf(stderr, "Error: problem linking program: %s\n", log);
		piglit_report_result(PIGLIT_FAIL);
	}

	glUseProgramObjectARB(program);
}

static void compile_shaders()
{
	compile_shader(vs);
	compile_shader(fs);
}

static void setup_shaders(const char * vstext, const char * fstext)
{
	glShaderSourceARB(vs, 1, (const GLchar **)&vstext, NULL);
	glShaderSourceARB(fs, 1, (const GLchar **)&fstext, NULL);
}

enum piglit_result
piglit_display(void)
{
	enum piglit_result result = PIGLIT_PASS;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClear(GL_COLOR_BUFFER_BIT);

	setup_shaders(vs_one, fs_one);
	compile_shaders();
	link_and_use_program();
	piglit_draw_rect(0, 0, piglit_width / 2, piglit_height / 2);
	if (!piglit_probe_pixel_rgb(piglit_width / 4, piglit_height / 4, expect_one_one))
		result = PIGLIT_FAIL;

	setup_shaders(vs_two, fs_two);
	compile_shaders();
	link_and_use_program();
	piglit_draw_rect(piglit_width / 2, 0, piglit_width / 2, piglit_height / 2);
	if (!piglit_probe_pixel_rgb(3 * piglit_width / 4, piglit_height / 4, expect_two_two))
		result = PIGLIT_FAIL;

	/* We have now seen all the shaders so Mesa will skip compiling them
	 * in future. If we link with a combination it hasn't seen before it
	 * will be forced to fallback and compile them, which is what will
	 * happen in the following two tests.
	 */

	setup_shaders(vs_two, fs_one);
	compile_shaders();
	setup_shaders(vs_one, fs_two);
	link_and_use_program();
	piglit_draw_rect(0, piglit_height / 2, piglit_width / 2, piglit_height / 2);
	if (!piglit_probe_pixel_rgb(piglit_width / 4, 3 * piglit_height / 4, expect_two_one))
		result = PIGLIT_FAIL;

	compile_shaders();
	setup_shaders(vs_two, fs_two);
	setup_shaders(vs_two, fs_one);
	link_and_use_program();
	piglit_draw_rect(piglit_width / 2, piglit_height / 2, piglit_width / 2, piglit_height / 2);
	if (!piglit_probe_pixel_rgb(3 * piglit_width / 4, 3 * piglit_height / 4, expect_one_two))
		result = PIGLIT_FAIL;

	return result;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_GLSL();

	vs = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	fs = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	program = glCreateProgramObjectARB();
	glAttachObjectARB(program, vs);
	glAttachObjectARB(program, fs);
}
