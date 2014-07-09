/* Copyright © 2013 Intel Corporation
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
 * Tests the TEXTURE_IMMUTABLE_LEVELS and TEXTURE_VIEW_NUM_LEVELS parameters.
 *
 * The ARB_texture_view spec says:
 *
 *     "If the command is successful, TEXTURE_IMMUTABLE_FORMAT becomes TRUE,
 *      TEXTURE_IMMUTABLE_LEVELS and TEXTURE_VIEW_NUM_LEVELS become <levels>."
 *
 * where <command> is glTexStorage?D.
 *
 * Test by calling glTexStorage*D with <levels> = 3, <width>, <height>, and
 * <depth> = 32; and then confirming that TEXTURE_IMMUTABLE_LEVELS was
 * correctly set to <levels>.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 12;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	GLuint tex[5];
	GLint level;
	GLint num_level;

	/* The GL ES 3.0 spec says:
	 *
	 *     "The [initial] value of TEXTURE_IMMUTABLE_LEVELS is 0."
	 */
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_IMMUTABLE_LEVELS, &level);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}
	if (level != 0) {
		printf("Expected 0 levels initially, but glGetTexParameteriv "
		       "returned %d for GL_TEXTURE_1D.\n", level);
		piglit_report_result(PIGLIT_FAIL);
	}

	glGenTextures(ARRAY_SIZE(tex), tex);

	glBindTexture(GL_TEXTURE_1D, tex[0]);
	glTexStorage1D(GL_TEXTURE_1D, 3, GL_RGBA8, 32);
	glGetTexParameteriv(GL_TEXTURE_1D, GL_TEXTURE_IMMUTABLE_LEVELS, &level);
	glGetTexParameteriv(GL_TEXTURE_1D, GL_TEXTURE_VIEW_NUM_LEVELS, &num_level);
	if (level != 3) {
		printf("Expected 3 levels, but glGetTexParameteriv returned "
		       "%d for GL_TEXTURE_1D.\n", level);
		piglit_report_result(PIGLIT_FAIL);
	} else if (level != num_level) {
		printf("Expected queries of TEXTURE_IMMUTABLE_LEVELS and "
		       "TEXTURE_VIEW_NUM_LEVELS to return identical results.");
		piglit_report_result(PIGLIT_FAIL);
	}

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, 32, 32);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_IMMUTABLE_LEVELS, &level);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_VIEW_NUM_LEVELS, &num_level);
	if (level != 3) {
		printf("Expected 3 levels, but glGetTexParameteriv returned "
		       "%d for GL_TEXTURE_2D.\n", level);
		piglit_report_result(PIGLIT_FAIL);
	} else if (level != num_level) {
		printf("Expected queries of TEXTURE_IMMUTABLE_LEVELS and "
		       "TEXTURE_VIEW_NUM_LEVELS to return identical results.");
		piglit_report_result(PIGLIT_FAIL);
	}

	glBindTexture(GL_TEXTURE_3D, tex[2]);
	glTexStorage3D(GL_TEXTURE_3D, 3, GL_RGBA8, 32, 32, 32);
	glGetTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_IMMUTABLE_LEVELS, &level);
	glGetTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_VIEW_NUM_LEVELS, &num_level);
	if (level != 3) {
		printf("Expected 3 levels, but glGetTexParameterfv returned "
		       "%d for GL_TEXTURE_3D.\n", level);
		piglit_report_result(PIGLIT_FAIL);
	} else if (level != num_level) {
		printf("Expected queries of TEXTURE_IMMUTABLE_LEVELS and "
		       "TEXTURE_VIEW_NUM_LEVELS to return identical results.");
		piglit_report_result(PIGLIT_FAIL);
	}

	glBindTexture(GL_TEXTURE_2D, tex[3]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_FLOAT, NULL);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_IMMUTABLE_LEVELS, &level);
	if (level != 0) {
		printf("Expected 0 levels, but glGetTexParameteriv returned "
		       "%d for GL_TEXTURE_2D.\n", level);
		piglit_report_result(PIGLIT_FAIL);
	}

	glBindTexture(GL_TEXTURE_3D, tex[4]);
	glTexImage2D(GL_TEXTURE_3D, 0, GL_RGBA, 32, 32, 32, GL_RGBA, GL_FLOAT, NULL);
	glGetTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_IMMUTABLE_LEVELS, &level);
	if (level != 0) {
		printf("Expected 0 levels, but glGetTexParameteriv returned "
		       "%d for GL_TEXTURE_3D.\n", level);
		piglit_report_result(PIGLIT_FAIL);
	}

	glDeleteTextures(5, tex);

	piglit_report_result(PIGLIT_PASS);
	return 0;
}

void
piglit_init(int argc, char *argv[])
{
	piglit_require_extension("GL_ARB_texture_view");
}
