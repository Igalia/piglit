/*
 * Copyright © 2019 Advanced Micro Devices, Inc.
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
/*
 * Tests the interactions of EXT_direct_state_access and ARB_vertex_program
 *
 * When ARB_vertex_program is supported, EXT_dsa adds 9 program commands and
 * queries:
 *     NamedProgramStringEXT
 *     NamedProgramStringEXT
 *     NamedProgramLocalParameter4dEXT
 *     NamedProgramLocalParameter4dvEXT
 *     NamedProgramLocalParameter4fEXT
 *     NamedProgramLocalParameter4fvEXT
 *     GetNamedProgramLocalParameterdvEXT
 *     GetNamedProgramLocalParameterfvEXT
 *     GetNamedProgramivEXT
 *     GetNamedProgramStringEXT
 *
 * Each time a one of these functions is called we make sure that the named
 * program is not bound.
 * The NamedProgram* functions can be compiled in display list so the 3 dlist
 * modes are tested as well.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
		PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static GLenum use_display_list = GL_NONE;
static GLuint list;

static void
n_floats(float* m, int n)
{
	for (unsigned i = 0; i < n; ++i) {
		m[i] = (float) (rand() % 1000);
	}
}

static void
n_doubles(double* m, int n)
{
	for (unsigned i = 0; i < n; ++i) {
		m[i] = (double) (rand() % 1000);
	}
}

static enum piglit_result
test_NamedProgramStringEXT(void* data)
{
	bool pass = true;
	static const GLchar *const vp_code =
		"!!ARBvp1.0\n"
		"MOV result.position, {0, 0, 1, 0};\n"
		"END";
	static const GLchar *const fp_code =
		"!!ARBfp1.0\n"
		"MOV	result.color, fragment.color;\n"
		"END"
		;
	char get[512];
	GLenum target = (GLenum) (intptr_t) data;
	const char* code = (target == GL_VERTEX_PROGRAM_ARB) ? vp_code : fp_code;

	GLuint program = rand() % INT_MAX;

	if (use_display_list != GL_NONE)
		glNewList(list, use_display_list);

	glNamedProgramStringEXT(
	 	program,
		target,
		GL_PROGRAM_FORMAT_ASCII_ARB,
		strlen(code),
		code);

	if (use_display_list != GL_NONE)
		glEndList(list);

	if (use_display_list == GL_COMPILE) {
		pass = !glIsProgramARB(program) && pass;
		glCallList(list);
	}
	pass = glIsProgramARB(program) && pass;

	glGetNamedProgramStringEXT(
		program,
		target,
		GL_PROGRAM_STRING_ARB,
		get);

	pass = memcmp(code, get, strlen(code)) == 0 && pass;

	glDeleteProgramsARB(1, &program);

	return piglit_check_gl_error(GL_NO_ERROR) && pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_NamedProgramLocalParameter4fEXT(void* data)
{
	bool pass = true;
	int max_param;

	static const GLenum targets[] = {
		GL_VERTEX_PROGRAM_ARB,
		GL_FRAGMENT_PROGRAM_ARB
	};

	for (unsigned i = 0; i < ARRAY_SIZE(targets); i++) {
		GLuint programs[2];
		GLenum target = targets[i];
		float* values, *got, *original;
		glGetProgramivARB(target, GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB, &max_param);

		glGenProgramsARB(2, programs);
		glBindProgramARB(target, programs[1]);

		values = (float*) malloc(sizeof(float) * max_param * 4);
		got = (float*) malloc(2 * sizeof(float) * max_param * 4);
		original = (float*) malloc(sizeof(float) * max_param * 4);
		n_floats(values, max_param * 4);

		/* Read initial values */
		for (unsigned j = 0; j < max_param; j++) {
			glGetNamedProgramLocalParameterfvEXT(
				programs[0],
				target,
				j,
				&original[4 * j]);
		}

		if (use_display_list != GL_NONE)
			glNewList(list, use_display_list);

		/* Update all parameters */
		for (unsigned j = 0; j < max_param; j++) {
			if (j % 2) {
				glNamedProgramLocalParameter4fEXT(
					programs[0],
					target,
					j,
					values[4 * j], values[4 * j + 1],
					values[4 * j + 2], values[4 * j + 3]);
			} else {
				glNamedProgramLocalParameter4fvEXT(
					programs[0],
					target,
					j,
					&values[4 * j]);
			}
		}

		if (use_display_list != GL_NONE) {
			glEndList(list);
		}

		/* Re-read values */
		for (unsigned j = 0; j < max_param; j++) {
			glGetNamedProgramLocalParameterfvEXT(
				programs[0],
				target,
				j,
				&got[4 * j]);
			glGetProgramLocalParameterfvARB(
				target,
				j,
				&got[4 * (max_param + j)]);
		}

		if (use_display_list == GL_COMPILE) {
			/* Values shouldn't have been modified yet */
			pass = memcmp(original, got, sizeof(float) * max_param * 4) == 0 && pass;
			/* Execute list and re-read values */
			glCallList(list);
			for (unsigned j = 0; j < max_param; j++) {
				glGetNamedProgramLocalParameterfvEXT(
					programs[0],
					target,
					j,
					&got[4 * j]);
			}
		}

		/* Check that programs[1] values have been modified */
		pass = memcmp(values, got, sizeof(float) * max_param * 4) == 0 && pass;
		/* Check that programs[0] values have been untouched */
		pass = memcmp(original, &got[4 * max_param], sizeof(float) * max_param * 4) == 0 && pass;

		free(values);
		free(got);
		free(original);
		glDeleteProgramsARB(2, programs);
	}
	return pass && piglit_check_gl_error(GL_NO_ERROR) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_NamedProgramLocalParameter4dEXT(void* data)
{
	bool pass = true;
	int max_param;

	static const GLenum targets[] = {
		GL_VERTEX_PROGRAM_ARB,
		GL_FRAGMENT_PROGRAM_ARB
	};

	for (unsigned i = 0; i < ARRAY_SIZE(targets); i++) {
		GLuint programs[2];
		GLenum target = targets[i];
		double* values, *got, *original;

		glGenProgramsARB(2, programs);
		glBindProgramARB(target, programs[1]);

		glGetProgramivARB(target, GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB, &max_param);

		values = (double*) malloc(sizeof(double) * max_param * 4);
		got = (double*) malloc(2 * sizeof(double) * max_param * 4);
		original = (double*) malloc(sizeof(double) * max_param * 4);
		n_doubles(values, max_param * 4);

		/* Read initial values */
		for (unsigned j = 0; j < max_param; j++) {
			glGetNamedProgramLocalParameterdvEXT(
				programs[0],
				target,
				j,
				&original[4 * j]);
		}

		if (use_display_list != GL_NONE)
			glNewList(list, use_display_list);

		/* Update all parameters */
		for (unsigned j = 0; j < max_param; j++) {
			if (j % 2) {
				glNamedProgramLocalParameter4dEXT(
					programs[0],
					target,
					j,
					values[4 * j], values[4 * j + 1],
					values[4 * j + 2], values[4 * j + 3]);
			} else {
				glNamedProgramLocalParameter4dvEXT(
					programs[0],
					target,
					j,
					&values[4 * j]);
			}
		}

		if (use_display_list != GL_NONE) {
			glEndList(list);
		}

		/* Re-read values */
		for (unsigned j = 0; j < max_param; j++) {
			glGetNamedProgramLocalParameterdvEXT(
				programs[0],
				target,
				j,
				&got[4 * j]);

			glGetProgramLocalParameterdvARB(
				target,
				j,
				&got[4 * (max_param + j)]);
		}

		if (use_display_list == GL_COMPILE) {
			/* Values shouldn't have been modified yet */
			pass = memcmp(original, got, sizeof(double) * max_param * 4) == 0 && pass;
			/* Execute list and re-read values */
			glCallList(list);
			for (unsigned j = 0; j < max_param; j++) {
				glGetNamedProgramLocalParameterdvEXT(
					programs[0],
					target,
					j,
					&got[4 * j]);
			}
		}

		/* Check that programs[1] values have been modified */
		pass = memcmp(values, got, sizeof(double) * max_param * 4) == 0 && pass;
		/* Check that programs[0] values have been untouched */
		pass = memcmp(original, &got[4 * max_param], sizeof(double) * max_param * 4) == 0 && pass;

		free(values);
		free(got);
		free(original);
		glDeleteProgramsARB(2, programs);
	}
	return pass && piglit_check_gl_error(GL_NO_ERROR) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_GetNamedProgramivEXT(void* data)
{
	bool pass = true;
	GLenum target = (GLenum) (intptr_t) data;

	static const GLenum pnames[] = {
        	GL_PROGRAM_LENGTH_ARB,
        	GL_PROGRAM_FORMAT_ARB,
        	GL_PROGRAM_BINDING_ARB,
        	GL_PROGRAM_INSTRUCTIONS_ARB,
        	GL_MAX_PROGRAM_INSTRUCTIONS_ARB,
        	GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB,
        	GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB,
        	GL_PROGRAM_TEMPORARIES_ARB,
        	GL_MAX_PROGRAM_TEMPORARIES_ARB,
        	GL_PROGRAM_NATIVE_TEMPORARIES_ARB,
        	GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB,
        	GL_PROGRAM_PARAMETERS_ARB,
        	GL_MAX_PROGRAM_PARAMETERS_ARB,
        	GL_PROGRAM_NATIVE_PARAMETERS_ARB,
        	GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB,
        	GL_PROGRAM_ATTRIBS_ARB,
        	GL_MAX_PROGRAM_ATTRIBS_ARB,
        	GL_PROGRAM_NATIVE_ATTRIBS_ARB,
        	GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB,
        	GL_PROGRAM_ADDRESS_REGISTERS_ARB,
        	GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB,
        	GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB,
        	GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB,
        	GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB,
        	GL_MAX_PROGRAM_ENV_PARAMETERS_ARB,
        	GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB,
	};

	static const char* vp_code[] = {
		"!!ARBvp1.0\n"
		"MOV result.position, {0, 0, 1, 0};\n"
		"END",

		"!!ARBvp1.0\n"
		"PARAM mvp[4] = { state.matrix.mvp };\n"
		"DP4 result.position.x, mvp[0], vertex.attrib[0];\n"
		"DP4 result.position.y, mvp[1], vertex.attrib[0];\n"
		"DP4 result.position.z, mvp[2], vertex.attrib[0];\n"
		"DP4 result.position.w, mvp[3], vertex.attrib[0];\n"
		"MOV result.color, {0, 0, 1, 0};\n"
		"END"
	};

	static const char* fp_code[] = {
		"!!ARBfp1.0\n"
		"MOV result.color, fragment.color;\n"
		"END",

		"!!ARBfp1.0\n"
		"TEMP	R0;\n"
		"ADD	R0, {0.5}.r, fragment.color;\n"
		"ABS	result.color, R0;\n"
		"END"
	};

	const char** code = (target == GL_VERTEX_PROGRAM_ARB) ? vp_code : fp_code;

	GLuint programs[2];
	glGenProgramsARB(2, programs);

	for (unsigned i = 0; i < ARRAY_SIZE(programs); i++) {
		glNamedProgramStringEXT(
		 	programs[i],
			target,
			GL_PROGRAM_FORMAT_ASCII_ARB,
			strlen(code[i]),
			code[i]);
	}

	for (unsigned i = 0; i < ARRAY_SIZE(pnames); i++) {
		int ref_value, got;
		GLenum pname = pnames[i];

		/* Read reference value */
		glBindProgramARB(target,
				 programs[1]);
		glGetProgramivARB(target,
				  pname,
				  &ref_value);
		/* Bind a different program */
		glBindProgramARB(target,
				 programs[0]);
		/* Verify glGetNamedProgramivEXT returns the same value */
		glGetNamedProgramivEXT(programs[1],
				       target,
				       pname,
				       &got);

		if (pname == GL_PROGRAM_BINDING_ARB) {
			pass = got != ref_value && pass;
		} else {
			pass = got == ref_value && pass;
		}
	}

	glDeleteProgramsARB(2, programs);

	return pass && piglit_check_gl_error(GL_NO_ERROR) ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	enum piglit_result result;

	piglit_require_extension("GL_EXT_direct_state_access");
	piglit_require_extension("GL_ARB_vertex_program");

	struct piglit_subtest tests[] = {
		{
			"NamedProgramStringEXT Vertex",
			NULL,
			test_NamedProgramStringEXT,
			(void*) GL_VERTEX_PROGRAM_ARB
		},
		{
			"NamedProgramStringEXT Fragment",
			NULL,
			test_NamedProgramStringEXT,
			(void*) GL_FRAGMENT_PROGRAM_ARB
		},
		{
			"NamedProgramLocalParameter4fEXT",
			NULL,
			test_NamedProgramLocalParameter4fEXT
		},
		{
			"NamedProgramLocalParameter4dEXT",
			NULL,
			test_NamedProgramLocalParameter4dEXT
		},
		{
			"GetNamedProgramivEXT Vertex",
			NULL,
			test_GetNamedProgramivEXT,
			(void*) GL_VERTEX_PROGRAM_ARB
		},
		{
			"GetNamedProgramivEXT Fragment",
			NULL,
			test_GetNamedProgramivEXT,
			(void*) GL_FRAGMENT_PROGRAM_ARB
		},
		{
			NULL
		}
	};


	result = piglit_run_selected_subtests(tests, NULL, 0, PIGLIT_PASS);

	list = glGenLists(1);

	/* Re-run the same test but using display list GL_COMPILE */
	for (unsigned i = 0; tests[i].name; i++) {
		char* test_name_display_list;
		asprintf(&test_name_display_list, "%s + display list GL_COMPILE", tests[i].name);
		tests[i].name = test_name_display_list;
	}
	use_display_list = GL_COMPILE;
	result = piglit_run_selected_subtests(tests, NULL, 0, result);

	/* Re-run the same test but using display list GL_COMPILE_AND_EXECUTE */
	for (unsigned i = 0; tests[i].name; i++) {
		char* test_name_display_list;
		asprintf(&test_name_display_list, "%s_AND_EXECUTE", tests[i].name);
		tests[i].name = test_name_display_list;
	}
	use_display_list = GL_COMPILE_AND_EXECUTE;
	result = piglit_run_selected_subtests(tests, NULL, 0, result);

	glDeleteLists(list, 1);

	piglit_report_result(result);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}
