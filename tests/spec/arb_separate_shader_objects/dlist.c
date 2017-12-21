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
 * \file dlist.c
 * Verify the commands that are / are not compiled into display lists
 *
 * The GL_ARB_separate_shader_objects spec says:
 *
 *     "Add CreateShaderProgram, GenProgramPipelines, DeleteProgramPipelines,
 *     and BindProgramPipelines to the "Program and shader objects" list of
 *     commands that cannot be compiled into a display list but are instead
 *     executed immediately."
 *
 * The issues section further says:
 *
 *     "9.  Is glUseProgramStages allowed to be compiled within a
 *          display list?
 *
 *          RESOLVED:  Yes, just like glUseProgram is allowed within a
 *          display list.
 *
 *     ...
 *
 *      11. Can glCreateShaderProgram be compiled into a display list?
 *
 *          RESOLVED:  No.
 *
 *          glCreateShaderProgram is equivalent to a sequence of commands
 *          that are themselves not allowed to be compiled  into a display
 *          list."
 */
#include "piglit-util-gl.h"
#include "sso-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	/* No supports_gl_core_version setting because there are no display
	 * lists in core profile.
	 */
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static bool GenProgramPipelines(void);
static bool DeleteProgramPipelines(void);
static bool BindProgramPipelines(void);
static bool CreateShaderProgramv(void);
static bool ProgramUniformf(void);
static bool ProgramUniformd(void);
static bool ProgramUniformi(void);
static bool ProgramUniformui(void);
static bool ProgramUniformMatrixf(void);
static bool ProgramUniformMatrixd(void);
static bool UseProgramStages(void);

