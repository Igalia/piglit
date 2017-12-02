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

/** Paragraph 13 of the Errors section:
 *
 * The error INVALID_ENUM is generated if <argN> passed to
 * ColorFragmentOp[1..3]ATI or AlphaFragmentOp[1..3]ATI is not a valid
 * constant, interpolator, or register.
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

#define check_enum_error(en) if (!piglit_check_gl_error(GL_INVALID_ENUM)) \
	{ printf("Enum %s 0x%x not rejected\n", piglit_get_gl_enum_name(en), en); pass = false; }

bool
try_enum(unsigned e)
{
	bool pass = true;

	printf(" trying %s 0x%x\n", piglit_get_gl_enum_name(e), e);

	glBeginFragmentShaderATI();
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	glColorFragmentOp1ATI(GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE,
			e, GL_NONE, GL_NONE);
	check_enum_error(e);

	glAlphaFragmentOp1ATI(GL_MOV_ATI, GL_REG_0_ATI, GL_NONE,
			e, GL_NONE, GL_NONE);
	check_enum_error(e);
	glEndFragmentShaderATI();
	/* All instructions were invalid, so the shader should be empty,
	 * which is invalid
	 */
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* TODO check fragment ops with more arguments? */

	return pass;
}

/* Trying all possible enum values is overkill, only try ones that are
 * used in fragment shaders, thus being common user errors.
 * Note that some of them have the same numeric value.
 */
static const unsigned enums[] = {
	GL_MOV_ATI,
	GL_ADD_ATI,
	GL_MUL_ATI,
	GL_SUB_ATI,
	GL_DOT3_ATI,
	GL_DOT4_ATI,
	GL_MAD_ATI,
	GL_LERP_ATI,
	GL_CND_ATI,
	GL_CND0_ATI,
	GL_DOT2_ADD_ATI,
	GL_SWIZZLE_STR_ATI,
	GL_SWIZZLE_STQ_ATI,
	GL_SWIZZLE_STR_DR_ATI,
	GL_SWIZZLE_STQ_DQ_ATI,
	GL_SWIZZLE_STRQ_ATI,
	GL_SWIZZLE_STRQ_DQ_ATI,
	/*GL_RED_BIT_ATI,*/
	GL_GREEN_BIT_ATI,
	GL_BLUE_BIT_ATI,
	/*GL_2X_BIT_ATI,*/
	GL_4X_BIT_ATI,
	GL_8X_BIT_ATI,
	GL_HALF_BIT_ATI,
	GL_QUARTER_BIT_ATI,
	GL_EIGHTH_BIT_ATI,
	GL_SATURATE_BIT_ATI,
	GL_COMP_BIT_ATI,
	GL_NEGATE_BIT_ATI,
	GL_BIAS_BIT_ATI,
	GL_TEXTURE0_ARB,
	GL_TEXTURE1_ARB,
	GL_TEXTURE2_ARB,
	GL_TEXTURE3_ARB,
	GL_TEXTURE4_ARB,
	GL_TEXTURE5_ARB,
	GL_TEXTURE6_ARB,
	GL_TEXTURE7_ARB,
	/*GL_NONE,*/
	GL_RED,
	GL_GREEN,
	GL_BLUE,
	GL_ALPHA,
};

void
piglit_init(int argc, char **argv)
{
	unsigned i = 0;
	bool pass = true;

	piglit_require_extension("GL_ATI_fragment_shader");

	for (i=0; i<ARRAY_SIZE(enums); i++)
		if (!try_enum(enums[i]))
			pass = false;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
