/*
 * (C) Copyright IBM Corporation 2004
 * All Rights Reserved.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \file stencil_twoside.c
 *
 * Simple test of GL_ATI_separate_stencil (or the OGL 2.0 equivalent)
 * functionality.
 *
 * Five squares (or six if stencil wrap is available) are drawn
 * with different stencil modes, but all should be rendered with the same
 * final color.
 */

#include "piglit-util-gl.h"

static int use20syntax = 1;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 650;
	config.window_height = 200;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_STENCIL;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLint max_stencil;
	GLint stencil_bits;
	unsigned i;
	float expected[4] = {0.5, 0.5, 0.5, 0.5};
	int w = piglit_width / (6 * 2 + 1);
	int h = w;
	int start_y = (piglit_height - h) / 2;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glGetIntegerv(GL_STENCIL_BITS, & stencil_bits);
	max_stencil = (1U << stencil_bits) - 1;
	printf("Stencil bits = %u, maximum stencil value = 0x%08x\n",
		stencil_bits, max_stencil);

	glClearStencil(1);
	glClearColor(0.2, 0.2, 0.8, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		 | GL_STENCIL_BUFFER_BIT);

	/* This is the "reference" square. */
	glDisable(GL_STENCIL_TEST);
	glColor3f(0.5, 0.5, 0.5);
	piglit_draw_rect(w * 1, start_y, w, h);

	glEnable(GL_STENCIL_TEST);

	/* Draw the first two squares using incr for the affected face
	 */

	/* 2nd square */
	if (use20syntax) {
		glStencilFuncSeparate(GL_FRONT, GL_ALWAYS, 0, ~0);
		glStencilFuncSeparate(GL_BACK, GL_ALWAYS, 0, ~0);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR);
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR);
	}
	else {
		glStencilFuncSeparateATI(GL_ALWAYS, GL_ALWAYS, 0, ~0);
		glStencilOpSeparateATI(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR);
		glStencilOpSeparateATI(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR);
	}

	glColor3f(0.9, 0.9, 0.9);
	for (i = 0 ; i < (max_stencil + 5) ; i++) {
		/* this should be front facing */
		piglit_draw_rect(w * 3, start_y, w, h);
	}

	/* stencil vals should be equal to max_stencil */
	glStencilFunc(GL_EQUAL, max_stencil, ~0);
	glColor3f(0.5, 0.5, 0.5);
	piglit_draw_rect(w * 3, start_y, w, h);

	/* 3rd square */
	if (use20syntax) {
		glStencilFuncSeparate(GL_FRONT, GL_ALWAYS, 0, ~0);
		glStencilFuncSeparate(GL_BACK, GL_ALWAYS, 0, ~0);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_DECR);
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_INCR);
	}
	else {
		glStencilFuncSeparateATI(GL_ALWAYS, GL_ALWAYS, 0, ~0);
		glStencilOpSeparateATI(GL_FRONT, GL_KEEP, GL_KEEP, GL_DECR);
		glStencilOpSeparateATI(GL_BACK, GL_KEEP, GL_KEEP, GL_INCR);
	}

	glColor3f(0.9, 0.9, 0.9);
	for (i = 0 ; i < (max_stencil + 5) ; i++) {
		/* this should be back facing */
		piglit_draw_rect(w * 5, start_y + h, w, -h);
	}

	/* stencil vals should be equal to max_stencil */
	glStencilFunc(GL_EQUAL, max_stencil, ~0);
	glColor3f(0.5, 0.5, 0.5);
	piglit_draw_rect(w * 5, start_y, w, h);

	/* 4th square */
	if (use20syntax) {
		glStencilFuncSeparate(GL_FRONT, GL_NEVER, 0, ~0);
		glStencilFuncSeparate(GL_BACK, GL_ALWAYS, 0, ~0);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_DECR);
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_INCR);
	}
	else {
		glStencilFuncSeparateATI(GL_NEVER, GL_ALWAYS, 0, ~0);
		glStencilOpSeparateATI(GL_FRONT, GL_KEEP, GL_KEEP, GL_DECR);
		glStencilOpSeparateATI(GL_BACK, GL_KEEP, GL_KEEP, GL_INCR);
	}

	glColor3f(0.9, 0.9, 0.9);
	for (i = 0 ; i < (max_stencil + 5) ; i++) {
		/* this should be back facing */
		piglit_draw_rect(w * 7, start_y + h, w, -h);
		/* this should be front facing */
		piglit_draw_rect(w * 7, start_y, w, h);
	}

	/* stencil vals should be equal to max_stencil */
	glStencilFunc(GL_EQUAL, max_stencil, ~0);
	glColor3f(0.5, 0.5, 0.5);
	piglit_draw_rect(w * 7, start_y, w, h);

	/* 5th square */
	if (use20syntax) {
		glStencilFuncSeparate(GL_FRONT, GL_ALWAYS, 0, ~0);
		glStencilFuncSeparate(GL_BACK, GL_ALWAYS, 0, ~0);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR);
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR);
	}
	else {
		glStencilFuncSeparateATI(GL_ALWAYS, GL_ALWAYS, 0, ~0);
		glStencilOpSeparateATI(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR);
		glStencilOpSeparateATI(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR);
	}

	glColor3f(0.9, 0.9, 0.9);
	for (i = 0 ; i < (max_stencil + 5) ; i++) {
		/* this should be back facing */
		piglit_draw_rect(w * 9, start_y + h, w, -h);
		/* this should be front facing */
		piglit_draw_rect(w * 9, start_y, w, h);
	}

	glStencilFunc(GL_EQUAL, 1, ~0);
	glColor3f(0.5, 0.5, 0.5);
	piglit_draw_rect(w * 9, start_y, w, h);

	/* 6th square */
	if (piglit_is_extension_supported("GL_EXT_stencil_wrap")) {
		if (use20syntax) {
			glStencilFuncSeparate(GL_FRONT, GL_ALWAYS, 0, ~0);
			glStencilFuncSeparate(GL_BACK, GL_ALWAYS, 0, ~0);
			glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_KEEP);
			glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
		}
		else {
			glStencilFuncSeparateATI(GL_ALWAYS, GL_ALWAYS, 0, ~0);
			glStencilOpSeparateATI(GL_FRONT, GL_KEEP, GL_KEEP, GL_KEEP);
			glStencilOpSeparateATI(GL_BACK, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
		}

		glColor3f(0.9, 0.9, 0.9);
		for (i = 0 ; i < (max_stencil + 5) ; i++) {
			/* this should be back facing */
			piglit_draw_rect(w * 11, start_y + h, w, -h);
			/* this should be front facing */
			piglit_draw_rect(w * 11, start_y, w, h);
		}

		glStencilFunc(GL_EQUAL, 260 - 255, ~0);
		glColor3f(0.5, 0.5, 0.5);
		piglit_draw_rect(w * 11, start_y, w, h);
	}

	pass = piglit_probe_pixel_rgb(w * 1.5, piglit_height / 2, expected) && pass;
	pass = piglit_probe_pixel_rgb(w * 3.5, piglit_height / 2, expected) && pass;
	pass = piglit_probe_pixel_rgb(w * 5.5, piglit_height / 2, expected) && pass;
	pass = piglit_probe_pixel_rgb(w * 7.5, piglit_height / 2, expected) && pass;
	pass = piglit_probe_pixel_rgb(w * 9.5, piglit_height / 2, expected) && pass;
	pass = piglit_probe_pixel_rgb(w * 11.5, piglit_height / 2, expected) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	if (!piglit_is_extension_supported("GL_ATI_separate_stencil") && piglit_get_gl_version() < 20) {
		printf("Sorry, this program requires either "
		       "GL_ATI_separate_stencil or OpenGL 2.0.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	if (piglit_get_gl_version() < 20) {
		use20syntax = 0;
	}

	printf("\nAll 5 (or 6) squares should be the same color.\n");
}

