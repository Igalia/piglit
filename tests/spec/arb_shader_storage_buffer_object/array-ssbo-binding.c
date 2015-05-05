/*
 * Copyright © 2015 Intel Corporation
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

/** @file array-ssbo-binding.c
 *
 * Tests that modifying the binding point of an array of shader
 * storage block works correctly, i.e., former attached buffer is not
 * modified and the other is.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.window_width = 100;
	config.window_height = 100;
	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

#define SSBO_SIZE 4

static const char vs_pass_thru_text[] =
	"#version 330\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"\n"
	"layout(std140, binding=2) buffer ssbo {\n"
	"       vec4 v;\n"
	"} a[2];\n"
	"\n"
	"in vec4 piglit_vertex;\n"
	"\n"
	"void main() {\n"
	"	gl_Position = piglit_vertex;\n"
	"       a[0].v = a[0].v + 1.0;\n"
	"       a[1].v = a[1].v + 10.0;\n"
        "}\n";

static const char fs_source[] =
	"#version 330\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"\n"
	"out vec4 color;\n"
	"\n"
	"layout(std140, binding=2) buffer ssbo {\n"
	"       vec4 v;\n"
	"} a[2];\n"
	"\n"
	"void main() {\n"
	"       color = a[0].v;\n"
	"}\n";

GLuint prog;

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint buffer[2];
	unsigned int i;
	float ssbo_values[SSBO_SIZE] = {0};
	float *map;
	int index;

	piglit_require_extension("GL_ARB_shader_storage_buffer_object");
	piglit_require_extension("GL_ARB_program_interface_query");

	prog = piglit_build_simple_program(vs_pass_thru_text, fs_source);

	glUseProgram(prog);

	glClearColor(0, 0, 0, 0);

	glGenBuffers(2, buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffer[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, SSBO_SIZE*sizeof(GLfloat),
				&ssbo_values[0], GL_DYNAMIC_DRAW);
	/* Change binding point */
	index = glGetProgramResourceIndex(prog,
					  GL_SHADER_STORAGE_BLOCK, "ssbo[0]");
	glShaderStorageBlockBinding(prog, index, 4);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, buffer[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, SSBO_SIZE*sizeof(GLfloat),
				&ssbo_values[0], GL_DYNAMIC_DRAW);

	glViewport(0, 0, piglit_width, piglit_height);

	piglit_draw_rect(-1, -1, 2, 2);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer[0]);
	map = (float *) glMapBuffer(GL_SHADER_STORAGE_BUFFER,  GL_READ_ONLY);

	/* Former bound buffer should not be modified */
	for (i = 0; i < SSBO_SIZE; i++) {
		if (map[i] != 0) {
			printf("Wrong %d value in buffer[0]: %.2f\n",
			       i, map[i]);
			pass = false;
		}
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer[1]);
	map = (float *) glMapBuffer(GL_SHADER_STORAGE_BUFFER,  GL_READ_ONLY);

	for (i = 0; i < SSBO_SIZE; i++) {
                /* Values should be below ten but different than zero */
		if (map[i] == 0 || map[i] > 10) {
			printf("Wrong %d value in buffer[1]: %.2f\n",
			       i, map[i]);
			pass = false;
		}
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	if (!piglit_check_gl_error(GL_NO_ERROR))
	   pass = false;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