void
piglit_init(int argc, char **argv)
{
	unsigned glsl_version;
	bool pass = true;

	piglit_require_vertex_shader();
	piglit_require_fragment_shader();
	piglit_require_extension("GL_ARB_separate_shader_objects");

	glsl_version = pick_a_glsl_version();

	pass = GenProgramPipelines() && pass;
	pass = DeleteProgramPipelines() && pass;
	pass = BindProgramPipelines() && pass;
	pass = CreateShaderProgramv() && pass;
	pass = UseProgramStages() && pass;

	pass = ProgramUniformf() && pass;
	pass = ProgramUniformi() && pass;

	if (glsl_version >= 130)
		pass = ProgramUniformui() && pass;

	if (glsl_version >= 120)
		pass = ProgramUniformMatrixf() && pass;

	if (glsl_version >= 130
	    && piglit_is_extension_supported("GL_ARB_gpu_shader_fp64")) {
		pass = ProgramUniformd() && pass;
		pass = ProgramUniformMatrixd() && pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

bool
GenProgramPipelines(void)
{
	bool pass = true;
	GLuint list;
	GLuint pipe;

	printf("Testing gl%s\n", __func__);

	list = glGenLists(1);

	glNewList(list, GL_COMPILE);
	pipe = 0;
	glGenProgramPipelines(1, &pipe);
	glEndList();

	if (pipe == 0) {
		printf("    gl%s did not execute immediately.\n", __func__);
		pass = false;
	}

	glDeleteProgramPipelines(1, &pipe);
	pipe = 0;

	glCallList(list);

	if (pipe != 0) {
		glDeleteProgramPipelines(1, &pipe);
		printf("    gl%s was compiled in display list.\n", __func__);
		pass = false;
	}

	glDeleteLists(list, 1);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	return pass;
}

bool
DeleteProgramPipelines(void)
{
	bool pass = true;
	GLuint list;
	GLuint pipe;

	printf("Testing gl%s\n", __func__);

	/* Must generate the program pipeline after generating the name so
	 * that it will be "live."  Otherwise, glIsProgramPipeline will return
	 * false even if the name hasn't been deleted.
	 */
	glGenProgramPipelines(1, &pipe);
	glBindProgramPipeline(pipe);
	glBindProgramPipeline(0);

	if (!glIsProgramPipeline(pipe)) {
		printf("    Program pipeline is not \"live.\"\n");
		pass = false;
	}

	list = glGenLists(1);

	glNewList(list, GL_COMPILE);
	glDeleteProgramPipelines(1, &pipe);
	glEndList();

	if (glIsProgramPipeline(pipe)) {
		printf("    gl%s did not execute immediately.\n", __func__);
		pass = false;
	}

	/* There is no way to determine whether glDeleteProgramPipelines is
	 * compiled into the display list.  The object is already deleted, so
	 * we can't use glIsProgramPipeline.  Deleting an already deleted
	 * object doesn't generate an error.
	 */

	glDeleteLists(list, 1);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	return pass;
}

bool
BindProgramPipelines(void)
{
	bool pass = true;
	GLuint list;
	GLuint pipe;
	GLuint binding;

	printf("Testing gl%s\n", __func__);

	glGenProgramPipelines(1, &pipe);

	list = glGenLists(1);

	glNewList(list, GL_COMPILE);
	glBindProgramPipeline(pipe);
	glEndList();

	glGetIntegerv(GL_PROGRAM_PIPELINE_BINDING, (GLint *) &binding);
	if (binding != pipe) {
		printf("    gl%s did not execute immediately.\n", __func__);
		pass = false;
	}

	glBindProgramPipeline(0);
	glCallList(list);

	glGetIntegerv(GL_PROGRAM_PIPELINE_BINDING, (GLint *) &binding);
	if (binding != 0) {
		glBindProgramPipeline(0);
		printf("    gl%s was compiled in display list.\n", __func__);
		pass = false;
	}

	glDeleteProgramPipelines(1, &pipe);
	glDeleteLists(list, 1);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	return pass;
}

bool
CreateShaderProgramv(void)
{
	static const char *source =
		"void main() { gl_Position = vec4(0); }";
	bool pass = true;
	GLuint list;
	GLuint prog;

	printf("Testing gl%s\n", __func__);

	list = glGenLists(1);

	glNewList(list, GL_COMPILE);
	prog = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &source);
	glEndList();

	if (prog == 0) {
		printf("    gl%s did not execute immediately.\n", __func__);
		pass = false;
	}

	/* Since glCreateShaderProgramv returns a value, it is not clear how
	 * to test whether or not it was compiled into the display list.
	 */

	glDeleteProgram(prog);
	glDeleteLists(list, 1);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	return pass;
}

bool
UseProgramStages(void)
{
	static const char *vs_source =
		"void main() { gl_Position = gl_Vertex; }";
	static const char *red_fs_source =
		"void main() { gl_FragColor = vec4(1, 0, 0, 1); }";
	static const char *green_fs_source =
		"void main() { gl_FragColor = vec4(0, 1, 0, 1); }";
	bool pass = true;
	GLuint list;
	GLuint pipe;
	GLuint vert_prog;
	GLuint red_frag_prog;
	GLuint green_frag_prog;
	GLuint prog;

	printf("Testing gl%s\n", __func__);

	vert_prog = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vs_source);
	red_frag_prog = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1,
					       &red_fs_source);
	green_frag_prog = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1,
						 &green_fs_source);

	glGenProgramPipelines(1, &pipe);
	glBindProgramPipeline(pipe);

	glUseProgramStages(pipe, GL_VERTEX_SHADER_BIT, vert_prog);
	glUseProgramStages(pipe, GL_FRAGMENT_SHADER_BIT, red_frag_prog);

	list = glGenLists(1);

	glNewList(list, GL_COMPILE);
	glUseProgramStages(pipe, GL_FRAGMENT_SHADER_BIT, green_frag_prog);
	glEndList();

	glGetProgramPipelineiv(pipe, GL_FRAGMENT_SHADER, (GLint *) &prog);
	if (prog != red_frag_prog) {
		printf("    gl%s executed immediately.\n", __func__);
		pass = false;
	}

	/* Restore the red program (just in case the green program was
	 * incorrectly bound during display list compilation.
	 */
	glUseProgramStages(pipe, GL_FRAGMENT_SHADER_BIT, red_frag_prog);

	/* Call the list to use the green program, and query the result.
	 */
	glCallList(list);

	glGetProgramPipelineiv(pipe, GL_FRAGMENT_SHADER, (GLint *) &prog);
	if (prog != green_frag_prog) {
		printf("    gl%s was not compiled into the display list.\n",
		       __func__);
		pass = false;
	}

	glBindProgramPipeline(0);
	glDeleteProgram(vert_prog);
	glDeleteProgram(red_frag_prog);
	glDeleteProgram(green_frag_prog);
	glDeleteProgramPipelines(1, &pipe);
	glDeleteLists(list, 1);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	return pass;
}

