/*
 * Copyright © 2013 LunarG, Inc.
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
 *
 * Author: Jon Ashburn <jon@lunarg.com>
 */

/**
 * Tests GL_ARB_texture_view  and validity of input parameters.
 * Use both valid and invalid parameters, although mostly invalid
 * parameters are tested  since other tests use valid parameters.
 * Only the parameters  "texture", "origtexture", "minlevel", "numlevels",
 * "minlayer", "numlayers"  are tested for validity  as per section 8.18 of
 * OpenGL 4.3 Core spec.
 * Tests formats.c and targets.c test the valid and invalid "format"  and
 * "target" input parameters respectively.
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 15;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "arb_texture_view-params";

/**
 * Test TextureView with various invalid arguments for "texture"
 * and "origtexture".
 * Errors as per OpenGL core 4.3 spec section 8.18:
 *     "An INVALID_VALUE error is generated if origtexture is not the
 *     name of a texture."
 *     "An INVALID_OPERATION error is generated if the value of
 *     TEXTURE_IMMUTABLE_FORMAT for origtexture is not TRUE."
 *     "An INVALID_VALUE error is generated if texture is zero."
 *     "An INVALID_OPERATION error is generated if texture is not a valid name
 *     returned by GenTextures, or if texture has already been bound and
 *     given a target."
 */
static bool
invalid_texture_param(void)
{
	bool pass = true;
	GLuint tex[2];

	/* invalid original texture param (origtexture) */
	glGenTextures(2, tex);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("%s Found gl errors prior to testing glTextureView\n",
				   TestName);
		pass = false;
		goto err_out;
	}
	/* origtexture IMMUTABLE_FORMAT == FALSE */
	glTextureView(tex[1], GL_TEXTURE_2D, tex[0], GL_R8, 0, 1, 0, 1);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glDeleteTextures(1, &tex[1]);
	glTexStorage2D(GL_TEXTURE_2D,2, GL_RGBA32F, 16, 16);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glGenTextures(1, &tex[1]);
	/* origtexture is not the name of a texture */
	glTextureView(tex[1], GL_TEXTURE_2D, 0, GL_RGBA32UI, 0, 1, 0, 1);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
	glDeleteTextures(1, &tex[1]);

	/* invalid  texture param (value  is 0)*/
	glTextureView(0, GL_TEXTURE_2D, tex[0], GL_RGBA32I, 0, 1, 0 ,1);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	glGenTextures(1, &tex[1]);
	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTextureView(tex[1], GL_TEXTURE_2D, tex[0], GL_RGBA32F, 0, 1, 0, 1);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glDeleteTextures(2, tex);

	/* invalid texture param (value not a valid name from GenTextures)*/
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexStorage2D(GL_TEXTURE_2D, 3, GL_RG16F, 16, 16);
	glTextureView(~tex[0], GL_TEXTURE_2D, tex[0], GL_RGBA8, 0,1,0,1);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	glDeleteTextures(1, tex);

	/* orig texture not immutable */
	glGenTextures(2, tex);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, 32, 32, 0, GL_RGB,
		     GL_SHORT, NULL);
	glTexImage2D(GL_TEXTURE_2D, 1, GL_RGB16, 16, 16, 0, GL_RGB,
		     GL_SHORT, NULL);
	glTextureView(tex[1], GL_TEXTURE_2D, tex[0], GL_RGBA32F, 0,1,0,1);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

err_out:
	glDeleteTextures(2, tex);
	return pass;
}

/**
 * Test TextureView with invalid arguments for "minlayer"
 * and "numlayers".
 * Errors as per OpenGL core 4.3 spec section 8.18:
 *    "An INVALID_VALUE error is generated if minlevel or minlayer are larger
 *    than the greatest level or layer, respectively, of origtexture."
 *    "An INVALID_VALUE error is generated if target is TEXTURE_1D,
 *    TEXTURE_2D, TEXTURE_3D, TEXTURE_RECTANGLE, or TEXTURE_2D_-
 *    MULTISAMPLE and numlayers does not equal 1."
 */
static bool
invalid_layer_param(GLenum target)
{
	bool pass = true;
	GLenum tar;
	GLuint tex[2];

	/* invalid minlayer param */
	glGenTextures(2, tex);
	glBindTexture(target, tex[0]);
	if (target == GL_TEXTURE_1D_ARRAY) {
		tar = GL_TEXTURE_1D;
		glTexStorage2D(target, 7, GL_RGB16I, 64, 4);
	} else if (target == GL_TEXTURE_2D_ARRAY) {
		tar = GL_TEXTURE_2D;
		glTexStorage3D(target, 7, GL_RGB16F, 64, 64, 4);
	} else {
		printf("%s: called with invalid target\n", TestName);
		return false;
	}
	glTextureView(tex[1], target, tex[0], GL_RGB16UI, 0, 7, 4, 2);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
	glDeleteTextures(1, &tex[1]);

	/* invalid numlayer param */
	glGenTextures(1, &tex[1]);
	glTextureView(tex[1], tar, tex[0], GL_RGB16I, 1, 5, 0, 4);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	glDeleteTextures(2, tex);
	return pass;
}

/**
 * Test TextureView with invalid argument for "minlevel"
 * Errors as per OpenGL core 4.3 spec section 8.18:
 *    "An INVALID_VALUE error is generated if minlevel or minlayer are larger
 *    than the greatest level or layer, respectively, of origtexture."
 */
