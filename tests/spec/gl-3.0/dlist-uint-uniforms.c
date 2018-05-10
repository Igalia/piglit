/*
 * Copyright © 2014 Intel Corporation
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
 * Verify that unsigned glUniform* commands added in GL 3.0 are compiled into
 * display lists.
 *
 * This test is adapted from tests/spec/arb_separate_shader_objects/dlist.c
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	/* No supports_gl_core_version setting because there are no display
	 * lists in core profile.
	 */
	config.supports_gl_compat_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static bool Uniformui(void);

void
piglit_init(int argc, char **argv)
{
	unsigned glsl_version;
	bool pass = true;

	pass = Uniformui() && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum mode {
	set_scalar,
	set_vector,
	get_and_compare
};

#define UINT_UNIFORM(type, n, suffix)				\
	do {							\
		type inbuf[n];					\
		type outbuf[n];					\
		unsigned jjj;					\
								\
		for (jjj = 0; jjj < n; jjj++)			\
			outbuf[jjj] = (type) value++;		\
								\
		switch (m) {					\
		case set_scalar:				\
			switch (n) {				\
			case 1:					\
				glUniform1 ## suffix		\
					(loc,			\
					 outbuf[0]);		\
				break;				\
			case 2:					\
				glUniform2 ## suffix		\
					(loc,			\
					 outbuf[0],		\
					 outbuf[1]);		\
				break;				\
			case 3:					\
				glUniform3 ## suffix		\
					(loc,			\
					 outbuf[0],		\
					 outbuf[1],		\
					 outbuf[2]);		\
				break;				\
			case 4:					\
				glUniform4 ## suffix		\
					(loc,			\
					 outbuf[0],		\
					 outbuf[1],		\
					 outbuf[2],		\
					 outbuf[3]);		\
				break;				\
			default:				\
				printf("internal error - "	\
				       "cannot set_scalar a "	\
				       "%d count\n", n);	\
				pass = false;			\
				break;				\
			}					\
			break;					\
								\
		case set_vector:				\
			glUniform ## n ## suffix ## v		\
				(loc, 1, outbuf);		\
			break;					\
								\
		case get_and_compare:				\
			glGetUniform ## suffix ## v		\
				(prog, loc, inbuf);		\
			if (memcmp(inbuf, outbuf,		\
				   sizeof(type) * n) != 0) {	\
				printf("            %s data "	\
				       "does not match.\n",	\
				       name);			\
				pass = false;			\
			}					\
			break;					\
		}						\
	} while (0)

/**
 * Set or get/verify all the active uniforms in a program
 *
 * \param prog        Program to operate on
 * \param base_value  Value set (or expected) for the first element of the
 *                    first uniform.  Each element expects a successively
 *                    incremented value.
 * \param m           Mode of operation.  Set using scalars (e.g., using
 *                    \c glUniform4f), set using vectors (e.g., using
 *                    \c glUniform4fv), or get and verify.
 */
bool
process_program_uniforms(GLuint prog, unsigned base_value, enum mode m)
{
	unsigned num_uniforms;
	unsigned i;
	unsigned value;
	bool pass = true;

	glGetProgramiv(prog, GL_ACTIVE_UNIFORMS, (GLint *) &num_uniforms);

	value = base_value;
	for (i = 0; i < num_uniforms; i++) {
		GLint size;
		GLenum type;
		char name[64];
		GLuint loc;

		glGetActiveUniform(prog, i, sizeof(name), NULL,
				   &size, &type, name);

		loc = glGetUniformLocation(prog, name);
		if (loc == -1) {
			printf("%s was active, but could not get location.\n",
			       name);
			pass = false;
			continue;
		}

		switch (type) {

		case GL_UNSIGNED_INT:
			UINT_UNIFORM(unsigned, 1, ui);
			break;
		case GL_UNSIGNED_INT_VEC2:
			UINT_UNIFORM(unsigned, 2, ui);
			break;
		case GL_UNSIGNED_INT_VEC3:
			UINT_UNIFORM(unsigned, 3, ui);
			break;
		case GL_UNSIGNED_INT_VEC4:
			UINT_UNIFORM(unsigned, 4, ui);
			break;
		}
	}

	return pass;
}

static bool
process_shader(const char *func, const char *source)
{
	static const struct {
		GLenum list_mode;
		enum mode setter_mode;
		const char *setter_mode_name;
		unsigned base_value;
	} tests[] = {
		{
			GL_COMPILE,
			set_scalar, "scalar",
			5
		},
		{
			GL_COMPILE,
			set_vector, "vector",
			7
		},
		{
			GL_COMPILE_AND_EXECUTE,
			set_scalar, "scalar",
			11
		},
		{
			GL_COMPILE_AND_EXECUTE,
			set_vector, "vector",
			13
		}
	};

	bool pass = true;

	printf("Testing gl%s\n", func);

	GLuint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, source);
	GLuint prog = piglit_link_simple_program(vs, 0);

	glUseProgram(prog);

	GLuint list = glGenLists(1);

	for (unsigned i = 0; i < ARRAY_SIZE(tests); i++) {
		const unsigned post_compile_base_value =
			(tests[i].list_mode == GL_COMPILE)
			? 0 : tests[i].base_value;

		printf("    %s: %s mode\n",
		       piglit_get_gl_enum_name(tests[i].list_mode),
		       tests[i].setter_mode_name);

		printf("        pre-initialize\n");
		pass = process_program_uniforms(prog, 0, tests[i].setter_mode)
			&& pass;
		pass = process_program_uniforms(prog, 0, get_and_compare)
			&& pass;

		glNewList(list, tests[i].list_mode);
		printf("        compiling\n");
		pass = process_program_uniforms(prog,
						tests[i].base_value,
						tests[i].setter_mode)
			&& pass;
		glEndList();

		printf("        post-compile verify\n");
		pass = process_program_uniforms(prog, post_compile_base_value,
						get_and_compare)
			&& pass;

		/* Reset the values back.  This is useful if GL_COMPILE
		 * executed the commands and for GL_COMPILE_AND_EXECUTE.  We
		 * want to know that glCallList changed things.
		 */
		printf("        restore original values\n");
		pass = process_program_uniforms(prog, 0, tests[i].setter_mode)
			&& pass;
		pass = process_program_uniforms(prog, 0, get_and_compare)
			&& pass;

		printf("        post-glCallList verify\n");
		glCallList(list);
		pass = process_program_uniforms(prog, tests[i].base_value,
						get_and_compare)
			&& pass;
	}

	glDeleteLists(list, 1);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	return pass;
}

bool
Uniformui(void)
{
	const char *source =
		"#version 130\n"
		"uniform uint s;\n"
		"uniform uvec2 v2;\n"
		"uniform uvec3 v3;\n"
		"uniform uvec4 v4;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position = vec4(v3, s) + vec4(v2, v2) + vec4(v4);\n"
		"}\n"
		;

	return process_shader(__func__, source);
}

enum piglit_result
piglit_display(void)
{
	/* NOTREACHED */
	return PIGLIT_FAIL;
}