enum mode {
	set_scalar,
	set_vector,
	get_and_compare
};

#define NONMATRIX_UNIFORM(type, n, suffix)			\
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
				glProgramUniform1 ## suffix	\
					(prog, loc,		\
					 outbuf[0]);		\
				break;				\
			case 2:					\
				glProgramUniform2 ## suffix	\
					(prog, loc,		\
					 outbuf[0],		\
					 outbuf[1]);		\
				break;				\
			case 3:					\
				glProgramUniform3 ## suffix	\
					(prog, loc,		\
					 outbuf[0],		\
					 outbuf[1],		\
					 outbuf[2]);		\
				break;				\
			case 4:					\
				glProgramUniform4 ## suffix	\
					(prog, loc,		\
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
			glProgramUniform ## n ## suffix ## v	\
				(prog, loc, 1, outbuf);		\
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

#define glProgramUniformMatrix2x2fv glProgramUniformMatrix2fv
#define glProgramUniformMatrix3x3fv glProgramUniformMatrix3fv
#define glProgramUniformMatrix4x4fv glProgramUniformMatrix4fv
#define glProgramUniformMatrix2x2dv glProgramUniformMatrix2dv
#define glProgramUniformMatrix3x3dv glProgramUniformMatrix3dv
#define glProgramUniformMatrix4x4dv glProgramUniformMatrix4dv

#define MATRIX_UNIFORM(type, r, c, suffix)				\
	do {								\
		type inbuf[r * c];					\
		type outbuf[r * c];					\
		unsigned jjj;						\
									\
		for (jjj = 0; jjj < (r * c); jjj++)			\
			outbuf[jjj] = (type) value++;			\
									\
		switch (m) {						\
		case set_scalar:					\
			printf("internal error - cannot set_scalar a "	\
			       "matrix\n");				\
			pass = false;					\
			break;						\
									\
		case set_vector:					\
			glProgramUniformMatrix ## r ## x ## c ## suffix	## v \
				(prog, loc, 1, GL_FALSE, outbuf);	\
				break;					\
									\
		case get_and_compare:					\
			glGetUniform ## suffix ## v			\
				(prog, loc, inbuf);			\
			if (memcmp(inbuf, outbuf,			\
				   sizeof(type) * r * c) != 0) {	\
				printf("            %s data "		\
				       "does not match.\n",		\
				       name);				\
				pass = false;				\
			}						\
			break;						\
		}							\
	} while (0)

/**
 * Set or get/verify all the active uniforms in a program
 *
 * \param prog        Program to operate on
 * \param base_value  Value set (or expected) for the first element of the
 *                    first uniform.  Each element expects a successively
 *                    incremented value.
 * \param m           Mode of operation.  Set using scalars (e.g., using
 *                    \c glProgramUniform4f), set using vectors (e.g., using
 *                    \c glProgramUniform4fv), or get and verify.
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
		case GL_FLOAT:
			NONMATRIX_UNIFORM(float, 1, f);
			break;
		case GL_FLOAT_VEC2:
			NONMATRIX_UNIFORM(float, 2, f);
			break;
		case GL_FLOAT_VEC3:
			NONMATRIX_UNIFORM(float, 3, f);
			break;
		case GL_FLOAT_VEC4:
			NONMATRIX_UNIFORM(float, 4, f);
			break;

		case GL_DOUBLE:
			NONMATRIX_UNIFORM(double, 1, d);
			break;
		case GL_DOUBLE_VEC2:
			NONMATRIX_UNIFORM(double, 2, d);
			break;
		case GL_DOUBLE_VEC3:
			NONMATRIX_UNIFORM(double, 3, d);
			break;
		case GL_DOUBLE_VEC4:
			NONMATRIX_UNIFORM(double, 4, d);
			break;

		case GL_INT:
			NONMATRIX_UNIFORM(int, 1, i);
			break;
		case GL_INT_VEC2:
			NONMATRIX_UNIFORM(int, 2, i);
			break;
		case GL_INT_VEC3:
			NONMATRIX_UNIFORM(int, 3, i);
			break;
		case GL_INT_VEC4:
			NONMATRIX_UNIFORM(int, 4, i);
			break;

		case GL_UNSIGNED_INT:
			NONMATRIX_UNIFORM(unsigned, 1, ui);
			break;
		case GL_UNSIGNED_INT_VEC2:
			NONMATRIX_UNIFORM(unsigned, 2, ui);
			break;
		case GL_UNSIGNED_INT_VEC3:
			NONMATRIX_UNIFORM(unsigned, 3, ui);
			break;
		case GL_UNSIGNED_INT_VEC4:
			NONMATRIX_UNIFORM(unsigned, 4, ui);
			break;

		case GL_FLOAT_MAT2:
			MATRIX_UNIFORM(float, 2, 2, f);
			break;
		case GL_FLOAT_MAT2x3:
			MATRIX_UNIFORM(float, 2, 3, f);
			break;
		case GL_FLOAT_MAT2x4:
			MATRIX_UNIFORM(float, 2, 4, f);
			break;
		case GL_FLOAT_MAT3x2:
			MATRIX_UNIFORM(float, 3, 2, f);
			break;
		case GL_FLOAT_MAT3:
			MATRIX_UNIFORM(float, 3, 3, f);
			break;
		case GL_FLOAT_MAT3x4:
			MATRIX_UNIFORM(float, 3, 4, f);
			break;
		case GL_FLOAT_MAT4x2:
			MATRIX_UNIFORM(float, 4, 2, f);
			break;
		case GL_FLOAT_MAT4x3:
			MATRIX_UNIFORM(float, 4, 3, f);
			break;
		case GL_FLOAT_MAT4:
			MATRIX_UNIFORM(float, 4, 4, f);
			break;

		case GL_DOUBLE_MAT2:
			MATRIX_UNIFORM(double, 2, 2, d);
			break;
		case GL_DOUBLE_MAT2x3:
			MATRIX_UNIFORM(double, 2, 3, d);
			break;
		case GL_DOUBLE_MAT2x4:
			MATRIX_UNIFORM(double, 2, 4, d);
			break;
		case GL_DOUBLE_MAT3x2:
			MATRIX_UNIFORM(double, 3, 2, d);
			break;
		case GL_DOUBLE_MAT3:
			MATRIX_UNIFORM(double, 3, 3, d);
			break;
		case GL_DOUBLE_MAT3x4:
			MATRIX_UNIFORM(double, 3, 4, d);
			break;
		case GL_DOUBLE_MAT4x2:
			MATRIX_UNIFORM(double, 4, 2, d);
			break;
		case GL_DOUBLE_MAT4x3:
			MATRIX_UNIFORM(double, 4, 3, d);
			break;
		case GL_DOUBLE_MAT4:
			MATRIX_UNIFORM(double, 4, 4, d);
			break;
		}
	}

	return pass;
}

static bool
process_shader(const char *func, const char *source, bool matrix)
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

	GLuint prog;
	GLuint list;
	unsigned i;
	bool pass = true;

	printf("Testing gl%s\n", func);

	prog = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &source);

	list = glGenLists(1);

	for (i = 0; i < ARRAY_SIZE(tests); i++) {
		const unsigned post_compile_base_value =
			(tests[i].list_mode == GL_COMPILE)
			? 0 : tests[i].base_value;

		if (matrix && tests[i].setter_mode == set_scalar)
			continue;

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
ProgramUniformf(void)
{
	const char *source =
		"uniform float s;\n"
		"uniform vec2 v2;\n"
		"uniform vec3 v3;\n"
		"uniform vec4 v4;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position = vec4(v3, s) + vec4(v2, v2) + vec4(v4);\n"
		"}\n"
		;

	return process_shader(__func__, source, false);
}

bool
ProgramUniformd(void)
{
	const char *source =
		"#version 130\n"
		"#extension GL_ARB_gpu_shader_fp64: require\n"
		"uniform double s;\n"
		"uniform dvec2 v2;\n"
		"uniform dvec3 v3;\n"
		"uniform dvec4 v4;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position = vec4(v3, s) + vec4(v2, v2) + vec4(v4);\n"
		"}\n"
		;

	return process_shader(__func__, source, false);
}

bool
ProgramUniformi(void)
{
	const char *source =
		"uniform int s;\n"
		"uniform ivec2 v2;\n"
		"uniform ivec3 v3;\n"
		"uniform ivec4 v4;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position = vec4(v3, s) + vec4(v2, v2) + vec4(v4);\n"
		"}\n"
		;

	return process_shader(__func__, source, false);
}

bool
ProgramUniformui(void)
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

	return process_shader(__func__, source, false);
}

bool
ProgramUniformMatrixf(void)
{
	const char *source =
		"#version 120\n"
		"uniform mat2x2 m22;\n"
		"uniform mat2x3 m23;\n"
		"uniform mat2x4 m24;\n"
		"uniform mat3x2 m32;\n"
		"uniform mat3x3 m33;\n"
		"uniform mat3x4 m34;\n"
		"uniform mat4x2 m42;\n"
		"uniform mat4x3 m43;\n"
		"uniform mat4x4 m44;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position = "
		"vec4(m22[0], 0, 0) + vec4(m32[0], 0, 0) + vec4(m42[0], 0, 0) "
		"+ vec4(m23[0], 0)  + vec4(m33[0], 0)    + vec4(m43[0], 0) "
		"+ vec4(m24[0])     + vec4(m34[0])       + vec4(m44[0]);\n"
		"}\n"
		;

	return process_shader(__func__, source, true);
}

bool
ProgramUniformMatrixd(void)
{
	const char *source =
		"#version 130\n"
		"#extension GL_ARB_gpu_shader_fp64: require\n"
		"uniform dmat2x2 m22;\n"
		"uniform dmat2x3 m23;\n"
		"uniform dmat2x4 m24;\n"
		"uniform dmat3x2 m32;\n"
		"uniform dmat3x3 m33;\n"
		"uniform dmat3x4 m34;\n"
		"uniform dmat4x2 m42;\n"
		"uniform dmat4x3 m43;\n"
		"uniform dmat4x4 m44;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position = "
		"vec4(m22[0], 0, 0) + vec4(m32[0], 0, 0) + vec4(m42[0], 0, 0) "
		"+ vec4(m23[0], 0)  + vec4(m33[0], 0)    + vec4(m43[0], 0) "
		"+ vec4(m24[0])     + vec4(m34[0])       + vec4(m44[0]);\n"
		"}\n"
		;

	return process_shader(__func__, source, true);
}

enum piglit_result
piglit_display(void)
{
	/* NOTREACHED */
	return PIGLIT_FAIL;
}
