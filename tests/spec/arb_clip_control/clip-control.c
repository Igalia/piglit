/*
 * Copyright Mathias Fröhlich <Mathias.Froehlich@web.de>
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
 * Authors:
 *    Mathias Fröhlich <Mathias.Froehlich@web.de>
 */

/** @file clip-control.c
 *
 * Basic test for ARB_clip_contol.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_clip_control");

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}

GLboolean
test_clip_control(GLenum origin, GLenum mode)
{
	GLboolean pass;
	GLint value;

	pass = piglit_check_gl_error(GL_NO_ERROR);

	glGetIntegerv(GL_CLIP_ORIGIN, &value);
	if (value != origin) {
		fprintf(stderr, "glClipControl origin unexpected!\n");
		pass = GL_FALSE;
	}
	glGetIntegerv(GL_CLIP_DEPTH_MODE, &value);
	if (value != mode) {
		fprintf(stderr, "glClipControl mode unexpected!\n");
		pass = GL_FALSE;
	}

	return pass;
}

GLboolean
state_test(void)
{
	GLboolean pass = GL_TRUE;

	/* The initial values */
	pass = test_clip_control(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE) && pass;

	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	pass = test_clip_control(GL_LOWER_LEFT, GL_ZERO_TO_ONE) && pass;

	glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);
	pass = test_clip_control(GL_UPPER_LEFT, GL_ZERO_TO_ONE) && pass;

	glClipControl(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
	pass = test_clip_control(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE) && pass;

	/* Check bailing out on invalid input */
	glClipControl(GL_RGB, GL_NEGATIVE_ONE_TO_ONE);
	pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;
	piglit_reset_gl_error();
	pass = test_clip_control(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE) && pass;

	glClipControl(GL_LOWER_LEFT, GL_RGB);
	pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;
	piglit_reset_gl_error();
	pass = test_clip_control(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE) && pass;


	/* Check push/pop */
	glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
	pass = test_clip_control(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE) && pass;

	glPushAttrib(GL_TRANSFORM_BIT);

	glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);
	pass = test_clip_control(GL_UPPER_LEFT, GL_ZERO_TO_ONE) && pass;

	/* Back to default */
	glPopAttrib();
	pass = test_clip_control(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE) && pass;

	return pass;
}

GLboolean
test_patch(GLenum origin, GLenum depth,
	   GLclampd near, GLclampd far,
	   float x, float y, float z)
{
	GLboolean pass = GL_TRUE;
	float draw_y;
	float depth_z;

	/* Draw the patch */

	if (origin == GL_LOWER_LEFT)
		draw_y = y;
	else
		draw_y = piglit_height - y;

	piglit_draw_rect_z(z, x - 2, draw_y - 2, 4, 4);


	/* Check the depth and clip behavior */

	/* The usual orthographic projection flips the z sign */
	if (depth == GL_NEGATIVE_ONE_TO_ONE)
		depth_z = -z*0.5*(far - near) + 0.5*(near + far);
	else
		depth_z = -z*(far - near) + near;

	if ((depth_z < near && depth_z < far)
	    || (near < depth_z && far < depth_z))
		/* still the clear value */
		pass = piglit_probe_pixel_depth(x, y, 1.0) && pass;
	else
		/* the written depth value */
		pass = piglit_probe_pixel_depth(x, y, depth_z) && pass;

	return pass;
}

GLboolean
test_patches(GLenum origin, GLenum depth,
	     GLclampd near, GLclampd far,
	     float x)
{
	GLboolean pass = GL_TRUE;

	/* Back to default */
	glClipControl(origin, depth);
	glDepthRange(near, far);

	if (depth == GL_NEGATIVE_ONE_TO_ONE) {
		/* outside the clip space */
		pass = test_patch(origin, depth, near, far,
				  x, 10, 1.5) && pass;
		/* inside the clip space */
		pass = test_patch(origin, depth, near, far,
				  x, 20, 1.0) && pass;
		pass = test_patch(origin, depth, near, far,
				  x, 30, 0.5) && pass;
		pass = test_patch(origin, depth, near, far,
				  x, 40, 0.0) && pass;
		pass = test_patch(origin, depth, near, far,
				  x, 50, -0.5) && pass;
		pass = test_patch(origin, depth, near, far,
				  x, 60, -1.0) && pass;
		/* outside the clip space */
		pass = test_patch(origin, depth, near, far,
				  x, 70, -1.5) && pass;
	} else {
		/* outside the clip space */
		pass = test_patch(origin, depth, near, far,
				  x, 10, 0.25) && pass;
		/* inside the clip space */
		pass = test_patch(origin, depth, near, far,
				  x, 20, 0.0) && pass;
		pass = test_patch(origin, depth, near, far,
				  x, 30, -0.25) && pass;
		pass = test_patch(origin, depth, near, far,
				  x, 40, -0.5) && pass;
		pass = test_patch(origin, depth, near, far,
				  x, 50, -0.75) && pass;
		pass = test_patch(origin, depth, near, far,
				  x, 60, -1.0) && pass;
		/* outside the clip space */
		pass = test_patch(origin, depth, near, far,
				  x, 70, -1.25) && pass;
	}

	return pass;
}

GLboolean
draw_test(void)
{
	GLboolean pass = GL_TRUE;
	GLenum origin, depth;
	GLclampd near = 0, far = 1;

	/* Now prepare the draw buffer */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	/* Also test the winding order logic */
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	/* The clear value - to be sure */
	pass = piglit_probe_pixel_depth(5, 5, 1.0) && pass;


	/* Defaut depth rage all clip control combinations */
	pass = test_patches(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE,
			    near, far, 10) && pass;
	pass = test_patches(GL_LOWER_LEFT, GL_ZERO_TO_ONE,
			    near, far, 20) && pass;
	pass = test_patches(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE,
			    near, far, 30) && pass;
	pass = test_patches(GL_UPPER_LEFT, GL_ZERO_TO_ONE,
			    near, far, 40) && pass;


	/* Narrow depth rage all clip control combinations */
	near = 0.25;
	far = 0.75;
	pass = test_patches(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE,
			    near, far, 50) && pass;
	pass = test_patches(GL_LOWER_LEFT, GL_ZERO_TO_ONE,
			    near, far, 60) && pass;
	pass = test_patches(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE,
			    near, far, 70) && pass;
	pass = test_patches(GL_UPPER_LEFT, GL_ZERO_TO_ONE,
			    near, far, 80) && pass;


	/* Reverse narrow depth rage all clip control combinations */
	near = 0.75;
	far = 0.25;
	pass = test_patches(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE,
			    near, far, 90) && pass;
	pass = test_patches(GL_LOWER_LEFT, GL_ZERO_TO_ONE,
			    near, far, 100) && pass;
	pass = test_patches(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE,
			    near, far, 110) && pass;
	pass = test_patches(GL_UPPER_LEFT, GL_ZERO_TO_ONE,
			    near, far, 120) && pass;


	/* Back to default */
	glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
	glDepthRange(0.0, 1.0);

	piglit_present_results();

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass;

	/* Check for getting and setting the state. */
	pass = state_test();

	/* Check for correct draws according to the state. */
	pass = draw_test() && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
