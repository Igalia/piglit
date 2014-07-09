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
 * \file
 * This (arb_texture_view-targets) tests valid and invalid new TextureView
 * targets based on the original textures target.
 *
 * Section 8.18 (Texture Views) of OpenGL 4.3 Core says:
 *    "The new texture’s target must be compatible with the target of
 *     origtexture, as defined by table 8.20."
 *
 */

#include "piglit-util-gl.h"
#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 12;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "arb_texture_view-targets";

/**
 * Iterate through array of texture targets and check if call to TextureView
 * causes the gl error  "err"
 */
static bool
check_target_array(const GLenum err, const unsigned int numTargets,
		   const GLenum *targetArray, const GLenum format,
		   const GLuint tex, const GLuint levels)
{
	unsigned int i;
	bool pass = true;

	for (i = 0; i < numTargets; i++) {
		GLenum target;
		GLuint newTex, layers=1;
		target = targetArray[i];
		if (target == 0)
			continue;
		glGenTextures(1, &newTex);
		if (target== GL_TEXTURE_CUBE_MAP)
			layers = 6;
		else if (target == GL_TEXTURE_CUBE_MAP_ARRAY)
			layers = 12;

		glTextureView(newTex, target, tex, format, 0, levels, 0,
			      layers);
		glDeleteTextures(1, &newTex);
		if (!piglit_check_gl_error(err)) {
			pass = false;
			break;
		}
	}
	return pass;
}

/**
 * Do error-check tests for texture targets
 */
static bool
test_target_errors(GLenum target)
{
	GLint width = 64, height = 14, depth = 8;
	const GLsizei levels = 1;
	GLuint tex;
	enum piglit_result pass = true;
	GLenum legalTargets[4];
	unsigned int numTargets, numIllegalTargets;
	GLenum illegalTargets[] = {
		GL_TEXTURE_1D,
		GL_TEXTURE_2D,
		GL_TEXTURE_3D,
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_RECTANGLE,
		GL_TEXTURE_1D_ARRAY,
		GL_TEXTURE_2D_ARRAY,
		GL_TEXTURE_CUBE_MAP_ARRAY,
		GL_TEXTURE_2D_MULTISAMPLE,
		GL_TEXTURE_2D_MULTISAMPLE_ARRAY
	};
	
	if (piglit_is_extension_supported("GL_ARB_texture_storage_multisample"))
		numIllegalTargets = ARRAY_SIZE(illegalTargets);
	else
		numIllegalTargets =  ARRAY_SIZE(illegalTargets) -2;

	glGenTextures(1, &tex);   /* orig tex */
	glBindTexture(target, tex);

	switch (target) {
	case GL_TEXTURE_1D:
		glTexStorage1D(target, levels, GL_RGBA8, width);
		numTargets = update_valid_arrays(legalTargets, illegalTargets,
				    numIllegalTargets,
				    GL_TEXTURE_1D, GL_TEXTURE_1D_ARRAY, 0);
		break;
	case GL_TEXTURE_1D_ARRAY:
		glTexStorage2D(target, levels, GL_RGBA8, width, height);
		numTargets = update_valid_arrays(legalTargets, illegalTargets,
				    numIllegalTargets,
				    GL_TEXTURE_1D, GL_TEXTURE_1D_ARRAY, 0);
		break;
	case GL_TEXTURE_2D:
		glTexStorage2D(target, levels, GL_RGBA8, width, height);
		numTargets = update_valid_arrays(legalTargets, illegalTargets,
				    numIllegalTargets,
				    GL_TEXTURE_2D, GL_TEXTURE_2D_ARRAY, 0);
		break;
	case  GL_TEXTURE_RECTANGLE:
		glTexStorage2D(target, levels, GL_RGBA8, width, height);
		numTargets = update_valid_arrays(legalTargets, illegalTargets,
				    numIllegalTargets,
				    GL_TEXTURE_RECTANGLE, 0);
		break;
	case GL_TEXTURE_CUBE_MAP:
		width = height;
		glTexStorage2D(target, levels, GL_RGBA8, width, height);
		numTargets = update_valid_arrays(legalTargets, illegalTargets,
				    numIllegalTargets,
				    GL_TEXTURE_CUBE_MAP, GL_TEXTURE_2D,
				    GL_TEXTURE_2D_ARRAY,
				    GL_TEXTURE_CUBE_MAP_ARRAY, 0);
		break;
	case GL_TEXTURE_3D:
		glTexStorage3D(target, levels, GL_RGBA8, width, height, depth);
		numTargets = update_valid_arrays(legalTargets, illegalTargets,
				    numIllegalTargets,
				    GL_TEXTURE_3D, 0);
		break;
	case GL_TEXTURE_CUBE_MAP_ARRAY:
	case GL_TEXTURE_2D_ARRAY:
		height = width;
		glTexStorage3D(target, levels, GL_RGBA8, width, height, depth*6);
		numTargets = update_valid_arrays(legalTargets, illegalTargets,
				    numIllegalTargets,
				    GL_TEXTURE_CUBE_MAP, GL_TEXTURE_2D,
				    GL_TEXTURE_2D_ARRAY,
				    GL_TEXTURE_CUBE_MAP_ARRAY, 0);
		break;
	case GL_TEXTURE_2D_MULTISAMPLE:
		glTexStorage2DMultisample(target, 2, GL_RGBA8, width, height,
					  GL_TRUE);
		numTargets = update_valid_arrays(legalTargets, illegalTargets,
				    numIllegalTargets,
				    GL_TEXTURE_2D_MULTISAMPLE,
				    GL_TEXTURE_2D_MULTISAMPLE_ARRAY, 0);
		break;
	case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
		glTexStorage3DMultisample(target, 4, GL_RGBA8, width, height,
					  depth, GL_TRUE);
		numTargets = update_valid_arrays(legalTargets, illegalTargets,
				    numIllegalTargets,
				    GL_TEXTURE_2D_MULTISAMPLE,
				    GL_TEXTURE_2D_MULTISAMPLE_ARRAY, 0);
		break;
	default:
		assert(0);
		numTargets = 0;
		break;
	}

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("%s Found gl errors prior to testing glTextureView\n",
				   TestName);
		pass = false;
		goto err_out;
	}

	/* ensure TextureView of legal targets  works without gl errors */
	pass = pass && check_target_array(GL_NO_ERROR, numTargets, legalTargets,
					  GL_RG16, tex, levels);
	/* ensure TextureView  of illegal targets returns an error */
	pass = pass && check_target_array(GL_INVALID_OPERATION,
					  numIllegalTargets,
					  illegalTargets,
					  GL_RG16, tex, levels);
