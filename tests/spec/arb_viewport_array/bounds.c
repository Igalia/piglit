/*
 * Copyright © 2013 LunarG, Inc.
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
 *
 * Author: Jon Ashburn <jon@lunarg.com>
 */

/**
 * Tests for the validity  of Viewport bounds, Depth Range bounds  and
 * Scissor Box bounds with viewport arrays (0 to GL_MAX_VIEWPORTS-1).
 * "Bounds" are the rectangle or range (eg x, y, width, height).
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/**
 * Test clamping for viewport x,y, width, height. They should be clamped
 * to VIEWPORT_BOUNDS_RANGE and MAX_VIEWPORT_DIMS.  INVALID_VALUE for
 * negative w,h. Test default values of x,y,w,h.
 * OpenGL 4.3 Core section 13.6.1 ref:
 *    "The location of the viewport’s bottom-left corner, given by (x, y),
 *    are clamped to be within the implementation-dependent viewport bounds
 *    range. The viewport bounds range [min, max] tuple may be determined by
 *    calling GetFloatv with the symbolic constant VIEWPORT_BOUNDS_RANGE (see
 *    section 22)."
 *
 *    "Viewport width and height are clamped to implementation-dependent
 *    maximums when specified. The maximum width and height may be found by
 *    calling GetFloatv with the symbolic constant MAX_VIEWPORT_DIMS."
 *
 *    "An INVALID_VALUE error is generated if either w or h is negative."
 *
 *    "In the initial state, w and h for each viewport are set to the width
 *    and height, respectively, of the window into which the GL is to do its
 *    rendering. If the default framebuffer is bound but no default framebuffer
 *    is associated with the GL context (see chapter 9), then w and h are
 *    initially set to zero. ox, oy , n, and f are set to w/2 , h/2, 0.0, and
 *    1.0, respectively."
 */
