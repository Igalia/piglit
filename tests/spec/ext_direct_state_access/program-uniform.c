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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
		PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static GLuint prog;
static GLuint progNonLinked;
static GLenum use_display_list = GL_NONE;
static GLuint list;

static GLuint setup_shaders(const char* vs_code, const char* fs_code)
{
    GLuint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_code);
    GLuint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_code);
    return piglit_link_simple_program(vs, fs);
}

static void
n_floats(float* m, int n)
{
	int i;
	for (i = 0; i < n; ++i) {
		m[i] = (float) (rand() % 1000);
	}
}

static void
n_ints(int* m, int n)
{
	int i;
	for (i = 0; i < n; ++i) {
		m[i] = rand() - (int)(RAND_MAX / 2);
	}
}

static enum piglit_result
test_ProgramUniformfEXT(void* data)
{
	const int locations[] = {
		glGetUniformLocation(prog, "f1"),
		glGetUniformLocation(prog, "f2"),
		glGetUniformLocation(prog, "f3"),
		glGetUniformLocation(prog, "f4")
	};
	float values[8];
	float got[4];
	int i, j;
	bool pass = true;

	n_floats(values, ARRAY_SIZE(values));

	for (i = 0; i < 2; i++) {
		if (use_display_list != GL_NONE)
			glNewList(list, use_display_list);

		/* Update float uniforms values */
		if (i == 0) {
			/* glProgramUniformNfEXT variant */
			glProgramUniform1fEXT(prog, locations[0],
				values[0]);
			glProgramUniform2fEXT(prog, locations[1],
				values[0], values[1]);
			glProgramUniform3fEXT(prog, locations[2],
				values[0], values[1], values[2]);
			glProgramUniform4fEXT(prog, locations[3],
				values[0], values[1], values[2], values[3]);
		} else {
			/* glProgramUniformNfvEXT variant */
			glProgramUniform1fvEXT(prog, locations[0], 1, &values[4]);
			glProgramUniform2fvEXT(prog, locations[1], 1, &values[4]);
			glProgramUniform3fvEXT(prog, locations[2], 1, &values[4]);
			glProgramUniform4fvEXT(prog, locations[3], 1, &values[4]);
		}

		if (use_display_list != GL_NONE)
			glEndList(list);
		if (use_display_list == GL_COMPILE)
			glCallList(list);

		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			return PIGLIT_FAIL;
		}

		/* Read back the values and verify */
		for (j = 0; j < 4; j++) {
			glGetUniformfv(prog, locations[j], got);
			if (memcmp(got, &values[4 * i], (j + 1) * sizeof(float)) != 0) {
				piglit_loge("glProgramUniform%dfEXT(..) failed\n", j + 1);
				return PIGLIT_FAIL;
			}
		}
	}

	/* The GL_EXT_direct_state_access spec says:
	 *
	 * If the program named by the program parameter is not created or has not
	 * been successfully linked, the error INVALID_OPERATION is generated.
	 *
    	 */
	glProgramUniform1fEXT(progNonLinked, locations[0], 0.0f);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniform2fEXT(progNonLinked, locations[0], 0.0f, 0.0f);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniform3fEXT(progNonLinked, locations[0], 0.0f, 0.0f, 0.0f);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniform4fEXT(progNonLinked, locations[0], 0.0f, 0.0f, 0.0f, 0.0f);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniform1fvEXT(progNonLinked, locations[0], 1, &values[4]);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniform2fvEXT(progNonLinked, locations[0], 1, &values[4]);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniform3fvEXT(progNonLinked, locations[0], 1, &values[4]);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniform4fvEXT(progNonLinked, locations[0], 1, &values[4]);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	if (!pass) {
		piglit_loge(
			"glProgramUniformNfEXT(..) should emit GL_INVALID_OPERATION "
			"if the program has not been successfully linked\n");
		return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}

