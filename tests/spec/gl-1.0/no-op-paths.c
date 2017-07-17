/*  BEGIN_COPYRIGHT -*- glean -*-
 *
 *  Copyright (C) 1999  Allen Akin   All Rights Reserved.
 *  Copyright (C) 2015  Intel Corporation.
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
 *  END_COPYRIGHT
 */
/** @file paths.c
 *
 *  Test basic GL rendering paths.
 *
 *
 *     Based on the original Glean tpaths.cpp test, this test verifies
 *     that basic, trival OpenGL paths work as expected. For example,
 *     glAlphaFunc(GL_GEQUAL, 0.0) should always pass and
 *     glAlphaFunc(GL_LESS, 0.0) should always fail.  We setup trivial
 *     pass and fail conditions for each of alpha test, blending, color mask,
 *     depth test, logic ops, scissor, stencil, stipple, and texture and
 *     make sure they work as expected.  We also setup trival-pass for all
 *     these paths simultaneously and test that as well.
 *
 *     To test for pass/fail we examine the color buffer for white or black,
 *     respectively.
 *
 *     Authors:
 *     Brian Paul <brianp@valinux.com>
 *     Adapted to Piglit by Juliet Fru <julietfru@gmail.com>, August 2015.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
		PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH |
		PIGLIT_GL_VISUAL_STENCIL;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

enum path
{
	ALPHA,
	BLEND,
	COLOR_MASK,
	DEPTH,
	LOGIC,
	SCISSOR,
	STENCIL,
	STIPPLE,
	TEXTURE,
	NUM_PATHS		/* end-of-list token */
};

enum state
{
	DISABLE,
	ALWAYS_PASS,
	ALWAYS_FAIL
};

const char *
path_name(enum path paths)
{
	switch (paths) {
	case ALPHA:
		return "Alpha Test";
	case BLEND:
		return "Blending";
	case COLOR_MASK:
		return "Color Mask";
	case DEPTH:
		return "Depth Test";
	case LOGIC:
		return "LogicOp";
	case SCISSOR:
		return "Scissor Test";
	case STENCIL:
		return "Stencil Test";
	case STIPPLE:
		return "Polygon Stipple";
	case TEXTURE:
		return "Modulated Texture";
	case NUM_PATHS:
		return "paths";
	 default:
                return "BAD PATH VALUE!";
	}
}

