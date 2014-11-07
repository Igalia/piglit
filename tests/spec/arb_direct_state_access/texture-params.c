/*
 * Copyright 2014 Intel Corporation
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

/** @file texture-params.c
 *
 * Trivially validates all the TextureParameter and GetTextureParameter entry
 * points by setting something, then getting it.
 */

#include "piglit-util-gl.h"
#include <math.h>

/* Copied from Mesa. TODO: How do other drivers do this? */
/* a close approximation: */
#define FLOAT_TO_INT(X)     ( (GLint) (2147483647.0 * (X)) )

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | 
		PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_direct_state_access");
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	int i;
	GLuint name;
	GLfloat scalarf = -100.0f;
	GLfloat paramf;
	GLfloat colorfv[4] = { 1.0f, 0.2f, 0.3f, 0.4f };
	GLfloat paramfv[4];
	GLint scalari = 5;
	GLint parami;
	GLenum scalare = GL_NEAREST;
	GLenum parame;
	GLint coloriv[4];
	GLint colorIiv[4] = {256, 50, -75, 100};
	GLint paramIiv[4];
	GLuint colorIuiv[4] = {256, 50, 75, 100};
	GLuint paramIuiv[4];
	GLenum swizzle[4] = {GL_RED, GL_BLUE, GL_RED, GL_BLUE};
	GLenum paramse[4];

	glCreateTextures(GL_TEXTURE_2D, 1, &name);

	/* f case */
	glTextureParameterf(name, GL_TEXTURE_MIN_LOD, scalarf);
	glGetTextureParameterfv(name, GL_TEXTURE_MIN_LOD, &paramf);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	if (paramf != scalarf) {
		printf("glTextureParameterf did not correctly set "
		       "GL_TEXTURE_MIN_LOD.\n\tValue returned by "
		       "glGetTextureParameterfv was %.2f (expected %.2f).\n",
		       paramf, scalarf);
		pass = false;
	}

	/* fv case */
	glTextureParameterfv(name, GL_TEXTURE_BORDER_COLOR, colorfv);
	glGetTextureParameterfv(name, GL_TEXTURE_BORDER_COLOR, &paramfv[0]);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	for (i = 0; i < 4; ++i) {
		if (paramfv[i] != colorfv[i]) {
			printf("glTextureParameterfv did not correctly set "
			       "GL_TEXTURE_BORDER_COLOR.\n\tValue %i "
			       "returned by "
			       "glGetTextureParameterfv was %.2f "
			       "(expected %.2f).\n",
			       i, paramfv[i], colorfv[i]);
			pass = false;
		}
	}

	/* i case */
	glTextureParameteri(name, GL_TEXTURE_MAX_LEVEL, scalari);
	glGetTextureParameteriv(name, GL_TEXTURE_MAX_LEVEL, &parami);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	if (parami != scalari) {
		printf("glTextureParameteri did not correctly set "
		       "GL_TEXTURE_MAX_LEVEL.\n\tValue returned by "
		       "glGetTextureParameteriv was %i "
		       "(expected %i).\n",
		       parami, scalari);
		pass = false;
	}

	/* i (enum) case */
	glTextureParameteri(name, GL_TEXTURE_MIN_FILTER, scalare);
	glGetTextureParameterIuiv(name, GL_TEXTURE_MIN_FILTER, &parame);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	if (parame != scalare) {
		printf("glTextureParameteri did not correctly set "
		       "GL_TEXTURE_MIN_FILTER.\n\tValue returned by "
		       "glGetTextureParameterIuiv was %s "
		       "(expected %s).\n",
		       piglit_get_gl_enum_name(parame),
		       piglit_get_gl_enum_name(scalare));
		pass = false;
	}

	/* iv case */
	for (i = 0; i < 4; ++i) {
		coloriv[i] = FLOAT_TO_INT(colorfv[i]);
	}
	glTextureParameteriv(name, GL_TEXTURE_BORDER_COLOR, coloriv);
	glGetTextureParameterfv(name, GL_TEXTURE_BORDER_COLOR, &paramfv[0]);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	for (i = 0; i < 4; ++i) {
		if (fabs(paramfv[i] - colorfv[i]) >= piglit_tolerance[i]) {
			printf("glTextureParameteriv did not correctly set "
			       "GL_TEXTURE_BORDER_COLOR.\n\tValue %i "
			       "returned by "
			       "glGetTextureParameterfv was %.2f "
			       "(expected %.2f).\n",
			       i, paramfv[i], colorfv[i]);
			pass = false;
		}
	}

	/* Iiv case */
	glTextureParameterIiv(name, GL_TEXTURE_BORDER_COLOR, colorIiv);
	glGetTextureParameterIiv(name, GL_TEXTURE_BORDER_COLOR, &paramIiv[0]);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	for (i = 0; i < 4; ++i) {
		if (paramIiv[i] != colorIiv[i]) {
			printf("glTextureParameterIiv did not correctly set "
			       "GL_TEXTURE_BORDER_COLOR.\n\tValue %i "
			       "returned by "
			       "glGetTextureParameterIiv was %i "
			       "(expected %i).\n",
			       i, paramIiv[i], colorIiv[i]);
			pass = false;
		}
	}

	/* Iuiv case */
	glTextureParameterIuiv(name, GL_TEXTURE_BORDER_COLOR, colorIuiv);
	glGetTextureParameterIuiv(name, GL_TEXTURE_BORDER_COLOR, 
				  &paramIuiv[0]);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	for (i = 0; i < 4; ++i) {
		if (paramIuiv[i] != colorIuiv[i]) {
			printf("glTextureParameterIuiv did not correctly set "
			       "GL_TEXTURE_BORDER_COLOR.\n\tValue %i "
			       "returned by "
			       "glGetTextureParameterIuiv was %i "
			       "(expected %i).\n",
			       i, paramIuiv[i], colorIuiv[i]);
			pass = false;
		}
	}

	/* Iuiv (enum) case */
	glTextureParameterIuiv(name, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
	glGetTextureParameterIuiv(name, GL_TEXTURE_SWIZZLE_RGBA, &paramse[0]);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	for (i = 0; i < 4; ++i) {
		if (paramse[i] != swizzle[i]) {
			printf("glTextureParameterIuiv did not correctly set "
			       "GL_TEXTURE_SWIZZLE_RGBA.\n\tValue %i "
			       "returned by "
			       "glGetTextureParameterIuiv was %s "
			       "(expected %s).\n",
			       i,
			       piglit_get_gl_enum_name(paramse[i]),
			       piglit_get_gl_enum_name(swizzle[i]));
			pass = false;
		}
	}


	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

