/*
 * Copyright © 2017 Miklós Máté
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

/** Paragraph 3 of the Errors section:
 *
 * The error INVALID_OPERATION is generated if EndFragmentShaderATI,
 * PassTexCoordATI, SampleMapATI, ColorFragmentOp[1..3]ATI, or
 * AlphaFragmentOp[1..3]ATI is specified outside a
 * Begin/EndFragmentShaderATI pair.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_ATI_fragment_shader");

	glEndFragmentShaderATI();
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glPassTexCoordATI(GL_REG_0_ATI, GL_TEXTURE0_ARB, GL_SWIZZLE_STR_ATI);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glSampleMapATI(GL_REG_0_ATI, GL_TEXTURE1_ARB, GL_SWIZZLE_STR_ATI);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glColorFragmentOp1ATI(GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE,
			GL_REG_1_ATI, GL_NONE, GL_NONE);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glColorFragmentOp2ATI(GL_ADD_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE,
			GL_REG_1_ATI, GL_NONE, GL_NONE,
			GL_REG_2_ATI, GL_NONE, GL_NONE);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glColorFragmentOp3ATI(GL_MAD_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE,
			GL_REG_1_ATI, GL_NONE, GL_NONE,
			GL_REG_2_ATI, GL_NONE, GL_NONE,
			GL_REG_3_ATI, GL_NONE, GL_NONE);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glAlphaFragmentOp1ATI(GL_MOV_ATI, GL_REG_0_ATI, GL_NONE,
			GL_REG_1_ATI, GL_NONE, GL_NONE);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glAlphaFragmentOp2ATI(GL_ADD_ATI, GL_REG_0_ATI, GL_NONE,
			GL_REG_1_ATI, GL_NONE, GL_NONE,
			GL_REG_2_ATI, GL_NONE, GL_NONE);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glAlphaFragmentOp3ATI(GL_MAD_ATI, GL_REG_0_ATI, GL_NONE,
			GL_REG_1_ATI, GL_NONE, GL_NONE,
			GL_REG_2_ATI, GL_NONE, GL_NONE,
			GL_REG_3_ATI, GL_NONE, GL_NONE);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
