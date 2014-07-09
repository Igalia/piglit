/*
 * Copyright 2012 Google Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

/**
 * \file glsl-uniform-out-of-bounds-2.c
 *
 * glGetUniformLocation should return:
 * -1 for inactive array elements (as reported by glGetActiveUniform)
 * not -1 for active array elements (as reported by glGetActiveUniform)
 * -1 for non-existent array elements (indices outside the array)
 *
 * Write and read some invalid locations and check for GL_INVALID_OPERATION.
 *
 * \author Frank Henigman <fjhenigman@google.com>
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* unreached */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint prog;
	GLint vs, fs;
	int i, k;
	bool pass = true;
	GLint num_active_uniform;
	GLint min = -1, max = -1;
	GLint bogus[99];
	int num_bogus = 0;

	piglit_require_gl_version(20);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER,
		"attribute vec4 p;\n"
		"void main() { gl_Position = p; }\n"
	);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER,
		"uniform vec4 v[4];\n"
		"uniform mat4 m[4];\n"
		"void main() { gl_FragColor = v[1] + m[1][1]; }\n"
	);

	prog = piglit_link_simple_program(vs, fs);
	glUseProgram(prog);
	glGetProgramiv(prog, GL_ACTIVE_UNIFORMS, &num_active_uniform);
	printf("active uniforms %d\n", num_active_uniform);

	/* for each array in shader */
	for (k = 0; k < num_active_uniform; ++k) {
		GLchar name[99];
		GLint num_active_elements;
		GLenum type;
		char *bracket;

		glGetActiveUniform(prog, k, ARRAY_SIZE(name), NULL,
				   &num_active_elements,
				   &type, name);

		/* OpenGL 4.2 and OpenGL ES 3.0 require that the name returned
		 * for an array have "[0]" on the end.  Earlier versions make
		 * it optional.
		 */
		bracket = strchr(name, '[');
		if (bracket != NULL) {
			if (strncmp(bracket, "[0]", 3) != 0) {
				printf("FAIL: invalid uniform array element "
				       "returned: %s\n", name);
				pass = false;
			}
			*bracket = '\0';
		}

		if (!((name[0] == 'v' || name[0] == 'm') && name[1] == 0))
			continue;
		printf("array '%s' active elements %d\n",
		       name, num_active_elements);

		/* for each index in array, plus some before and after */
		for (i = -2; i < 6; ++i) {
			bool is_active = 0 <= i && i < num_active_elements;
			GLchar element[9];
			GLint loc;
			sprintf(element, "%s[%d]", name, i);
			loc = glGetUniformLocation(prog, element);

			/* does glGetUniformLocation agree with
			 * glGetActiveUniform?
			 */
			if (loc == -1) {
				if (is_active) {
					printf("FAIL: no location for "
					       "active %s\n", element);
					pass = false;
				}
			} else {
				if (!is_active) {
					printf("FAIL: got location for "
					       "inactive %s\n", element);
					pass = false;
				}

				/* keep track of location min/max so
				 * we can pick some locations to test
				 * that we know aren't available.
				 */
				if (min == -1 || loc < min)
					min = loc;
				if (max == -1 || loc > max)
					max = loc;
			}
		}
	}

	// make up some bogus locations
	for (i = 1; i < 6; ++i) {
		bogus[num_bogus++] = min - i;
		bogus[num_bogus++] = max + i;
		/* mesa encodes the uniform variable in the upper 16
		 * bits of a location and puts the array index in the
		 * lower 16
		 */
		bogus[num_bogus++] = max + (1<<16) + i - 3;
	}

	/* test writing and reading bogus locations */
	for (i = 0; i < num_bogus; ++i) {
		GLfloat v[16];
		if (bogus[i] == -1)
			continue;
		printf("trying bogus location %d\n", bogus[i]);
		glUniform4fv(bogus[i], 1, v);
		if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
			printf("FAIL: wrote vector to bogus location\n");
			pass = false;
		}
		glUniformMatrix4fv(bogus[i], 1, GL_FALSE, v);
		if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
			printf("FAIL: wrote matrix to bogus location\n");
			pass = false;
		}
		glGetUniformfv(prog, bogus[i], v);
		if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
			printf("FAIL: read from bogus location\n");
			pass = false;
		}
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
