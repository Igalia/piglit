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

/** @file referenced-by-shader.c
 *
 * From the GL_ARB_uniform_buffer_object spec:
 *
 *     "If <pname> is UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER,
 *      UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER, or
 *      UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER, then a boolean
 *      value indicating whether the uniform block identified by
 *      <uniformBlockIndex> is referenced by the vertex, geometry, or
 *      fragment programming stage of <program>, respectively, is
 *      returned."
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_MAIN(
    10 /*window_width*/,
    10 /*window_height*/,
    GLUT_RGB | GLUT_DOUBLE | GLUT_ALPHA)

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	unsigned int i;
	GLuint vs, fs, prog;
	const char *vs_source =
		"#extension GL_ARB_uniform_buffer_object : enable\n"
		"uniform vs { float a; };\n"
		"uniform vsfs { float c; };\n"
		"uniform float dddd;\n"
		"void main() {\n"
		"	gl_Position = gl_Vertex + vec4(a + c);\n"
		"}\n";
	const char *fs_source =
		"#extension GL_ARB_uniform_buffer_object : enable\n"
		"uniform fs { float b; };\n"
		"uniform vsfs { float c; };\n"
		"uniform float dddd;\n"
		"void main() {\n"
		"	gl_FragColor = vec4(b + c);\n"
		"}\n";
	char name[10];

	piglit_require_extension("GL_ARB_uniform_buffer_object");

	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_source);
	if (!fs) {
		fprintf(stderr, "Failed to compile shader:\n%s", fs_source);
		piglit_report_result(PIGLIT_FAIL);
	}
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);
	if (!vs) {
		fprintf(stderr, "Failed to compile shader:\n%s", vs_source);
		piglit_report_result(PIGLIT_FAIL);
	}
	prog = piglit_link_simple_program(vs, fs);
	if (!prog) {
		fprintf(stderr, "Failed to link shaders.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	for (i = 0; i < 3; i++) {
		GLint ref_vs, ref_fs;

		glGetActiveUniformBlockName(prog, i, sizeof(name), NULL, name);

		glGetActiveUniformBlockiv(prog, i,
					  GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER,
					  &ref_vs);
		glGetActiveUniformBlockiv(prog, i,
					  GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER,
					  &ref_fs);

		printf("%10s: %d %d", name, ref_vs, ref_fs);

		if (strcmp(name, "vs") == 0) {
			if (!ref_vs || ref_fs) {
				printf(" FAIL");
				pass = false;
			}
		} else if (strcmp(name, "fs") == 0) {
			if (ref_vs || !ref_fs) {
				printf(" FAIL");
				pass = false;
			}
		} else if (strcmp(name, "vsfs") == 0) {
			if (!ref_vs || !ref_fs) {
				printf(" FAIL");
				pass = false;
			}
		} else {
			printf(" FAIL");
			pass = false;
		}

		printf("\n");
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

