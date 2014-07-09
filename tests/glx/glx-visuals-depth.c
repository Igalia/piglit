/*
 * Copyright © 2011 Intel Corporation
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
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/**
 * @file glx-visuals-depth.c
 *
 * Tests that when a depth buffer is reported as present in the GLX
 * visual that it behaves appropriately (can set a value in it with
 * drawing, and use the depth test on that value), and that when a
 * depth buffer is not present the depth test always passes even
 * if we try to enable it.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_width = 20;
int piglit_height = 20;

enum piglit_result
draw(Display *dpy, GLXFBConfig config)
{
	int dbits;
	float green[3] = {0.0, 1.0, 0.0};
	float blue[3] = {0.0, 0.0, 1.0};
	float *left, *right;
	bool pass = true;

	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);
	glXGetFBConfigAttrib(dpy, config, GLX_DEPTH_SIZE, &dbits);

	piglit_ortho_projection(piglit_width, piglit_height, false);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	/* Set half the FB to depth 0, half to 1, and everything blue */
	glColor3fv(blue);
	piglit_draw_rect_z(1.0,
			   0, 0,
			   piglit_width / 2, piglit_height);
	piglit_draw_rect_z(0.0,
			   piglit_width / 2, 0,
			   piglit_width, piglit_height);

	/* Now draw a rect trying to set just the 1 values to green. */
	glColor3fv(green);
	glDepthFunc(GL_LESS);
	piglit_draw_rect_z(0.5,
			   0, 0, piglit_width, piglit_height);

	/* If there was a depth buffer, then we get half the window
	 * set to green.  Otherwise, the depth test always passes
	 * and the whole thing should have been set green.
	 */
	if (dbits) {
		left = blue;
		right = green;
	} else {
		left = green;
		right = green;
	}

	pass = pass && piglit_probe_rect_rgb(0, 0,
					     piglit_width / 2, piglit_height,
					     left);
	pass = pass && piglit_probe_rect_rgb(piglit_width / 2, 0,
					     piglit_width - piglit_width / 2,
					     piglit_height,
					     right);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

int
main(int argc, char **argv)
{
	enum piglit_result result;

	if (argc > 1 && strcmp(argv[1], "-pixmap") == 0)
		result = piglit_glx_iterate_pixmap_fbconfigs(draw);
	else
		result = piglit_glx_iterate_visuals(draw);

	piglit_report_result(result);

	return 0; /* UNREACHED */
}
