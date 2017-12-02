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
 * Tests rendering with GL_ATI_fragment_shader:
 * - precedence between ATI_fragment_shader, ARB_fragment_program, GLSL
 *   - ARB_fragment_program overrides ATI_fragment_shader
 *   - GLSL overrides both
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const float color1[] = {0.2, 0.3, 0.8};
static const float color2[] = {0.9, 0.8, 0.3};
static const float texcoord[] = {0.2, 0.7, 0.4};

static bool have_fp = false;
static bool have_fs = false;

static GLuint glslprog;

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glColor3fv(color1);
	glSecondaryColor3fvEXT(color2);
	glTexCoord3fv(texcoord);

	glEnable(GL_FRAGMENT_SHADER_ATI);
	piglit_draw_rect(0, 0, piglit_width / 4, piglit_height);
	if (have_fp) {
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
		piglit_draw_rect(piglit_width / 4, 0,
				 piglit_width / 4, piglit_height);
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
	}
	if (have_fs) {
		glUseProgram(glslprog);
		piglit_draw_rect(2 * piglit_width / 4, 0,
				 piglit_width / 4, piglit_height);
		glUseProgram(0);
	}
	if (have_fp && have_fs) {
		glUseProgram(glslprog);
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
		piglit_draw_rect(3 * piglit_width / 4, 0,
				 piglit_width / 4, piglit_height);
		glUseProgram(0);
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
	}
	glDisable(GL_FRAGMENT_SHADER_ATI);

	pass = pass && piglit_probe_rect_rgb(0, 0,
					     piglit_width / 4,
					     piglit_height, color2);
	if (have_fp)
		pass = pass && piglit_probe_rect_rgb(piglit_width / 4, 0,
						     piglit_width / 4,
						     piglit_height, texcoord);
	if (have_fs)
		pass = pass && piglit_probe_rect_rgb(2 * piglit_width / 4, 0,
						     piglit_width / 4,
						     piglit_height, color1);
	if (have_fp && have_fs)
		pass = pass && piglit_probe_rect_rgb(3 * piglit_width / 4, 0,
						     piglit_width / 4,
						     piglit_height, color1);

	piglit_present_results();

	pass &= piglit_check_gl_error(GL_NO_ERROR);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

const char *arbfp_str =
"!!ARBfp1.0\n"
"MOV result.color, fragment.texcoord[0];\n"
"END";

const char *glslfs_str =
"void main() { gl_FragColor = gl_Color; }";

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ATI_fragment_shader");

	/* create shaders */

	glBeginFragmentShaderATI();
	glColorFragmentOp1ATI(GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE,
			GL_SECONDARY_INTERPOLATOR_ATI, GL_NONE, GL_NONE);
	glEndFragmentShaderATI();

	if (piglit_is_extension_supported("GL_ARB_fragment_program")) {
		have_fp = true;
		piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, arbfp_str);
	}

	if (piglit_is_extension_supported("GL_ARB_fragment_shader")) {
		have_fs = true;
		glslprog = piglit_build_simple_program(NULL, glslfs_str);
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}
