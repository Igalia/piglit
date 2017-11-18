/*
 * Copyright © 2017 Fabian Bieler
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
 * @file built-in-matrix-state.c:  Access uniform matrix derived state in GLSL
 *
 * Set coordiante transformation matrices with the OpenGL API and access them
 * and their derived uniforms in a GLSL shader.
 */

#include "piglit-util-gl.h"
#include "piglit-matrix.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#ifdef _WIN32
#define SRAND(x) srand(x)
#define DRAND() ((float)rand() / RAND_MAX)
#else
#define SRAND(x) srand48(x)
#define DRAND() drand48()
#endif

/**
 * Compute the transpose inverse of the 4x4 matrix \m and return the upper
 * left 3x3 block matrix in \out.
 */
static void
compute_normal_matrix(float out[9], const float m[16])
{
	float m_inv[16], m_inv_T[16];
	piglit_matrix_inverse(m_inv, m);
	piglit_matrix_transpose(m_inv_T, m_inv);
	out[0] = m_inv_T[0];
	out[1] = m_inv_T[1];
	out[2] = m_inv_T[2];
	out[3] = m_inv_T[4];
	out[4] = m_inv_T[5];
	out[5] = m_inv_T[6];
	out[6] = m_inv_T[8];
	out[7] = m_inv_T[9];
	out[8] = m_inv_T[10];
}

static const char *vs_text =
	"void main() {\n"
	"   gl_Position = gl_Vertex;\n"
	"}\n";
static const char *fs_mat4 =
	"void main() {\n"
	"   vec4 epsilon = vec4(1.0 / 256.0);\n"
	"   vec4 green = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"   vec4 red = vec4(1.0, 0.0, 0.0, 1.0);\n"
	"   mat4 a = %s;\n"
	"   mat4 b = mat4(%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, "
	"%f, %f, %f, %f);\n"
	"   bool pass = true;\n"
	"   pass = pass && all(lessThan(abs(a[0] - b[0]), epsilon));\n"
	"   pass = pass && all(lessThan(abs(a[1] - b[1]), epsilon));\n"
	"   pass = pass && all(lessThan(abs(a[2] - b[2]), epsilon));\n"
	"   pass = pass && all(lessThan(abs(a[3] - b[3]), epsilon));\n"
	"   gl_FragColor = pass ? green : red;\n"
	"}\n";
static const char *fs_mat3 =
	"void main() {\n"
	"   vec3 epsilon = vec3(1.0 / 256.0);\n"
	"   vec4 green = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"   vec4 red = vec4(1.0, 0.0, 0.0, 1.0);\n"
	"   mat3 a = %s;\n"
	"   mat3 b = mat3(%f, %f, %f, %f, %f, %f, %f, %f, %f);\n"
	"   bool pass = true;\n"
	"   pass = pass && all(lessThan(abs(a[0] - b[0]), epsilon));\n"
	"   pass = pass && all(lessThan(abs(a[1] - b[1]), epsilon));\n"
	"   pass = pass && all(lessThan(abs(a[2] - b[2]), epsilon));\n"
	"   gl_FragColor = pass ? green : red;\n"
	"}\n";
static const char *fs_float =
	"void main() {\n"
	"   float epsilon = (1.0 / 256.0);\n"
	"   vec4 green = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"   vec4 red = vec4(1.0, 0.0, 0.0, 1.0);\n"
	"   gl_FragColor = abs(%s - %f) < epsilon ? green : red;\n"
	"}\n";

/**
 * Check that the built-in shader uniform \name of type \type is equal to \m.
 *
 * Since we also test for derived state involving floating point computation
 * don't test for strict equality but rather only check if the uniform's
 * components are within and espilon of their expected values.
 */
static bool
check_shader_builtin(const GLenum type, const float *m, const char *name)
{
	char *fs_text;
	const float green[3] = {0.0, 1.0, 0.0};

	switch(type) {
	case GL_FLOAT:
		asprintf(&fs_text, fs_float, name, m[0]);
		break;
	case GL_FLOAT_MAT4:
		asprintf(&fs_text, fs_mat4, name, m[0], m[1], m[2], m[3],
			 m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11],
			 m[12], m[13], m[14], m[15]);
		break;
	case GL_FLOAT_MAT3:
		asprintf(&fs_text, fs_mat3, name, m[0], m[1], m[2], m[3],
			 m[4], m[5], m[6], m[7], m[8], m[9]);
		break;
	default:
		assert(0);
	}

	const GLuint program = piglit_build_simple_program(vs_text, fs_text);
	free(fs_text);
	glUseProgram(program);
	glDeleteProgram(program);
	glClear(GL_COLOR_BUFFER_BIT);
	piglit_draw_rect(-1, -1, 2, 2);

	if (piglit_probe_pixel_rgb_silent(piglit_width / 2, piglit_height / 2,
					  green, NULL))
		return true;
	printf("Failed uniform: '%s'.\n", name);
	return false;
}