static enum piglit_result
test_ProgramUniformiEXT(void* data)
{
	const int locations[] = {
		glGetUniformLocation(prog, "i1"),
		glGetUniformLocation(prog, "i2"),
		glGetUniformLocation(prog, "i3"),
		glGetUniformLocation(prog, "i4")
	};
	int values[8];
	int got[4];
	int i, j;
	bool pass = true;
	n_ints(values, ARRAY_SIZE(values));

	for (i = 0; i < 2; i++) {
		if (use_display_list != GL_NONE)
			glNewList(list, use_display_list);

		/* Update int uniforms values */
		if (i == 0) {
			/* glProgramUniformNfEXT variant */
			glProgramUniform1iEXT(prog, locations[0],
				values[0]);
			glProgramUniform2iEXT(prog, locations[1],
				values[0], values[1]);
			glProgramUniform3iEXT(prog, locations[2],
				values[0], values[1], values[2]);
			glProgramUniform4iEXT(prog, locations[3],
				values[0], values[1], values[2], values[3]);
		} else {
			/* glProgramUniformNfvEXT variant */
			glProgramUniform1ivEXT(prog, locations[0], 1, &values[4]);
			glProgramUniform2ivEXT(prog, locations[1], 1, &values[4]);
			glProgramUniform3ivEXT(prog, locations[2], 1, &values[4]);
			glProgramUniform4ivEXT(prog, locations[3], 1, &values[4]);
		}

		if (use_display_list != GL_NONE)
			glEndList(list);
		if (use_display_list == GL_COMPILE)
			glCallList(list);

		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			return PIGLIT_FAIL;
		}

		/* Read back the values and verify */
		for (j = 0; j < 4; j++) {
			glGetUniformiv(prog, locations[j], got);
			if (memcmp(got, &values[4 * i], (j + 1) * sizeof(int)) != 0) {
				piglit_loge("glProgramUniform%diEXT(..) failed\n", j + 1);
				return PIGLIT_FAIL;
			}
		}
	}

	/* The GL_EXT_direct_state_access spec says:
	 *
	 * If the program named by the program parameter is not created or has not
	 * been successfully linked, the error INVALID_OPERATION is generated.
	 *
    	 */
	glProgramUniform1iEXT(progNonLinked, locations[0], 0);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniform2iEXT(progNonLinked, locations[0], 0, 0);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniform3iEXT(progNonLinked, locations[0], 0, 0, 0);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniform4iEXT(progNonLinked, locations[0], 0, 0, 0, 0);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniform1ivEXT(progNonLinked, locations[0], 1, &values[4]);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniform2ivEXT(progNonLinked, locations[0], 1, &values[4]);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniform3ivEXT(progNonLinked, locations[0], 1, &values[4]);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniform4ivEXT(progNonLinked, locations[0], 1, &values[4]);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	if (!pass) {
		piglit_loge(
			"glProgramUniformNiEXT(..) should emit GL_INVALID_OPERATION "
			"if the program has not been successfully linked\n");
		return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}

