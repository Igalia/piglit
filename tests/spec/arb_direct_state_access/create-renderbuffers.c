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

/** @file create-renderbuffers.c
 *
 * Tests glCreateRenderbuffers to see if it behaves in the expected way, throwing
 * the correct errors, etc.
 *
 * From OpenGL 4.5, section 9.2.4 "Renderbuffer Objects", page 297:
 *
 * "void CreateRenderbuffers( sizei n, uint *renderbuffers );
 *
 * CreateRenderbuffers returns n previously unused renderbuffer names in
 * renderbuffers, each representing a new renderbuffer object which is a state
 * vector comprising all the state and with the initial values listed in table
 * 23.27. The state of each renderbuffer object is as if a name returned from
 * GenRenderbuffers had been bound to the RENDERBUFFER target, except that any
 * existing binding to RENDERBUFFER is not affected.
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
	piglit_require_extension("GL_ARB_framebuffer_object");
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	GLchar label[11];
	GLsizei length;
	GLuint ids[10];
	GLint param;

	/* Throw some invalid inputs at glCreateRenderbuffers */

	/* n is negative */
	glCreateRenderbuffers(-1, ids);
	SUBTEST(GL_INVALID_VALUE, pass, "n < 0");

	/* Throw some valid inputs at glCreateRenderbuffers. */

	/* n is zero */
	glCreateRenderbuffers(0, NULL);
	SUBTEST(GL_NO_ERROR, pass, "n == 0");

	/* n is more than 1 */
	glCreateRenderbuffers(10, ids);
	SUBTEST(GL_NO_ERROR, pass, "n > 1");

	/* test the default state of dsa-created render buffer objects */
	SUBTESTCONDITION(glIsRenderbuffer(ids[2]), pass, "IsRenderbuffer()");

	glBindRenderbuffer(GL_RENDERBUFFER, ids[2]);
	piglit_check_gl_error(GL_NO_ERROR);

	glGetRenderbufferParameteriv(GL_RENDERBUFFER,
				     GL_RENDERBUFFER_WIDTH, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == 0, pass,
			 "default width(%d) == 0", param);

	glGetRenderbufferParameteriv(GL_RENDERBUFFER,
				     GL_RENDERBUFFER_HEIGHT, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == 0, pass,
			 "default height(%d) == 0", param);

	glGetRenderbufferParameteriv(GL_RENDERBUFFER,
				     GL_RENDERBUFFER_INTERNAL_FORMAT, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == GL_RGBA, pass,
			 "default internal format == RGBA");

	glGetRenderbufferParameteriv(GL_RENDERBUFFER,
				     GL_RENDERBUFFER_RED_SIZE, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == 0, pass,
			 "default red size(%d) == 0", param);

	glGetRenderbufferParameteriv(GL_RENDERBUFFER,
				     GL_RENDERBUFFER_GREEN_SIZE, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == 0, pass,
			 "default green size(%d) == 0", param);

	glGetRenderbufferParameteriv(GL_RENDERBUFFER,
				     GL_RENDERBUFFER_BLUE_SIZE, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == 0, pass,
			 "default blue size(%d) == 0", param);

	glGetRenderbufferParameteriv(GL_RENDERBUFFER,
				     GL_RENDERBUFFER_ALPHA_SIZE, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == 0, pass,
			 "default alpha size(%d) == 0", param);

	glGetRenderbufferParameteriv(GL_RENDERBUFFER,
				     GL_RENDERBUFFER_DEPTH_SIZE, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == 0, pass,
			 "default depth size(%d) == 0", param);

	glGetRenderbufferParameteriv(GL_RENDERBUFFER,
				     GL_RENDERBUFFER_STENCIL_SIZE, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == 0, pass,
			 "default stencil size(%d) == 0", param);

	glGetRenderbufferParameteriv(GL_RENDERBUFFER,
				     GL_RENDERBUFFER_SAMPLES, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == 0, pass,
			 "default no. of samples(%d) == 0", param);

	glGetObjectLabel(GL_RENDERBUFFER, ids[2], 11, &length, label);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(length == 0, pass,
			 "default label size(%d) == 0", length);

	/* clean up */
	glDeleteRenderbuffers(10, ids);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
