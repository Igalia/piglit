/**
 * Copyright © 2013 Intel Corporation
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
 * Test that when target is TEXTURE_RECTANGLE, the correct error messages are
 * generated when certain texture parameter values are specified.
 *
 * Section 3.8.8(Texture Parameters) of OpenGL 3.3 Core says:
 *
 *     "When target is TEXTURE_RECTANGLE, certain texture parameter values may
 *     not be specified. In this case, the error INVALID_ENUM is generated if
 *     the TEXTURE_WRAP_S, TEXTURE_WRAP_T, or TEXTURE_WRAP_R parameter is set
 *     to REPEAT or MIRRORED_REPEAT. The error INVALID_ENUM is generated if
 *     TEXTURE_MIN_FILTER is set to a value other than NEAREST or LINEAR (no
 *     mipmap filtering is permitted). The error INVALID_VALUE is generated if
 *     TEXTURE_BASE_LEVEL is set to any value other than zero."
 *
 * The GL_ATI_texture_mirror_repeat_once spec is silent on the topic of
 * rectangle textures, but GL_ARB_texture_mirror_clamp_to_edge says:
 *
 *     "The error INVALID_ENUM is generated when TexParameter* is called with
 *     a target of TEXTURE_RECTANGLE and the TEXTURE_WRAP_S, TEXTURE_WRAP_T,
 *     or TEXTURE_WRAP_R parameter is set to REPEAT, MIRRORED_REPEAT, or
 *     MIRROR_CLAMP_TO_EDGE."
 *
 * And GL_NV_texture_mirror_clamp says:
 *
 *     "Certain texture parameter values may not be specified for textures
 *     with a target of TEXTURE_RECTANGLE_NV.  The error INVALID_ENUM is
 *     generated if the target is TEXTURE_RECTANGLE_NV and the TEXTURE_WRAP_S,
 *     TEXTURE_WRAP_T, or TEXTURE_WRAP_R parameter is set to REPEAT,
 *     MIRRORED_REPEAT_IBM, MIRROR_CLAMP_EXT, MIRROR_CLAMP_TO_EDGE_EXT, and
 *     MIRROR_CLAMP_TO_BORDER_EXT."
 *
 * So also verify that the GL_MIRROR_CLAMP_* values also generate
 * GL_INVALID_ENUM.  Note that we don't need to check for the extensions for
 * this test.  If the extensions are not supported, the values should generate
 * GL_INVALID_ENUM anyway!
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;
	config.supports_gl_compat_version = 10;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	int i;
	static const GLenum invalidWrapParams[] = {
		GL_REPEAT,
		GL_MIRRORED_REPEAT,
		GL_MIRROR_CLAMP_EXT,
		GL_MIRROR_CLAMP_TO_BORDER_EXT,

		/* This has the same value as GL_MIRROR_CLAMP_TO_EDGE, but
		 * glext.h may not be new enough.  Just use the _EXT version.
		 */
		GL_MIRROR_CLAMP_TO_EDGE_EXT,
	};
	static const GLenum invalidFilterParams[] = {
		GL_NEAREST_MIPMAP_NEAREST,
		GL_NEAREST_MIPMAP_LINEAR,
		GL_LINEAR_MIPMAP_NEAREST,
		GL_LINEAR_MIPMAP_LINEAR
	};

	if (piglit_get_gl_version() < 33)
		piglit_require_extension("GL_ARB_texture_rectangle");

	/* "...the error INVALID_ENUM is generated if the TEXTURE_WRAP_S,
	 * TEXTURE_WRAP_T, or TEXTURE_WRAP_R parameter is set to REPEAT or
	 * MIRRORED_REPEAT."
	 */
	for(i = 0; i < ARRAY_SIZE(invalidWrapParams); i++) {
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S,
				invalidWrapParams[i]);
		pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;

		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T,
				invalidWrapParams[i]);
		pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;

		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_R,
				invalidWrapParams[i]);
		pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;
	}

	/* "The error INVALID_ENUM is generated if TEXTURE_MIN_FILTER is set
	 * to a value other than NEAREST or LINEAR."
	 */
	for (i = 0; i < ARRAY_SIZE(invalidFilterParams); i++) {
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER,
				invalidFilterParams[i]);
		pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;
	}

	/* "The error INVALID_VALUE is generated if TEXTURE_BASE_LEVEL is set
	 * to any value other than zero."
	 */
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_BASE_LEVEL, 37);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