void
set_path_state(enum path paths, enum state states)
{
	int i;
	switch (paths) {
	case ALPHA:
		if (states == ALWAYS_PASS) {
			glAlphaFunc(GL_GEQUAL, 0.0);
			glEnable(GL_ALPHA_TEST);
		} else if (states == ALWAYS_FAIL) {
			glAlphaFunc(GL_GREATER, 1.0);
			glEnable(GL_ALPHA_TEST);
		} else {
			glDisable(GL_ALPHA_TEST);
		}
		break;
	case BLEND:
		if (states == ALWAYS_PASS) {
			glBlendFunc(GL_ONE, GL_ZERO);
			glEnable(GL_BLEND);
		} else if (states == ALWAYS_FAIL) {
			glBlendFunc(GL_ZERO, GL_ONE);
			glEnable(GL_BLEND);
		} else {
			glDisable(GL_BLEND);
		}
		break;
	case COLOR_MASK:
		if (states == ALWAYS_PASS) {
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		} else if (states == ALWAYS_FAIL) {
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		} else {
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}
		break;
	case DEPTH:
		if (states == ALWAYS_PASS) {
			glDepthFunc(GL_ALWAYS);
			glEnable(GL_DEPTH_TEST);
		} else if (states == ALWAYS_FAIL) {
			glDepthFunc(GL_NEVER);
			glEnable(GL_DEPTH_TEST);
		} else {
			glDisable(GL_DEPTH_TEST);
		}
		break;
	case LOGIC:
		if (states == ALWAYS_PASS) {
			glLogicOp(GL_OR);
			glEnable(GL_COLOR_LOGIC_OP);
		} else if (states == ALWAYS_FAIL) {
			glLogicOp(GL_AND);
			glEnable(GL_COLOR_LOGIC_OP);
		} else {
			glDisable(GL_COLOR_LOGIC_OP);
		}
		break;
	case SCISSOR:
		if (states == ALWAYS_PASS) {
			glScissor(0, 0, piglit_width, piglit_height);
			glEnable(GL_SCISSOR_TEST);
		} else if (states == ALWAYS_FAIL) {
			glScissor(0, 0, 0, 0);
			glEnable(GL_SCISSOR_TEST);
		} else {
			glDisable(GL_SCISSOR_TEST);
		}
		break;
	case STENCIL:
		if (states == ALWAYS_PASS) {
			/* pass if reference <= stencil value (ref = 0) */
			glStencilFunc(GL_LEQUAL, 0, ~0);
			glEnable(GL_STENCIL_TEST);
		} else if (states == ALWAYS_FAIL) {
			/* pass if reference > stencil value (ref = 0) */
			glStencilFunc(GL_GREATER, 0, ~0);
			glEnable(GL_STENCIL_TEST);
		} else {
			glDisable(GL_STENCIL_TEST);
		}
		break;
	case STIPPLE:
		if (states == ALWAYS_PASS) {
			GLubyte stipple[4 * 32];
			for (i = 0; i < 4 * 32; i++)
				stipple[i] = 0xff;
			glPolygonStipple(stipple);
			glEnable(GL_POLYGON_STIPPLE);
		} else if (states == ALWAYS_FAIL) {
			GLubyte stipple[4 * 32];
			for (i = 0; i < 4 * 32; i++)
				stipple[i] = 0x0;
			glPolygonStipple(stipple);
			glEnable(GL_POLYGON_STIPPLE);
		} else {
			glDisable(GL_POLYGON_STIPPLE);
		}
		break;
	case TEXTURE:
		if (states == DISABLE) {
			glDisable(GL_TEXTURE_2D);
		} else {
			GLubyte val = (states == ALWAYS_PASS) ? 0xff : 0x00;
			GLubyte texImage[4 * 4 * 4];
			for (i = 0; i < 4 * 4 * 4; i++)
				texImage[i] = val;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0,
				     GL_RGBA, GL_UNSIGNED_BYTE, texImage);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
					GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
					GL_NEAREST);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
				  GL_MODULATE);
			glEnable(GL_TEXTURE_2D);
		}
		break;
	default:
		break;
	}
}

void
piglit_init(int argc, char **argv)
{
	/* No init */
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	int i;
	
	static const float white[3] = { 1.0, 1.0, 1.0 };
	static const float black[3] = { 0.0, 0.0, 0.0 };

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glDisable(GL_DITHER);
	
	/* test always-pass paths */
	for (i = 0; i < NUM_PATHS; i++) {
		glClear(GL_COLOR_BUFFER_BIT);
		set_path_state(i, ALWAYS_PASS);

		/* draw polygon */
		piglit_draw_rect(i * 10, 0, 10, 10);

		set_path_state(i, DISABLE);

		/* test buffer */
		if (!piglit_probe_pixel_rgb(i * 10 + 4, 4, white)) {
			printf("Failure with path %s set to always pass.\n",
                               path_name(i));

			pass = false;
		}
	}

	/* enable all always-pass paths */
	{
		glClear(GL_COLOR_BUFFER_BIT);
		for (i = 0; i < NUM_PATHS; i++) {
			set_path_state(i, ALWAYS_PASS);
		}

		/* draw polygon */
		piglit_draw_rect(0, 10, 10, 10);

		for (i = 0; i < NUM_PATHS; i++) {
			set_path_state(i, DISABLE);
		}

		/* test buffer */
		if (!piglit_probe_pixel_rgb(4, 14, white)) {
			printf("Failure with always-pass paths enabled.\n");
			pass = false;
		}
	}

	/* test never-pass paths */
	for (i = 0; i < NUM_PATHS; i++) {
		glClear(GL_COLOR_BUFFER_BIT);
		set_path_state(i, ALWAYS_FAIL);

		/* draw polygon */
		piglit_draw_rect(10 * i, 20, 10, 10);

		set_path_state(i, DISABLE);

		/* test buffer */
		if (!piglit_probe_pixel_rgb(10 * i + 4, 24, black)) {
			printf("Failure with %s set to fail mode.\n",
                               path_name(i));
			pass = false;
		}
	}

	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