err_out:
	glDeleteTextures(1, &tex);

	return pass;
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

#define X(f, desc)							\
	do {								\
		const bool subtest_pass = (f);				\
		piglit_report_subtest_result(subtest_pass		\
					     ? PIGLIT_PASS : PIGLIT_FAIL, \
					     (desc));			\
		pass = pass && subtest_pass;				\
	} while (0)

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_ARB_texture_view");
	piglit_require_extension("GL_ARB_texture_cube_map_array");
	piglit_require_extension("GL_EXT_texture_array");
	piglit_require_extension("GL_ARB_texture_rectangle");

	if (piglit_get_gl_version() < 31)
	    piglit_require_extension("GL_ARB_texture_cube_map");

	X(test_target_errors(GL_TEXTURE_1D), "1D tex target validity");
	X(test_target_errors(GL_TEXTURE_2D), "2D tex target validity");
	X(test_target_errors(GL_TEXTURE_3D), "3D tex target validity");
	X(test_target_errors(GL_TEXTURE_CUBE_MAP),
		"Cubemap tex target validity");
	X(test_target_errors(GL_TEXTURE_RECTANGLE),
		"Rectangle tex target validity");
	X(test_target_errors(GL_TEXTURE_1D_ARRAY),
		"1D Array tex target validity");
	X(test_target_errors(GL_TEXTURE_2D_ARRAY),
		"2D Array tex target validity");
	X(test_target_errors(GL_TEXTURE_CUBE_MAP_ARRAY),
		"Cubemap Array tex target validity");
	if (piglit_is_extension_supported("GL_ARB_texture_storage_multisample")) {
		X(test_target_errors(GL_TEXTURE_2D_MULTISAMPLE),
		  "Multisample 2D tex target validity");
		X(test_target_errors(GL_TEXTURE_2D_MULTISAMPLE_ARRAY),
		  "Multisample 2D array tex target validity");
	}
#undef X
    pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
    piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