static bool
invalid_level_param(void)
{
	bool pass = true;
	GLuint tex[2];

	/* invalid minlevel param */
	glGenTextures(2, tex);
	glBindTexture(GL_TEXTURE_1D, tex[0]);
	glTexStorage1D(GL_TEXTURE_1D, 6, GL_RGB16F, 32);
	glTextureView(tex[1], GL_TEXTURE_1D, tex[0], GL_RGB16UI, 7, 5, 1, 1);
	if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
		pass = false;
	}
	glDeleteTextures(2, tex);
	return pass;
}

/**
 * Test TextureView with "minlevel" range over legal values  and
 * with "numlevels"  clamped correctly.
 * As per OpenGL 4.3 Core spec section 8.18:
 *     "TEXTURE_VIEW_MIN_LEVEL is set to minlevel plus the value of
 *      TEXTURE_VIEW_MIN_LEVEL for origtexture."
 *
 *     "The minlevel and minlayer parameters are relative to the view
 *      of origtexture. If numlayers or numlevels extend beyond origtexture,
 *      they are clamped to the maximum extent of the original texture."
 */
static bool
levels_clamping(void)
{
	GLuint tex[2];
	GLint level, i;
	bool pass = true;
	const GLuint numLevels = 8;

	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_1D, tex[0]);
	glTexStorage1D(GL_TEXTURE_1D, numLevels-1, GL_RG16, 64);
	for (i = 0; i < numLevels - 1; i++) {
		glGenTextures(1, &tex[1]);
		glTextureView(tex[1], GL_TEXTURE_1D_ARRAY, tex[0], GL_RG16I,
			      i, numLevels-i, 0, 3);
		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			pass = false;
			break;
		}
		glBindTexture(GL_TEXTURE_1D_ARRAY, tex[1]);
		glGetTexParameteriv(GL_TEXTURE_1D_ARRAY,
				    GL_TEXTURE_VIEW_MIN_LEVEL, &level);
		if (level != i) {
			printf("failed at min_level=%d, queried view_min_level=%d\n",
					   i, level);
			pass = false;
			break;
		}
		glGetTexParameteriv(GL_TEXTURE_1D_ARRAY,
				    GL_TEXTURE_VIEW_NUM_LEVELS,	&level);
		if (level != (numLevels - 1 -i)) {
			printf("failed at min_level=%d, queried view_num_level=%d\n",
					   i, level);
			pass = false;
			break;
		}
		glDeleteTextures(1, &tex[1]);
		glBindTexture(GL_TEXTURE_1D, tex[0]);
	}

	glDeleteTextures(2, tex);
	return pass;
}

/**
 * Test TextureView with "minlayer" range over legal values  and
 * with "numlayers"  clamped correctly.
 * As per OpenGL 4.3 Core spec section 8.18:
 *     "TEXTURE_VIEW_MIN_LAYER is set to minlayer plus the value of
 *      TEXTURE_VIEW_MIN_LAYER for origtexture."
 *
 *     "The minlevel and minlayer parameters are relative to the view
 *      of origtexture. If numlayers or numlevels extend beyond origtexture,
 *      they are clamped to the maximum extent of the original texture."
 */
static bool
layers_clamping(void)
{
	bool pass = true;
	GLuint tex[2];
	GLint layer, i;

	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_1D_ARRAY, tex[0]);
	glTexStorage2D(GL_TEXTURE_1D_ARRAY, 5, GL_RGBA16F, 16, 4);
	for (i = 0; i < 4; i++) {
		glGenTextures(1, &tex[1]);
		glTextureView(tex[1], GL_TEXTURE_1D_ARRAY, tex[0], GL_RGBA16I,
			      0, 7, i, 5-i);
		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			pass = false;
			break;
		}
		glBindTexture(GL_TEXTURE_1D_ARRAY, tex[1]);
		glGetTexParameteriv(GL_TEXTURE_1D_ARRAY,
				    GL_TEXTURE_VIEW_MIN_LAYER, &layer);
		if (layer != i) {
			printf("failed at min_layer=%d, queried view_min_layer=%d\n",
					   i, layer);
			pass = false;
			break;
		}
		glGetTexParameteriv(GL_TEXTURE_1D_ARRAY,
				    GL_TEXTURE_VIEW_NUM_LAYERS,	&layer);
		if (layer != (4-i)) {
			printf("failed at min_layer=%d, queried view_num_layer=%d\n",
					   i, layer);
			pass = false;
			break;
		}
		glDeleteTextures(1, &tex[1]);
		glBindTexture(GL_TEXTURE_1D_ARRAY, tex[0]);
	}

	glDeleteTextures(2, tex);
	return pass;
}


enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

#define X(f, desc)						\
	do {							\
		const bool subtest_pass = (f);			\
		piglit_report_subtest_result(subtest_pass	\
					     ? PIGLIT_PASS : PIGLIT_FAIL, \
					     (desc));		\
		pass = pass && subtest_pass;			\
	} while (0)


void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_ARB_texture_view");
	piglit_require_extension("GL_EXT_texture_integer");
	piglit_require_extension("GL_ARB_texture_float");
	piglit_require_extension("GL_EXT_texture_array");

	X(invalid_texture_param(), "Invalid texture or origtexture");
	X(invalid_layer_param(GL_TEXTURE_1D_ARRAY), "Invalid layer param 1D");
	X(invalid_layer_param(GL_TEXTURE_2D_ARRAY), "Invalid layer param 2D");
	X(invalid_level_param(), "Invalid level param");
	X(levels_clamping(), "Minlevel range and numlevel clamp");
	X(layers_clamping(), "Minlayer range and numlayer clamp");

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
