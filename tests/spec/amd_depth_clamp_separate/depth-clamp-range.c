/*
 * Copyright © 2018 Intel Corporation
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
 *
 */

/** @file depth-clamp-range.c
 *
 * Tests that AMD_depth_clamp_separate enablement didn't break DepthRange
 * functionality, and properly uses the min/max selection.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;
	config.supports_gl_compat_version = 32;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE
			       | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

static GLuint program;
static GLint projection_loc;

static GLuint
make_program(void)
{
	static const char *vs_text =
		"#version 330 \n"
		"in vec4 vertex; \n"
		"uniform mat4 projection; \n"
		"void main() \n"
		"{ \n"
		"   gl_Position = projection * vertex; \n"
		"} \n";

	static const char *fs_text =
		"#version 330 \n"
		"void main() \n"
		"{ \n"
		"   gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0); \n"
		"} \n";

	GLuint program = piglit_build_simple_program(vs_text, fs_text);

	return program;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_AMD_depth_clamp_separate");
	program = make_program();
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	float white[3] = {1.0, 1.0, 1.0};
	float clear[3] = {0.0, 0.0, 0.0};

	glClearDepth(0.5);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glUseProgram(program);

	projection_loc = glGetUniformLocation(program, "projection");
	piglit_ortho_uniform(projection_loc, piglit_width, piglit_height);

	/* Keep in mind that the ortho projection flips near and far's signs,
	 * so 1.0 to quad()'s z maps to glDepthRange's near, and -1.0 maps to
	 * glDepthRange's far.
	 */

	/* Basic glDepthRange testing. */
	glDisable(GL_DEPTH_CLAMP_NEAR_AMD);
	glDisable(GL_DEPTH_CLAMP_FAR_AMD);
	glDepthRange(0, 1);
	piglit_draw_rect_z(0.5, 10, 10, 10, 10); /* .25 - drawn. */

	glDepthRange(1, 0);
	piglit_draw_rect_z(0.5, 10, 30, 10, 10); /* 0.75 - not drawn. */

	/* Now, test that near depth clamping works.*/
	glEnable(GL_DEPTH_CLAMP_NEAR_AMD);
	glDepthRange(0.25, 1.0);
	piglit_draw_rect_z(4, 30, 10, 10, 10); /* .25 - drawn. */

	glDisable(GL_DEPTH_CLAMP_NEAR_AMD);
	glEnable(GL_DEPTH_CLAMP_FAR_AMD);
	glDepthRange(0.75, 1.0);
	piglit_draw_rect_z(4, 30, 30, 10, 10); /* 0.75 - not drawn. */

	/* Test that far clamping works.*/
	glDepthRange(0.0, 0.25);
	piglit_draw_rect_z(-4, 50, 10, 10, 10); /* .25 - drawn. */

	glDepthRange(0.0, 0.75);
	piglit_draw_rect_z(-4, 50, 30, 10, 10); /* 0.75 - not drawn. */

	/* Now, flip near and far around and make sure that it's doing the
	 * min/max of near and far in the clamping.
	 */

	/* Test that near (max) clamping works. */
	glEnable(GL_DEPTH_CLAMP_NEAR_AMD);
	glDepthRange(0.25, 0.0);
	piglit_draw_rect_z(4, 70, 10, 10, 10); /* .25 - drawn. */

	glDisable(GL_DEPTH_CLAMP_NEAR_AMD);
	glEnable(GL_DEPTH_CLAMP_FAR_AMD);
	glDepthRange(0.75, 0.0);
	piglit_draw_rect_z(4, 70, 30, 10, 10); /* 0.75 - not drawn. */

	/* Now, test far (min) clamping works. */
	glDepthRange(1.0, 0.0);
	piglit_draw_rect_z(-4, 90, 10, 10, 10); /* 0.0 - drawn. */

	glDisable(GL_DEPTH_CLAMP_FAR_AMD);
	glDepthRange(1.0, 0.75);
	piglit_draw_rect_z(-4, 90, 30, 10, 10); /* 0.75 - drawn. */

	pass = piglit_probe_pixel_rgb(15, 15, white) && pass;
	pass = piglit_probe_pixel_rgb(15, 35, clear) && pass;
	pass = piglit_probe_pixel_rgb(35, 15, white) && pass;
	pass = piglit_probe_pixel_rgb(35, 35, clear) && pass;
	pass = piglit_probe_pixel_rgb(55, 15, white) && pass;
	pass = piglit_probe_pixel_rgb(55, 35, clear) && pass;
	pass = piglit_probe_pixel_rgb(75, 15, white) && pass;
	pass = piglit_probe_pixel_rgb(75, 35, clear) && pass;
	pass = piglit_probe_pixel_rgb(95, 15, white) && pass;
	pass = piglit_probe_pixel_rgb(95, 35, clear) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
