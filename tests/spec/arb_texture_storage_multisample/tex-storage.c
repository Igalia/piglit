/*
 * Copyright © 2013 Chris Forbes
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
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static void
check_zero_texture(void)
{
	/* attempting to call TexStorage*Multisample on the zero texture
	 * must fail with INVALID_OPERATION
	 */

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
				  4, GL_RGBA8, 64, 64, GL_TRUE);

	piglit_report_subtest_result(
		piglit_check_gl_error(GL_INVALID_OPERATION) ? PIGLIT_PASS : PIGLIT_FAIL,
		"zero-texture");
}

static void
check_unsized_format(void)
{
	/* attempting to call TexStorage*Multisample with an unsized internalformat
	 * must fail with INVALID_ENUM
	 */

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);

	glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
				  4, GL_RGBA, 64, 64, GL_TRUE);

	/* unsized formats may not be used with TexStorage* */
	piglit_report_subtest_result(
		piglit_check_gl_error(GL_INVALID_ENUM) ? PIGLIT_PASS : PIGLIT_FAIL,
		"unsized-format");
}

static void
check_immutable(void)
{
	GLuint tex;
	GLint param;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);

	/* specify storage for the texture, and mark it immutable-format */
	glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
				  4, GL_RGBA8, 64, 64, GL_TRUE);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_subtest_result(PIGLIT_FAIL, "immutable");
		return;
	}

	/* should now have TEXTURE_IMMUTABLE_FORMAT */
	glGetTexParameteriv(GL_TEXTURE_2D_MULTISAMPLE,
			    GL_TEXTURE_IMMUTABLE_FORMAT, &param);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("failed to fetch texture parameter TEXTURE_IMMUTABLE_FORMAT\n");
		piglit_report_subtest_result(PIGLIT_FAIL, "immutable");
		return;
	}

	if (param != GL_TRUE) {
		printf("expected TEXTURE_IMMUTABLE_FORMAT to be true, got %d\n", param);
		piglit_report_subtest_result(PIGLIT_FAIL, "immutable");
		return;
	}

	/* calling TexStorage2DMultisample again on the same texture should fail */
	glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
				  4, GL_RGBA8, 32, 32, GL_TRUE);

	if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
		printf("expected respecifying an immutable-format texture (with TexStorage*Multisample) to fail\n");
		piglit_report_subtest_result(PIGLIT_FAIL, "immutable");
		return;
	}

	/* calling TexImage2DMultisample should fail too */
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
				4, GL_RGBA8, 32, 32, GL_TRUE);

	if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
		printf("expected respecifying an immutable-format texture (with TexImage*Multisample) to fail\n");
		piglit_report_subtest_result(PIGLIT_FAIL, "immutable");
		return;
	}

	piglit_report_subtest_result(PIGLIT_PASS, "immutable");
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_texture_storage_multisample");

	check_zero_texture();
	check_immutable();
	check_unsized_format();

	piglit_report_result(PIGLIT_SKIP);
}
