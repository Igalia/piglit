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
 * Tests GL_ARB_texture_view  queries of new state added by this extension
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 15;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/**
 * make sure default intial state is correct for textureView
 * In OpenGL Core 4.3 spec see  table 23.15 for default values.
 */
static bool
query_default_state(void)
{
	bool pass = true;
	GLuint tex[2];
	GLint param;

	glGenTextures(2,tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex[0]);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 7, GL_R32F, 64, 64);
	glTextureView(tex[1], GL_TEXTURE_CUBE_MAP, tex[0], GL_RG16I, 2, 4, 0, 6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex[1]);
	glDeleteTextures(1, &tex[1]);
	/* tex[1] which is bound to GL_TEXTURE_CUBE_MAP is deleted */

	glGetTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_VIEW_MIN_LEVEL,
							&param);
	if (param != 0) {
		printf("bad default of min_level, expected 0 got %u\n", param);
		pass = false;
	}
	glGetTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_VIEW_NUM_LEVELS,
							&param);
	if (param != 0) {
		printf("bad default of num_levels, expected 0 got %u\n", param);
		pass = false;
	}
	glGetTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_VIEW_MIN_LAYER,
							&param);
	if (param != 0) {
		printf("bad default of min_layer, expected 0 got %u\n", param);
		pass = false;
	}
	glGetTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_VIEW_NUM_LAYERS,
							&param);
	if (param != 0) {
		printf("bad default of num_layers, expected 0 got %u\n", param);
		pass = false;
	}
	glGetTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_IMMUTABLE_FORMAT,
							&param);
	if (param != 0) {
		printf("bad default of immutable_format, expected 0 got %u\n",
		       param);
		pass = false;
	}
	glGetTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_IMMUTABLE_LEVELS,
							&param);
	if (param != 0) {
		printf("bad default of immutable_levels, expected 0 got %u\n",
		       param);
		pass = false;
	}
	glDeleteTextures(1, tex);

	return pass;
}

/**
 * This tests min_levels, num_levels, immutable_levels  and imutable_format.
 * note: see params test for more min/num  level query testing
 *
 * In OpenGL Core 4.3 spec see section 8.18:
 *    "The minlevel and minlayer parameters are relative to the view of
 *    origtexture. If numlayers or numlevels extend beyond origtexture, they
 *    are clamped to the maximum extent of the original texture.
 *    If the command is successful, the texture parameters in <texture> are
 *     updated as follows:
 *       - TEXTURE_IMMUTABLE_FORMAT is set to TRUE.
 *
 *       - TEXTURE_IMMUTABLE_LEVELS is set to the value of
 *         TEXTURE_IMMUTABLE_LEVELS from the original texture.
 *
 *       - TEXTURE_VIEW_MIN_LEVEL is set to <minlevel> plus the value of
 *         TEXTURE_VIEW_MIN_LEVEL from the original texture.
 *
 *       - TEXTURE_VIEW_NUM_LEVELS is set to the lesser of numlevels and the
 *         value of TEXTURE_VIEW_NUM_LEVELS for origtexture minus minlevels."
 * In the ARB_texture_view extension registry spec see this for chained views:
 *   "(3) Is it possible to create a texture view using an original texture
 *   which is itself a view? And if so, how are the level/layer values
 *   interpreted?
 *
 *   RESOLVED: It is legal. For example, let's say texture 1 is a 2D_ARRAY
 *   texture with 200 layers. It will have TEXTURE_VIEW_MIN_LAYER=0,
 *   TEXTURE_VIEW_NUM_LAYERS=200. Then we create texture 2 from texture 1 using
 *   <minlayer>=100, <numlayers>=100. It will have TEXTURE_VIEW_MIN_LAYER=100,
 *   TEXTURE_VIEW_NUM_LAYERS=100. Then we create texture 3 from texture 2 using
 *   <minlayer>=50, <numlayers>=50. It will have TEXTURE_VIEW_MIN_LAYER=150,
 *   TEXTURE_VIEW_NUM_LAYERS=50."
 */