static bool
viewport_bounds(GLint maxVP)
{
	GLfloat maxDims[2];
	GLfloat range[2];
	GLfloat vp[4], vpGet[4];
	bool pass = true;
	int i;

	/* intial values for x,y,w,h */
	for (i = 0; i < maxVP; i++) {
		glGetFloati_v(GL_VIEWPORT, i, vp);
		if (vp[0] != 0.0 || vp[1] != 0.0 ||
		    vp[2] != (GLfloat) piglit_width ||
		    vp[3] != (GLfloat) piglit_height) {
			printf("viewport default value wrong for idx %d\n", i);
			pass = false;
		}
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* test clamping of viewport values */
	glGetFloatv(GL_MAX_VIEWPORT_DIMS, maxDims);
	glGetFloatv(GL_VIEWPORT_BOUNDS_RANGE, range);
	vp[0] = range[0] - 2.0;
	vp[1] = range[1] + 2.0;
	vp[2] = maxDims[0] + 1.0;
	vp[3] = maxDims[1] + 1.0;
	glViewportArrayv(0, 1, vp);
	glGetFloati_v(GL_VIEWPORT, 0, vpGet);
	if (vpGet[0] != range[0] || vpGet[1] != range[1] ||
	    vpGet[2] != maxDims[0] || vpGet[3] != maxDims[1]) {
		printf("viewport clamping failed glViewportArrayv\n");
		pass = false;
	}
	glViewportIndexedf(1, vp[0], vp[1], vp[2], vp[3]);
	glGetFloati_v(GL_VIEWPORT, 1, vpGet);
	if (vpGet[0] != range[0] || vpGet[1] != range[1] ||
	    vpGet[2] != maxDims[0] || vpGet[3] != maxDims[1]) {
		printf("viewport clamping failed glViewportIndexedf\n");
		pass = false;
	}
	glViewportIndexedfv(2, vp);
	glGetFloati_v(GL_VIEWPORT, 2, vpGet);
	if (vpGet[0] != range[0] || vpGet[1] != range[1] ||
	    vpGet[2] != maxDims[0] || vpGet[3] != maxDims[1]) {
		printf("viewport clamping failed glViewportIndexedfv\n");
		pass = false;
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* negative width, height gives gl error */
	vp[2] = -10.3;
	vp[3] = 0.0;
	for (i = 0; i < 2; i++) {
		glViewportArrayv(0, 1, vp);
		pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
		glViewportIndexedf(1, vp[0], vp[1], vp[2], vp[3]);
		pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
		glViewportIndexedfv(2, vp);
		pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
		vp[2] = 5.0;
		vp[3] = -12345.7;
	}

	return pass;
}

/**
 * Test clamping for depth range near and far. Make sure clamped
 *  to [0, 1]. Test default values for near and far.
 * OpenGL 4.3 Core section 13.6.1 ref:
 *    "Values in v are each clamped to the range [0, 1] when specified."
 *
 */
static bool
depth_range_bounds(GLint maxVP)
{
	bool pass = true;
	GLdouble dr[2], drGet[2];
	int i;

	/* intial values for near, far are 0.0, 1.0 repsectively */
	for (i = 0; i < maxVP; i++) {
		glGetDoublei_v(GL_DEPTH_RANGE, i, dr);
		if (dr[0] != 0.0 || dr[1] != 1.0) {
			printf("depth_range default value wrong for idx %d\n",
			       i);
			pass = false;
		}
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* test clamping of depth_range values */
	dr[0] = -0.001;
	dr[1] = 2.0;
	glDepthRangeArrayv(0, 1, dr);
	glGetDoublei_v(GL_DEPTH_RANGE, 0, drGet);
	if (drGet[0] != 0.0 || drGet[1] != 1.0) {
		printf("depth_range clamping failed glDepthRangeArrayv\n");
		pass = false;
	}
	glDepthRangeIndexed(1, dr[0], dr[1]);
	glGetDoublei_v(GL_DEPTH_RANGE, 1, drGet);
	if (drGet[0] != 0.0 || drGet[1] != 1.0) {
		printf("depth_range clamping failed glDepthRangeIndexed\n");
		pass = false;
	}

	return pass;
}

/**
 * Test invalid values for scissor left, bottom, width, height
 * INVALID_VALUE for negative w,h.  Test default values for left, bottom,
 * width, height.
 * OpenGL 4.3 Core section 13.6.1 ref:
 *    "In the initial state, left = bottom = 0, and width and
 *    height are determined by the size of the window into which the GL is
 *    to do its rendering for all viewports. If the default framebuffer is
 *    bound but no default framebuffer is associated with the GL context
 *    (see chapter 4), then width and height are initially set to zero."
 *
 *    "If either width or height is less than zero for any scissor rectangle,
 *    then an INVALID_VALUE error is generated."
 */
static bool
scissor_bounds(GLint maxVP)
{
	GLint sc[4];
	bool pass = true;
	int i;

	/* intial values for left, bottom, width, height */
	for (i = 0; i < maxVP; i++) {
		glGetIntegeri_v(GL_SCISSOR_BOX, i, sc);
		if (sc[0] != 0 || sc[1] != 0 || sc[2] != piglit_width ||
		    sc[3] != piglit_height) {
			printf("scissor box default value wrong for idx %d\n",
			       i);
			pass = false;
		}
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* make sure large values don't cause gl errors */
	glScissorIndexed(0, 0x8000, 0x80000000, 0x7ffff, 0x7fffffff);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* negative width, height gives gl error */
	sc[2] = -10;
	sc[3] = 0;
	for (i = 0; i < 2; i++) {
		glScissorArrayv(0, 1, sc);
		pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
		glScissorIndexed(1, sc[0], sc[1], sc[2], sc[3]);
		pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
		glScissorIndexedv(2, sc);
		pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
		sc[2] = 5;
		sc[3] = -12345;
	}

	return pass;
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

#define X(f, desc)					     	\
	do {							\
		const bool subtest_pass = (f);			\
		piglit_report_subtest_result(subtest_pass	\
					     ? PIGLIT_PASS : PIGLIT_FAIL, \
					     (desc));		\
		pass = pass && subtest_pass;			\
	} while (0)

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLint maxVP;

	piglit_require_extension("GL_ARB_viewport_array");

	glGetIntegerv(GL_MAX_VIEWPORTS, &maxVP);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("GL error prior to viewport bounds testing\n");
		piglit_report_result(PIGLIT_FAIL);
		return;
	}

	X(viewport_bounds(maxVP), "Viewport x, y, width, height validity");
	X(depth_range_bounds(maxVP), "DepthRange near, far validity");
	X(scissor_bounds(maxVP),
	  "Scissor left, bottom, width, height validity");
#undef X
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
