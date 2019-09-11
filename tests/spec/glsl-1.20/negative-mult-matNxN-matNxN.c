/*
 * Copyright © 2019 Intel Corporation
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

/** @file mult-matNxN-matNxN.c
 */

/* From section 5.9 of the GLSL spec:
 *     "The operator is multiply (*), where both operands are matrices or one operand is a vector and the
 *      other a matrix. A right vector operand is treated as a column vector and a left vector operand as a
 *      row vector. In all these cases, it is required that the number of columns of the left operand is equal
 *      to the number of rows of the right operand. Then, the multiply (*) operation does a linear
 *      algebraic multiply, yielding an object that has the same number of rows as the left operand and the
 *      same number of columns as the right operand. Section 5.10 “Vector and Matrix Operations”
 *      explains in more detail how vectors and matrices are operated on."
 *
 * This test checks just invalid matrix combinations because
 * we already have tests which check valid combinations
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 12;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

typedef
struct glsl_mat {
	unsigned col, row;
} glsl_mat_t;

static void
make_mat_type_name(glsl_mat_t m, char* buffer)
{
	if(m.col == m.row)
		sprintf(buffer, "mat%u", m.col);
	else
		sprintf(buffer, "mat%ux%u", m.col, m.row);
}

static const char*
make_shader_mult(glsl_mat_t first_type,
				 glsl_mat_t second_type)
{
	static char first_mat_name[32];
	static char second_mat_name[32];
	static char buffer[512];
	static char code[] =
		"#version 120\n"
		"void main() {\n"
		"	float t = (%s(1.3) * %s(1.3))[0][0];\n"
		"}\n"
		;
	make_mat_type_name(first_type, first_mat_name);
	make_mat_type_name(second_type, second_mat_name);
	sprintf(buffer, code, first_mat_name, second_mat_name);
	return buffer;
}

static const char*
make_shader_assignment_mult(glsl_mat_t first_type,
							glsl_mat_t second_type)
{
	static char first_mat_name[32];
	static char second_mat_name[32];
	static char buffer[512];
	static char code[] =
		"#version 120\n"
		"void main() {\n"
		"	%s p1;\n"
		"	p1 = %s(1.0);\n"
		"	%s p2;\n"
		"	p2 = %s(1.0);\n"
		"	p1 *= p2;\n"
		"}\n"
		;
	make_mat_type_name(first_type, first_mat_name);
	make_mat_type_name(second_type, second_mat_name);
	sprintf(buffer, code, first_mat_name, first_mat_name,
						second_mat_name, second_mat_name);
	return buffer;
}

static bool
check_compilation(GLenum shader_type, const char *shader_source)
{
	int compile_status = 0;
	GLint sh = glCreateShader(shader_type);
	if (!sh)
		return false;

	glShaderSource(sh, 1, &shader_source, NULL);
	glCompileShader(sh);
	glGetShaderiv(sh, GL_COMPILE_STATUS, &compile_status);
	glDeleteShader(sh);
	return (compile_status == GL_TRUE);
}

static bool
test(GLenum shader_type, const char* (*mkshader)(glsl_mat_t, glsl_mat_t))
{
	bool pass = true;
	/* test all invalid matrix combinations */
	glsl_mat_t a;
	for (a.col = 2; a.col < 5; a.col++) {
		for (a.row = 2; a.row < 5; a.row++) {

			glsl_mat_t b;
			for (b.col = 2; b.col < 5; b.col++) {
				for (b.row = 2; b.row < 5; b.row++) {

					if (a.col == b.row)
						continue;

					const char *shader_source = mkshader(a, b);

					if (!check_compilation(shader_type, shader_source))
						continue;

					fprintf(stderr,
							"error: the following %s shader must fail compilation:\n%s\n",
							(shader_type == GL_VERTEX_SHADER) ? "vertex" : "fragment",
							shader_source);
					pass = false;
				}
			}
		}
	}
	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	piglit_require_GLSL_version(120);
	pass = test(GL_VERTEX_SHADER, make_shader_mult) && pass;
	pass = test(GL_FRAGMENT_SHADER, make_shader_mult) && pass;
	pass = test(GL_VERTEX_SHADER, make_shader_assignment_mult) && pass;
	pass = test(GL_FRAGMENT_SHADER, make_shader_assignment_mult) && pass;
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
