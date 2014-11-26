/*
 * Copyright (c) 2010 VMware, Inc.
 * Copyright (c) 2014 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL VMWARE AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file
 * Adapted from teximage-errors.c to test ARB_direct_state_access by
 * Laura Ekstrand <laura@jlekstrand.net>.
 * Tests glTextureSubImage functions for invalid values, error reporting.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

/** Test texture size errors and subtexture position errors */
static bool
test_pos_and_sizes(void)
{
	bool pass = true;
	GLuint name;

	/* all of these should generate GL_INVALID_VALUE */
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, -16, 0, GL_RGBA, GL_FLOAT, NULL);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, -6, -5, 0, GL_RGBA, GL_FLOAT, NULL);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);

	glTexImage2D(GL_TEXTURE_2D, -2, GL_RGBA, 16, 16, 0, GL_RGBA, GL_FLOAT, NULL);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);

	glTexImage2D(GL_TEXTURE_2D, 2000, GL_RGBA, 16, 16, 0, GL_RGBA, GL_FLOAT, NULL);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);

	/* Setup dsa. */
	glCreateTextures(GL_TEXTURE_2D, 1, &name);
	glBindTextureUnit(0, name);	/* Since next command isn't bindless. */

	/* setup valid 2D texture for subsequent TexSubImage calls */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_FLOAT, NULL);

	glTextureSubImage2D(name, 0, 6, 6, 100, 100, GL_RGBA, GL_FLOAT, NULL);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);

	glTextureSubImage2D(name, 0, -6, -6, 10, 10, GL_RGBA, GL_FLOAT, NULL);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);

	glCopyTextureSubImage2D(name, 0, -6, -6, 2, 2, 10, 10);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);

	glCopyTextureSubImage2D(name, 0, 6, 6, 2, 2, 200, 200);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);

	/* mipmap level 1 doesn't exist */
	glTextureSubImage2D(name, 1, 0, 0, 8, 8, GL_RGBA, GL_FLOAT, NULL);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* mipmap level 2 doesn't exist */
	glCopyTextureSubImage2D(name, 2, 0, 0, 0, 0, 4, 4);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* To test 1D and 3D entry points, let's try using the wrong functions. */
	glTextureSubImage1D(name, 0, 0, 4, GL_RGBA, GL_FLOAT, NULL);
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);

	glTextureSubImage3D(name, 0, 0, 0, 0, 4, 4, 4, GL_RGBA, GL_FLOAT, NULL);
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);

	glCopyTextureSubImage1D(name, 0, 0, 0, 0, 4);
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);

	glCopyTextureSubImage3D(name, 0, 0, 0, 0, 0, 0, 4, 4);
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);

	return pass;
}

/* 
 * The texture parameter must be an existing texture object as returned
 * by glCreateTextures
 */
static bool
test_target_name(void)
{
	static const GLuint badname = 250;
	static const GLfloat fvec[2] = { 1.0, 1.0 };
	static const GLint ivec[2] = { -1, 1 };
	static const GLuint uvec[2] = { 1, 1 };
	bool pass = true;

	glTextureParameteri(badname, GL_TEXTURE_MAX_LEVEL, 4);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glTextureParameterf(badname, GL_TEXTURE_MAX_LEVEL, 4.0);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glTextureParameterfv(badname, GL_TEXTURE_MAX_LEVEL, fvec);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glTextureParameteriv(badname, GL_TEXTURE_MAX_LEVEL, ivec);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glTextureParameterIiv(badname, GL_TEXTURE_MAX_LEVEL, ivec);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glTextureParameterIuiv(badname, GL_TEXTURE_MAX_LEVEL, uvec);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
		"glTextureParameter: GL_INVALID_OPERATION on bad texture");
	return pass;
}

/* same as test_target_name, but for the getter functions */
static bool
test_getter_target_name(void)
{
	static const GLuint badname = 250;
	static GLfloat f = 1.0;
	static GLuint u = 1;
	static GLint i = -5;
	bool pass = true;

	glGetTextureParameterfv(badname, GL_TEXTURE_MAX_LEVEL, &f);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glGetTextureParameteriv(badname, GL_TEXTURE_MAX_LEVEL, &i);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glGetTextureParameterIiv(badname, GL_TEXTURE_MAX_LEVEL, &i);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glGetTextureParameterIuiv(badname, GL_TEXTURE_MAX_LEVEL, &u);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
		"glGetTextureParameter: GL_INVALID_OPERATION on bad texture");
	return pass;
}

static bool
test_getter_pname(void)
{
	static GLuint name;
	static GLfloat f = 1.0;
	static GLuint u = 1;
	static GLint i = -5;
	bool pass = true;

	/* Setup dsa. */
	glCreateTextures(GL_TEXTURE_2D, 1, &name);
	glBindTextureUnit(0, name);	/* Since next command isn't bindless. */

	glGetTextureParameterfv(name, GL_TEXTURE_1D, &f);
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);

	glGetTextureParameteriv(name, GL_TEXTURE_1D, &i);
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);

	glGetTextureParameterIiv(name, GL_TEXTURE_1D, &i);
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);

	glGetTextureParameterIuiv(name, GL_TEXTURE_1D, &u);
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
		"glGetTextureParameter: GL_INVALID_ENUM on bad pname");
	return pass;
}

static bool
test_pname(void)
{
	static GLuint name;
	const static GLfloat f = 1.0;
	const static GLuint u = 1;
	const static GLint i = -5;
	bool pass = true;

	/* Setup dsa. */
	glCreateTextures(GL_TEXTURE_2D, 1, &name);
	glBindTextureUnit(0, name);	/* Since next command isn't bindless. */

	glTextureParameterfv(name, GL_TEXTURE_1D, &f);
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);

	glTextureParameteriv(name, GL_TEXTURE_1D, &i);
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);

	glTextureParameterIiv(name, GL_TEXTURE_1D, &i);
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);

	glTextureParameterIuiv(name, GL_TEXTURE_1D, &u);
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
		"glTextureParameter: GL_INVALID_ENUM on bad pname");
	return pass;
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;
	pass &= test_pos_and_sizes();
	pass &= test_target_name();
	pass &= test_pname();
	pass &= test_getter_target_name();
	pass &= test_getter_pname();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_direct_state_access");
}