static bool
query_levels_test(void)
{
	GLuint tex[2], viewtex, l;
	GLint param;
	bool pass = true;

	glGenTextures(2, tex);

	/* test the view causes immutable_format to be set */
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexStorage2D(GL_TEXTURE_2D, 6, GL_R32F, 16, 32);
	glTextureView(tex[1], GL_TEXTURE_2D, tex[0], GL_RG16F, 0, 1, 0, 1);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_IMMUTABLE_FORMAT, &param);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	if (param != GL_TRUE) {
		printf("bad query of immutable_format\n");
		pass = false;
	}
	glDeleteTextures(2,tex);

	/**
	 * test min_levels are additive using chained views
	 * test immutable_levels  get set correctly based on origtexture
	 * test num_levels are correct for chained views
	 */
	glGenTextures(2,tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex[0]);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 7, GL_R32F, 64, 64);
	glTextureView(tex[1], GL_TEXTURE_CUBE_MAP,tex[0], GL_RG16I, 2, 4, 0, 6);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBindTexture(GL_TEXTURE_CUBE_MAP, tex[1]);
	glGetTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_IMMUTABLE_LEVELS,
						&param);
	if (param != 7) {
		printf("bad query of immutable_levels, expected 7 got %u\n",
		       param);
		pass = false;
	}
	for (l = 0; l < 4; l++) {
		glGenTextures(1, &viewtex);
		glTextureView(viewtex, GL_TEXTURE_CUBE_MAP, tex[1], GL_RG16F,
			      l, 4, 0, 6);
		glBindTexture(GL_TEXTURE_CUBE_MAP, viewtex);
		glGetTexParameteriv(GL_TEXTURE_CUBE_MAP,
				    GL_TEXTURE_VIEW_MIN_LEVEL, &param);
		if (param != (2 + l)) {
			glDeleteTextures(1, &viewtex);
			printf("bad query of view_min_level expected %u got %u\n",
			       2+l, param);
			pass = false;
			break;
		}
		glGetTexParameteriv(GL_TEXTURE_CUBE_MAP,
				    GL_TEXTURE_IMMUTABLE_LEVELS, &param);
		if (param != 7) {
			glDeleteTextures(1, &viewtex);
			printf("query of immutable_levels not tracking orig, expected 7 got %u\n",
			       param);
			pass = false;
			break;
		}
		glGetTexParameteriv(GL_TEXTURE_CUBE_MAP,
				    GL_TEXTURE_VIEW_NUM_LEVELS,	&param);
		if (param != (4 - l)) {
			glDeleteTextures(1, &viewtex);
			printf("bad query of view_num_levels expected %u got %u\n",
			       4 - l, param);
			pass = false;
			break;
		}
		glDeleteTextures(1, &viewtex);
	}
	glDeleteTextures(2, tex);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	return pass;
}
/**
 *
 */
/**
 * This tests correct queries of min_layers, num_layers.
 * note: see params test for more min/num  layers query testing
 *
 * In OpenGL Core 4.3 spec see section 8.18:
 *    "The minlevel and minlayer parameters are relative to the view of
 *    origtexture. If numlayers or numlevels extend beyond origtexture, they
 *    are clamped to the maximum extent of the original texture.
 *    If the command is successful, the texture parameters in <texture> are
 *     updated as follows:
 *       - TEXTURE_VIEW_MIN_LAYER is set to <minlayer> plus the value of
 *         TEXTURE_VIEW_MIN_LAYER from the original texture.
 *
 *       - TEXTURE_VIEW_NUM_LAYERS is set to the lesser of numlayerss and the
 *         value of TEXTURE_VIEW_NUM_LAYERS for origtexture minus minlayers."
 */

static bool
query_layers_state(void)
{
	bool pass = true;
	GLuint tex[2], viewtex, l;
	GLint param;

	glGenTextures(2, tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex[0]);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 7, GL_RG16F, 64, 64, 10);
	glTextureView(tex[1], GL_TEXTURE_2D_ARRAY,tex[0], GL_RG16I, 0, 3, 1, 8);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	for (l = 0; l < 4; l++) {
		glGenTextures(1, &viewtex);
		glTextureView(viewtex, GL_TEXTURE_2D_ARRAY, tex[1], GL_RG16UI,
			      0, 4, l, 9);
		glBindTexture(GL_TEXTURE_2D_ARRAY, viewtex);

		/* test min_layers are additive using chained views */
		glGetTexParameteriv(GL_TEXTURE_2D_ARRAY,
				    GL_TEXTURE_VIEW_MIN_LAYER, &param);

		if (param != (1 + l)) {
			glDeleteTextures(1, &viewtex);
			printf("bad query of view_min_layer expected %u got %u\n",
			       1+l, param);
			pass = false;
			break;
		}
		/* test num_layers are correct for chained views */
		glGetTexParameteriv(GL_TEXTURE_2D_ARRAY,
				    GL_TEXTURE_VIEW_NUM_LAYERS,	&param);
		if (param != (8 - l)) {
			glDeleteTextures(1, &viewtex);
			printf("bad query of view_num_layers expected %u got %u\n",
			       8 - l, param);
			pass = false;
			break;
		}

		glDeleteTextures(1, &viewtex);
	}
	glDeleteTextures(2, tex);

	return pass;
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_ARB_texture_view");

	pass = query_levels_test();
	pass = query_layers_state() && pass;
	pass = query_default_state() && pass;

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
