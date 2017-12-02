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

/**
 * Tests rendering with GL_ATI_fragment_shader: when no texture is bound
 * - glPassTexCoordATI() should work as normal
 * - glSampleMapATI() should return all zeros
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const float color1[] = {0.2, 0.3, 0.8};
static const float texcoord[] = {0.2, 0.7, 0.4};
static const float black[] = {0.0, 0.0, 0.0};

enum {
	SHADER_TEXCOORD = 1,
	SHADER_TEX,
};

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	GLuint tex;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glTexCoord3fv(texcoord);

	glEnable(GL_FRAGMENT_SHADER_ATI);
	glBindFragmentShaderATI(SHADER_TEXCOORD);
	piglit_draw_rect(0, 0, piglit_width / 4, piglit_height);
	glBindFragmentShaderATI(SHADER_TEX);
	piglit_draw_rect(piglit_width / 4, 0, piglit_width / 4, piglit_height);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT,
		     (void*)color1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	piglit_draw_rect(2 * piglit_width / 4, 0,
			 piglit_width / 4, piglit_height);
	glEnable(GL_TEXTURE_2D);
	piglit_draw_rect(3 * piglit_width / 4, 0,
			 piglit_width / 4, piglit_height);
	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &tex);
	glDisable(GL_FRAGMENT_SHADER_ATI);

	pass = pass && piglit_probe_rect_rgb(0, 0, piglit_width / 4,
					     piglit_height, texcoord);
	pass = pass && piglit_probe_rect_rgb(piglit_width / 4, 0,
					     piglit_width / 4,
					     piglit_height, black);
	pass = pass && piglit_probe_rect_rgb(2 * piglit_width / 4, 0,
					     piglit_width / 4,
					     piglit_height, black);
	pass = pass && piglit_probe_rect_rgb(3 * piglit_width / 4, 0,
					     piglit_width / 4,
					     piglit_height, color1);

	piglit_present_results();

	pass &= piglit_check_gl_error(GL_NO_ERROR);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ATI_fragment_shader");

	glBindFragmentShaderATI(SHADER_TEXCOORD);
	glBeginFragmentShaderATI();
	glPassTexCoordATI(GL_REG_1_ATI, GL_TEXTURE0_ARB, GL_SWIZZLE_STR_ATI);
	glColorFragmentOp1ATI(GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE,
			GL_REG_1_ATI, GL_NONE, GL_NONE);
	glEndFragmentShaderATI();

	glBindFragmentShaderATI(SHADER_TEX);
	glBeginFragmentShaderATI();
	glSampleMapATI(GL_REG_0_ATI, GL_TEXTURE0_ARB, GL_SWIZZLE_STR_ATI);
	glColorFragmentOp1ATI(GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE,
			GL_REG_0_ATI, GL_NONE, GL_NONE);
	glEndFragmentShaderATI();

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}