/**
 * Load random 16 floats between 0 and 1 int matrix \pname
 * and return them in \m.
 */
static void
load_matrix(float m[16], const GLenum pname)
{
	glMatrixMode(pname);
	for (int i = 0; i < 16; ++i)
		m[i] = DRAND();
	glLoadMatrixf(m);
}

/**
 * Check that matrix \pname contains the values \m.
 * if \idx is zero or positive add it as and index to the matrix array.
 * Also check the matrix' transpose, inverse and transpose inverse.
 */
static bool
check_matrix_variants(const char *prefix, const float m[16], const int idx)
{
	bool pass = true;
	char *name, *name_T, *name_inv, *name_inv_T;
	float m_T[16], m_inv[16], m_inv_T[16];

	if (idx >= 0) {
		asprintf(&name, "%s[%d]", prefix, idx);
		asprintf(&name_T, "%sTranspose[%d]", prefix, idx);
		asprintf(&name_inv, "%sInverse[%d]", prefix, idx);
		asprintf(&name_inv_T, "%sInverseTranspose[%d]", prefix, idx);
	} else {
		asprintf(&name, "%s", prefix);
		asprintf(&name_T, "%sTranspose", prefix);
		asprintf(&name_inv, "%sInverse", prefix);
		asprintf(&name_inv_T, "%sInverseTranspose", prefix);
	}

	piglit_matrix_transpose(m_T, m);
	piglit_matrix_inverse(m_inv, m);
	piglit_matrix_transpose(m_inv_T, m_inv);

	pass = check_shader_builtin(GL_FLOAT_MAT4, m, name) && pass;
	pass = check_shader_builtin(GL_FLOAT_MAT4, m_T, name_T) && pass;
	pass = check_shader_builtin(GL_FLOAT_MAT4, m_inv, name_inv) && pass;
	pass = check_shader_builtin(GL_FLOAT_MAT4, m_inv_T, name_inv_T) &&
	       pass;

	free(name);
	free(name_T);
	free(name_inv);
	free(name_inv_T);

	return pass;
}

/**
 * Load random data in matrix \pname and check it by it's shader name \name
 * with (optional) index \idx.
 */
static bool
load_and_test_matrix(const char *name, const GLenum pname, const int idx)
{
	float mat[16];

	load_matrix(mat, pname);
	return check_matrix_variants(name, mat, idx);
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	/* Test modelview and projection matrices. */
	pass = load_and_test_matrix("gl_ModelViewMatrix", GL_MODELVIEW, -1) &&
	       pass;

	pass = load_and_test_matrix("gl_ProjectionMatrix", GL_PROJECTION, -1)
		&& pass;

	/* Test modelview-projection matrix. */
	float mvp[16], proj[16], mview[16];
	load_matrix(mview, GL_MODELVIEW);
	load_matrix(proj, GL_PROJECTION);
	piglit_matrix_mul_matrix(mvp, proj, mview);
	pass = check_matrix_variants("gl_ModelViewProjectionMatrix", mvp, -1)
		&& pass;

	/* Test texture matrices. */
	int max_texture_coords;
	glGetIntegerv(GL_MAX_TEXTURE_COORDS, &max_texture_coords);
	for (int t = 0; t < max_texture_coords; ++t) {
		glActiveTexture(GL_TEXTURE0 + t);
		pass = load_and_test_matrix("gl_TextureMatrix",
						    GL_TEXTURE, t) && pass;
	}


	/* Test normal matrix. */
	float norm[9] = {};
	load_matrix(mview, GL_MODELVIEW);
	compute_normal_matrix(norm, mview);
	pass = check_shader_builtin(GL_FLOAT_MAT3, norm, "gl_NormalMatrix")
		&& pass;

	/* Test normal scale factor.
	 * Page 49 (63 of the PDF) of the OpenGL 2.0 spec says:
	 *
	 *     "Rescale multiplies the transformed normals by a scale factor
	 *     [f] [...] If rescaling is disabled, then f = 1."
	 *
	 * I'm unsure if this affacts the shader's built-in uniform, but
	 * enable normal rescaling just in case.
	 */
	glEnable(GL_RESCALE_NORMAL);
	float ns = norm[6] * norm[6] + norm[7] * norm[7] + norm[8] * norm[8];
	ns = 1 / sqrt(ns);
	pass = check_shader_builtin(GL_FLOAT, &ns, "gl_NormalScale") && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	SRAND(17);
}