static enum piglit_result
test_ProgramUniformMatrixfEXT(void* data)
{
	const int locations[] = {
		glGetUniformLocation(prog, "m2"),
		glGetUniformLocation(prog, "m3"),
		glGetUniformLocation(prog, "m4"),
		glGetUniformLocation(prog, "m23"),
		glGetUniformLocation(prog, "m24"),
		glGetUniformLocation(prog, "m32"),
		glGetUniformLocation(prog, "m34"),
		glGetUniformLocation(prog, "m42"),
		glGetUniformLocation(prog, "m33")
	};
	const int elem_count[] = {
		4,
		9,
		16,
		6,
		8,
		6,
		12,
		8,
		9
	};
	float values[16];
	float got[16];
	int i;
	bool pass = true;
	assert(ARRAY_SIZE(locations) == ARRAY_SIZE(elem_count));

	n_floats(values, ARRAY_SIZE(values));

	if (use_display_list != GL_NONE)
		glNewList(list, use_display_list);

	/* Update matrix uniforms values */
	glProgramUniformMatrix2fvEXT(prog, locations[0], 1, false, values);
	glProgramUniformMatrix3fvEXT(prog, locations[1], 1, false, values);
	glProgramUniformMatrix4fvEXT(prog, locations[2], 1, false, values);
	glProgramUniformMatrix2x3fvEXT(prog, locations[3], 1, false, values);
	glProgramUniformMatrix2x4fvEXT(prog, locations[4], 1, false, values);
	glProgramUniformMatrix3x2fvEXT(prog, locations[5], 1, false, values);
	glProgramUniformMatrix3x4fvEXT(prog, locations[6], 1, false, values);
	glProgramUniformMatrix4x2fvEXT(prog, locations[7], 1, false, values);
	glProgramUniformMatrix4x3fvEXT(prog, locations[8], 1, false, values);

	if (use_display_list != GL_NONE)
		glEndList(list);
	if (use_display_list == GL_COMPILE)
		glCallList(list);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		return PIGLIT_FAIL;
	}

	/* Read back the values and verify */
	for (i = 0; i < ARRAY_SIZE(locations); i++) {
		glGetUniformfv(prog, locations[i], got);
		if (memcmp(got, values, elem_count[i] * sizeof(float)) != 0) {
			piglit_loge("glProgramUniformXXXfvEXT(..) failed (test #%d)\n", i);
			return PIGLIT_FAIL;
		}
	}

	/* The GL_EXT_direct_state_access spec says:
	 *
	 * If the program named by the program parameter is not created or has not
	 * been successfully linked, the error INVALID_OPERATION is generated.
	 *
    	 */
	glProgramUniformMatrix2fvEXT(progNonLinked, locations[0], 1, false, values);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniformMatrix3fvEXT(progNonLinked, locations[0], 1, false, values);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniformMatrix4fvEXT(progNonLinked, locations[0], 1, false, values);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniformMatrix2x3fvEXT(progNonLinked, locations[0], 1, false, values);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniformMatrix2x4fvEXT(progNonLinked, locations[0], 1, false, values);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniformMatrix3x2fvEXT(progNonLinked, locations[0], 1, false, values);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniformMatrix3x4fvEXT(progNonLinked, locations[0], 1, false, values);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniformMatrix4x2fvEXT(progNonLinked, locations[0], 1, false, values);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glProgramUniformMatrix4x3fvEXT(progNonLinked, locations[0], 1, false, values);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	if (!pass) {
		piglit_loge(
			"glProgramUniformMatrixNfEXT(..) should emit GL_INVALID_OPERATION "
			"if the program has not been successfully linked\n");
		return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	int i;
	enum piglit_result result;
	const char vs_code[] =
		"#version 120\n"
		"uniform mat2 m2;\n"
		"uniform mat2x3 m23;\n"
		"uniform mat2x4 m24;\n"
		"uniform mat3 m3;\n"
		"uniform mat3x2 m32;\n"
		"uniform mat3x4 m34;\n"
		"uniform mat4 m4;\n"
		"uniform mat4x2 m42;\n"
		"uniform mat4x3 m43;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_Position = mat4(mat3(m2) * m3) * m4 * gl_Vertex;\n"
		"	gl_Position += (mat4(m23) * mat4(m24) * mat4(m32) *\n"
		"		       mat4(m34) * mat4(42) * mat4(43))[0]\n;"
		"}\n";

	const char fs_code[] =
		"#version 120\n"
		"uniform float f1;\n"
		"uniform vec2 f2;\n"
		"uniform vec3 f3;\n"
		"uniform vec4 f4;\n"
		"uniform int i1;\n"
		"uniform ivec2 i2;\n"
		"uniform ivec3 i3;\n"
		"uniform ivec4 i4;\n"
		"void main()\n"
		"{\n"
		"	gl_FragColor = vec4(f3, f1) * f4 + vec4(f2, f2);\n"
		"	if (i2.x + i3.y >= i4.z)\n"
		"		gl_FragColor.r = float(i1);\n"
		"}\n";

	piglit_require_extension("GL_EXT_direct_state_access");

	prog = setup_shaders(vs_code, fs_code);
	progNonLinked = setup_shaders(vs_code, "");

	struct piglit_subtest tests[] = {
		{
			"ProgramUniformfEXT",
			NULL,
			test_ProgramUniformfEXT
		},
		{
			"ProgramUniformiEXT",
			NULL,
			test_ProgramUniformiEXT
		},
		{
			"ProgramUniformMatrixfEXT",
			NULL,
			test_ProgramUniformMatrixfEXT
		},
		{
			NULL
		}
	};


	result = piglit_run_selected_subtests(tests, NULL, 0, PIGLIT_PASS);
	list = glGenLists(1);

	/* Re-run the same test but using display list GL_COMPILE */
	for (i = 0; tests[i].name; i++) {
		char* test_name_display_list;
		asprintf(&test_name_display_list, "%s + display list GL_COMPILE", tests[i].name);
		tests[i].name = test_name_display_list;
	}
	use_display_list = GL_COMPILE;
	result = piglit_run_selected_subtests(tests, NULL, 0, result);

	/* Re-run the same test but using display list GL_COMPILE_AND_EXECUTE */
	for (i = 0; tests[i].name; i++) {
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
