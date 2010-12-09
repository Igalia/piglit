/*
 * Copyright © 2010 Intel Corporation
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
 * \file getuniform-01.c
 * Verify that glGetUniformfv fetches uniform arrays correctly
 *
 * This test reproduces the failure reported in bugzilla #29823.
 * https://bugs.freedesktop.org/show_bug.cgi?id=29823
 */

#include "piglit-util.h"
#include "piglit-framework.h"

int piglit_width = 20, piglit_height = 20;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

static const char vs_text[] =
	"uniform float c[4];\n"
	"varying vec4 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
	"  color = vec4(c[3], c[2], c[1], c[0]);\n"
	"}\n"
	;

static const char fs_text[] =
	"varying vec4 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = color;\n"
	"}\n"
	;

static GLint prog;
static GLint base_location;
static GLint array_location[4];

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAILURE;
}

void
piglit_init(int argc, char **argv)
{
	static const float uniform_data[4] = {
		12.0, 0.5, 3.14169, 42.0
	};

	GLint vs;
	GLint fs;
	unsigned i;
	union {
		float f;
		uint32_t u;
	} buffer[16];

	piglit_require_vertex_shader();
	piglit_require_fragment_shader();

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);

	prog = piglit_link_simple_program(vs, 0);

	piglit_UseProgram(prog);

	base_location = piglit_GetUniformLocation(prog, "c");
	if (base_location < 0) {
		printf("Could not get location of `c'.\n");
		piglit_report_result(PIGLIT_FAILURE);
	}

	for (i = 0; i < 4; i++) {
		char name[5];

		name[0] = 'c';
		name[1] = '[';
		name[2] = '0' + i;
		name[3] = ']';
		name[4] = '\0';
		array_location[i] = piglit_GetUniformLocation(prog, name);
		if (array_location[i] < 0) {
			printf("Could not get location of `%s'.\n", name);
			piglit_report_result(PIGLIT_FAILURE);
		}
	}

	if (base_location != array_location[0]) {
		printf("Locations of `c' = %d and `c[0]' = %d, but they "
		       "should be the same.\n",
		       base_location, array_location[0]);
		piglit_report_result(PIGLIT_FAILURE);
	}

	piglit_Uniform1fv(base_location, 4, uniform_data);

	printf("Getting all array elements at once...\n");
	for (i = 0; i < ARRAY_SIZE(buffer); i++) {
		buffer[i].u = 0xdeadbeef;
	}

	piglit_GetUniformfv(prog, base_location, (GLfloat *) buffer);

	for (i = 0; i < 4; i++) {
		if (buffer[i].f != uniform_data[i]) {
			printf("index %u: got %f, expected %f\n",
			       i, buffer[i].f, uniform_data[i]);
			piglit_report_result(PIGLIT_FAILURE);
		} else if (!piglit_automatic) {
			printf("index %u: got %f, expected %f (good)\n",
			       i, buffer[i].f, uniform_data[i]);
		}
	}

	for (/* empty */; i < ARRAY_SIZE(buffer); i++) {
		if (buffer[i].u != 0xdeadbeef) {
			printf("glGetUniformfv overrun at index %u!\n", i);
			piglit_report_result(PIGLIT_FAILURE);
		}
	}

	if (!piglit_automatic) {
		printf("No buffer overrun.\n");
	}

	printf("Getting one array element at a time...\n");
	for (i = 0; i < 4; i++) {
		unsigned j;
		for (j = 0; i < ARRAY_SIZE(buffer); j++) {
			buffer[j].u = 0xdeadbeef;
		}

		piglit_GetUniformfv(prog, array_location[i],
				    (GLfloat *) buffer);
		if (buffer[0].f != uniform_data[i]) {
			printf("index %u: got %f, expected %f\n",
			       i, buffer[0].f, uniform_data[i]);
			piglit_report_result(PIGLIT_FAILURE);
		} else if (!piglit_automatic) {
			printf("index %u: got %f, expected %f (good)\n",
			       i, buffer[i].f, uniform_data[i]);
		}

		for (j = 0; j < ARRAY_SIZE(buffer); j++) {
			if (buffer[j].u != 0xdeadbeef) {
				printf("glGetUniformfv overrun at index %u!\n",
				       j);
				piglit_report_result(PIGLIT_FAILURE);
			}
		}

		if (!piglit_automatic) {
			printf("No buffer overrun.\n");
		}
	}

	piglit_report_result(PIGLIT_SUCCESS);
}
