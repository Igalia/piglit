/*
 * Copyright 2015 Intel Corporation
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

/** @file create-samplers.c
 *
 * Tests glCreateSamplers to see if it behaves in the expected way,
 * throwing the correct errors, etc.
 *
 * From OpenGL 4.5, section 8.2 "Sampler Objects", page 173:
 *
 * "void CreateSamplers( sizei n, uint *samplers );
 *
 * CreateSamplers returns n previously unused sampler names in samplers, each
 * representing a new sampler object which is a state vector comprising all
 * the state and with the same initial values listed in table 23.18.
 *
 * Errors
 * An INVALID_VALUE error is generated if n is negative."
 */

#include "piglit-util-gl.h"
#include "dsa-utils.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
		PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_direct_state_access");
	piglit_require_extension("GL_ARB_sampler_objects");
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	GLfloat bc[4], paramf;
	GLchar label[11];
	GLsizei length;
	GLuint ids[10];
	GLint param;

	/* Throw some invalid inputs at glCreateSamplers */

	/* n is negative */
	glCreateSamplers(-1, ids);
	SUBTEST(GL_INVALID_VALUE, pass, "n < 0");

	/* Throw some valid inputs at glCreateSamplers. */

	/* n is zero */
	glCreateSamplers(0, NULL);
	SUBTEST(GL_NO_ERROR, pass, "n == 0");

	/* n is more than 1 */
	glCreateSamplers(10, ids);
	SUBTEST(GL_NO_ERROR, pass, "n > 1");

	/* test the default state of dsa-created program pipeline objects */
	SUBTESTCONDITION(glIsSampler(ids[2]), pass,
			 "IsSampler()");

	glGetSamplerParameterfv(ids[2], GL_TEXTURE_BORDER_COLOR, bc);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(bc[0] == 0.0 && bc[1] == 0.0 && bc[2] == 0.0 &&
			bc[3] == 0.0, pass,
			 "default border color(%.02f, %.02f, %.02f, %.02f) "
			 "== 0.0, 0.0, 0.0, 0.0", bc[0], bc[1], bc[2], bc[3]);

	glGetSamplerParameteriv(ids[2], GL_TEXTURE_COMPARE_FUNC, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == GL_LEQUAL, pass,
			 "default compare function == LEQUAL");

	glGetSamplerParameteriv(ids[2], GL_TEXTURE_COMPARE_MODE, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == GL_NONE, pass,
			 "default compare function == NONE");

	glGetSamplerParameterfv(ids[2], GL_TEXTURE_LOD_BIAS, &paramf);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(paramf == 0.0, pass,
			 "default LOD bias(%f) == 0.0", paramf);

	glGetSamplerParameteriv(ids[2], GL_TEXTURE_MAX_LOD, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == 1000, pass,
			 "default maximum LOD(%d) == 1000", param);

	glGetSamplerParameteriv(ids[2], GL_TEXTURE_MAG_FILTER, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == GL_LINEAR, pass,
			 "default mag filter == LINEAR");

	/* OpenGL core 4.5 specs says it depends if the texture is rectangular
	 * or not while the man page says it is GL_NEAREST_MIPMAP_LINEAR by
	 * default. Test for the latter.
	 */
	glGetSamplerParameteriv(ids[2], GL_TEXTURE_MIN_FILTER, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == GL_NEAREST_MIPMAP_LINEAR, pass,
			 "default minimum filter == NEAREST");

	glGetSamplerParameteriv(ids[2], GL_TEXTURE_MIN_LOD, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == -1000, pass,
			 "default minimum LOD(%d) == -1000", param);

	glGetObjectLabel(GL_SAMPLER, ids[2], 11, &length, label);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(length == 0, pass,
			 "default label size(%d) == 0", length);

	/* OpenGL core 4.5 specs says the following tests depend if the texture
	 * the sampler is bound to is rectangular or not. The man page says it
	 * is REPEAT by default. Test for the latter while waiting for Khronos
	 * to respond.
	 */
	glGetSamplerParameteriv(ids[2], GL_TEXTURE_WRAP_S, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == GL_REPEAT, pass, "default wrap s == REPEAT");
	glGetSamplerParameteriv(ids[2], GL_TEXTURE_WRAP_T, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == GL_REPEAT, pass, "default wrap t == REPEAT");
	glGetSamplerParameteriv(ids[2], GL_TEXTURE_WRAP_T, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == GL_REPEAT, pass, "default wrap r == REPEAT");

	/* clean up */
	glDeleteSamplers(10, ids);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
