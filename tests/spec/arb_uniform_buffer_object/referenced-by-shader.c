/*
 * Copyright © 2012, 2013 Intel Corporation
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
 * From the GL_ARB_uniform_buffer_object spec and
 * Section 2.11.4(Uniform Variables) of OpenGL 3.2 Core:
 *
 *     "If <pname> is UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER,
 *      UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER, or
 *      UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER, then a boolean
 *      value indicating whether the uniform block identified by
 *      <uniformBlockIndex> is referenced by the vertex, geometry, or
 *      fragment programming stage of <program>, respectively, is
 *      returned."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	unsigned int i;
	GLuint vs, gs, fs, prog;
	const char *vs_source =
		"%s"
		"uniform vs { float v; };\n"
		"uniform vsgs { float vg; };\n"
		"uniform vsfs { float vf; };\n"
		"uniform vsgsfs { float vgf; };\n"
		"void main() {\n"
		"	gl_Position = vec4(v + vg + vf + vgf);\n"
		"}\n";

	const char *gs_source =
		"%s"
		"layout(triangles) in;\n"
		"layout(triangle_strip, max_vertices=3) out;\n"
		"uniform gs { float g; };\n"
		"uniform vsgs { float vg; };\n"
		"uniform gsfs { float gf; };\n"
		"uniform vsgsfs { float vgf; };\n"
		"void main() {\n"
		"	for(int i = 0; i < 3; i++) {\n"
		"		gl_Position = vec4(g + vg + gf + vgf);\n"
		"		EmitVertex();\n"
		"	}\n"
		"}\n";

	const char *fs_source =
		"%s"
		"uniform fs { float f; };\n"
		"uniform vsfs { float vf; };\n"
		"uniform gsfs { float gf; };\n"
		"uniform vsgsfs { float vgf; };\n"
		"void main() {\n"
		"	gl_FragColor = vec4(f + vf + gf + vgf);\n"
		"}\n";

	char name[10];
	bool use_gs = piglit_get_gl_version() >= 32;
	const char *header;
	char *temp_source;
	int num_uniforms_used = 0;

	if (use_gs) {
		header = "#version 150\n";
	} else {
		header = "#extension GL_ARB_uniform_buffer_object : enable\n";
		piglit_require_extension("GL_ARB_uniform_buffer_object");
	}

	prog = glCreateProgram();

	asprintf(&temp_source, vs_source, header);
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, temp_source);
	glAttachShader(prog, vs);
	free(temp_source);

	if (use_gs) {
		asprintf(&temp_source, gs_source, header);
		gs = piglit_compile_shader_text(GL_GEOMETRY_SHADER, temp_source);
		glAttachShader(prog, gs);
		free(temp_source);
	}

	asprintf(&temp_source, fs_source, header);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, temp_source);
	glAttachShader(prog, fs);
	free(temp_source);

	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	if (use_gs) {
		num_uniforms_used = 7;
		printf("            v g f\n");
	} else {
		num_uniforms_used = 6;
		printf("            v f\n");
	}

	for (i = 0; i < num_uniforms_used; i++) {
		GLint ref_vs = 0, ref_gs = 0, ref_fs = 0;
		bool block_fail = false;

		glGetActiveUniformBlockName(prog, i, sizeof(name), NULL, name);

		glGetActiveUniformBlockiv(prog, i,
					  GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER,
					  &ref_vs);
		if (use_gs) {
			glGetActiveUniformBlockiv(prog, i,
						  GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER,
						  &ref_gs);
		}
		glGetActiveUniformBlockiv(prog, i,
					  GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER,
					  &ref_fs);

		if (use_gs) {
			printf("%10s: %d %d %d", name, ref_vs, ref_gs, ref_fs);
		} else {
			printf("%10s: %d %d", name, ref_vs, ref_fs);
		}

		if ((strstr(name, "vs") != 0) != ref_vs)
			block_fail = true;
		if (use_gs) {
			if ((strstr(name, "gs") != 0) != ref_gs)
				block_fail = true;
		}
		if ((strstr(name, "fs") != 0) != ref_fs)
			block_fail = true;

		if (block_fail) {
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

