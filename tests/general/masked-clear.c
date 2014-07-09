/*
 * Port of Glean maskedClear test to piglit.  Original copyright follows.
 * 
 * Copyright (C) 1999  Allen Akin   All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the
 * Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * Test color/depth/stencil masking with glClear.
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = (PIGLIT_GL_VISUAL_RGB |
				PIGLIT_GL_VISUAL_DEPTH |
				PIGLIT_GL_VISUAL_STENCIL |
				PIGLIT_GL_VISUAL_DOUBLE);
	config.requires_displayed_window = true;
PIGLIT_GL_TEST_CONFIG_END


static void
failRGB(GLint chan, GLfloat expected,
	GLfloat actual, GLenum buffer)
{
	static const char *chanNames[] = { "Red", "Green", "Blue", "Alpha" };
	GLboolean mask[4];
	glGetBooleanv(GL_COLOR_WRITEMASK, mask);
	fprintf(stderr, "masked-clear: %s is %f, expected %f in %s\n",
	       chanNames[chan], actual, expected,
	       piglit_get_gl_enum_name(buffer));
	fprintf(stderr, "\tGL_COLOR_WRITEMASK = (%s, %s, %s, %s)\n",
	       (mask[0] ? "GL_TRUE" : "GL_FALSE"),
	       (mask[1] ? "GL_TRUE" : "GL_FALSE"),
	       (mask[2] ? "GL_TRUE" : "GL_FALSE"),
	       (mask[3] ? "GL_TRUE" : "GL_FALSE"));
}


static void
failZ(GLfloat expected, GLfloat actual)
{
	GLboolean mask;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &mask);
	fprintf(stderr, "masked-clear: depth buffer value is %f, expected %f\n",
	       actual, expected);
	fprintf(stderr, "\tGL_DEPTH_WRITEMASK = %s\n",
	       mask ? "GL_TRUE" : "GL_FALSE");
}


static void
failStencil(GLuint expected, GLuint actual)
{
	GLint mask;
	glGetIntegerv(GL_STENCIL_WRITEMASK, &mask);
	fprintf(stderr, "masked-clear: stencil buffer value is %d, expected %d\n",
	       actual, expected);
	fprintf(stderr, "\tGL_STENCIL_WRITEMASK = 0x%x\n", mask);
}


static bool
test_color_masking(GLenum buffer)
{
	GLint a;
	int chan, comp, numChannels;
	bool passed = true;

	assert(buffer == GL_FRONT || buffer == GL_BACK);

	glReadBuffer(buffer);
	glDrawBuffer(buffer);

	glGetIntegerv(GL_ALPHA_BITS, &a);
	numChannels = a ? 4 : 3;

	for (chan = 0; chan < numChannels && passed; chan++) {
		GLfloat pixel[4];

		/* clear to black */
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);

		/* select one channel to "clear" to 1.0 */
		glColorMask(chan == 0, chan == 1, chan == 2, chan == 3);

		/* try to clear surface to white */
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		/* read 1x1 image at (x,y)=(4,4) */
		glReadPixels(4, 4, 1, 1, GL_RGBA, GL_FLOAT, pixel);

		if (!piglit_automatic)
			piglit_present_results();

		/* test results */
		for (comp = 0; comp < numChannels && passed; comp++) {
			if (comp == chan) {
				/* component should be 1.0 */
				if (pixel[comp] < 0.5) {
					passed = false;
					failRGB(comp, 1.0,
						pixel[comp], buffer);
				}
			} else {
				/* component should be 0.0 */
				if (pixel[comp] > 0.5) {
					passed = false;
					failRGB(comp, 0.0,
						pixel[comp], buffer);
				}
			}
		}
	}

	return passed;
}


static bool
test_depth_masking(void)
{
	GLfloat depth;
	bool passed = true;

	/* clear depth buffer to zero */
	glDepthMask(GL_TRUE);
	glClearDepth(0.0);
	glClear(GL_DEPTH_BUFFER_BIT);

	/* disable Z writes, try to clear to one */
	glDepthMask(GL_FALSE);
	glClearDepth(1.0);
	glClear(GL_DEPTH_BUFFER_BIT);

	/* read 1x1 image at (x,y)=(4,4); */
	glReadPixels(4, 4, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

	/* test result */
	if (depth != 0.0) {
		passed = false;
		failZ(0.0, depth);
	}

	return passed;
}


static bool
test_stencil_masking(void)
{
	GLint stencilBits;
	int bit;
	bool passed = true;

	glGetIntegerv(GL_STENCIL_BITS, &stencilBits);

	/* We just run <stencilBits> tests rather than 2^stencilBits */
	for (bit = 0; bit < stencilBits; bit++) {
		GLuint stencil;

		/* clear to 0 */
		glStencilMask(~0);
		glClearStencil(0);
		glClear(GL_STENCIL_BUFFER_BIT);

		/* select one bit to "clear" to 1 */
		glStencilMask(1 << bit);

		/* try to clear stencil buffer to ~0 */
		glClearStencil(~0);
		glClear(GL_STENCIL_BUFFER_BIT);

		/* read 1x1 image at (x,y)=(4,4) */
		glReadPixels(4, 4, 1, 1,
			     GL_STENCIL_INDEX, GL_UNSIGNED_INT, &stencil);

		/* test results */
		if (stencil != (1U << bit)) {
			passed = false;
			failStencil(1 << bit, stencil);
		}
	}

	return passed;
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;

	pass = test_color_masking(GL_FRONT) && pass;

	pass = test_color_masking(GL_BACK) && pass;

	pass = test_depth_masking() && pass;

	pass = test_stencil_masking() && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	/* nothing */
}
