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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

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
	"void main() { gl_FragColor = color; }\n"
	;

static GLint prog;
static GLint base_location;
static GLint array_location[4];

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

union data_blob {
	float f;
	uint32_t u;
};

void
validate_buffer(const union data_blob *buffer, unsigned size, float expected)
{
	unsigned i;

	if (buffer[0].f != expected) {
		printf("index 0: got %f, expected %f\n",
		       buffer[0].f, expected);
		piglit_report_result(PIGLIT_FAIL);
	} else if (!piglit_automatic) {
		printf("index 0: got %f, expected %f (good)\n",
		       buffer[0].f, expected);
	}

	for (i = 1; i < size; i++) {
		if (buffer[i].u != 0xdeadbeef) {
			printf("glGetUniformfv overrun at index %u!\n", i);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	if (!piglit_automatic) {
		printf("No buffer overrun.\n");
	}
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
	union data_blob buffer[16];

	piglit_require_vertex_shader();
	piglit_require_fragment_shader();

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);

	prog = piglit_link_simple_program(vs, fs);

	glUseProgram(prog);

	base_location = glGetUniformLocation(prog, "c");
	if (base_location < 0) {
		printf("Could not get location of `c'.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	for (i = 0; i < 4; i++) {
		char name[5];

		name[0] = 'c';
		name[1] = '[';
		name[2] = '0' + i;
		name[3] = ']';
		name[4] = '\0';
		array_location[i] = glGetUniformLocation(prog, name);
		if (array_location[i] < 0) {
			printf("Could not get location of `%s'.\n", name);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	/* From page 80 of the OpenGL 2.1 spec:
	 *
	 *     The first element of a uniform array is identified using the
	 *     name of the uniform array appended with "[0]". Except if the
	 *     last part of the string name indicates a uniform array, then
	 *     the location of the first element of that array can be
	 *     retrieved by either using the name of the uniform array, or the
	 *     name of the uniform array appended with "[0]".
	 */
	if (base_location != array_location[0]) {
		printf("Locations of `c' = %d and `c[0]' = %d, but they "
		       "should be the same.\n",
		       base_location, array_location[0]);
		piglit_report_result(PIGLIT_FAIL);
	}

	glUniform1fv(base_location, 4, uniform_data);

	/* From page 264 of the OpenGL 2.1 spec:
	 *
	 *     In order to query the values of an array of uniforms, a
	 *     GetUniform* command needs to be issued for each array element.
	 *
	 * This means that querying using the location of 'array' is the same
	 * as 'array[0]'.
	 */
	printf("Getting array element 0 from base location...\n");
	for (i = 0; i < ARRAY_SIZE(buffer); i++) {
		buffer[i].u = 0xdeadbeef;
	}

	glGetUniformfv(prog, base_location, (GLfloat *) buffer);
	validate_buffer(buffer, ARRAY_SIZE(buffer), uniform_data[0]);

	printf("Getting one array element at a time...\n");
	for (i = 0; i < 4; i++) {
		unsigned j;
		for (j = 0; j < ARRAY_SIZE(buffer); j++) {
			buffer[j].u = 0xdeadbeef;
		}

		glGetUniformfv(prog, array_location[i],
				    (GLfloat *) buffer);
		validate_buffer(buffer, ARRAY_SIZE(buffer), uniform_data[i]);
	}

	piglit_report_result(PIGLIT_PASS);
}
