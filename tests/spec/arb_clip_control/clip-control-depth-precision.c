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

/** @file clip-control-depth-precision.c
 *
 * Test for ARB_clip_control.
 * This is actually the application level use case making use of the
 * close to logarithmic depth buffer precision available with the
 * GL_ZERO_TO_ONE depth mode that is newly provided with this extension.
 * The ARB_clip_control spec gives a set of web references explaining the
 * background greatly.
 * In short we set up a projection matrix that maps infinite far away
 * points to 0 and the near plane to 1. We use a float depth buffer
 * with a well known accuracy behavior. That together gives a depth
 * buffer resolution that is about the relative floating point accuracy
 * relative to the distance from the eye point.
 * This extension avoids adding a constant number even in an intermediate
 * step which would destroy the effective depth precision possible with
 * the floating point depth buffers.
 * Roughtly in numbers:
 * Two fragments at 5000001 and 5000000 \about 5000001*(1 - eps) distance
 * from the eye point should yield to different values in the depth buffer.
 * The same goes for about any fragment distance x that you should be able
 * to distinguish this from x*(1 - eps).
 * And this is exactly what this test checks. We draw two surfaces
 * a big red one at a distance x and a half that big green one at a distance
 * x*(1 - 10*eps) to have a security factor of 10 to allow for some roundoff
 * errors to accumulate. Due to the depth precision we must not get z fighting
 * between these two and see a nested green solid square inside a bigger red
 * square really behind it.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	GLdouble projection[16] = { 0.0, };

	piglit_require_extension("GL_ARB_clip_control");
	piglit_require_extension("GL_ARB_depth_buffer_float");
	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_EXT_framebuffer_blit");

	/* Set up a projection matrix mapping z = -1 to z = 1
	 * and z = -inf to z = 0 in projection space.
	 * Given the clip control setting below, this is just
	 * written as is into the float depth buffer.
	 */
	projection[0 + 4*0] = 1;
	projection[1 + 4*1] = (GLdouble)piglit_width/piglit_height;
	projection[2 + 4*3] = 1;
	projection[3 + 4*2] = -1;

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(projection);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

enum piglit_result
piglit_display(void)
{
	GLfloat red[3] = { 1, 0, 0 };
	GLfloat green[3] = { 0, 1, 0 };
	GLboolean pass = GL_TRUE;
	GLuint fb, cb, db;
	GLenum status;
	int range10;

	glGenRenderbuffers(1, &cb);
	glBindRenderbuffer(GL_RENDERBUFFER, cb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, piglit_width, piglit_height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenRenderbuffers(1, &db);
	glBindRenderbuffer(GL_RENDERBUFFER, db);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, piglit_width, piglit_height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, cb);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, db);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FBO incomplete status 0x%X\n", status);
		piglit_report_result(PIGLIT_FAIL);
	}

	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	glClearDepth(0);
	glDepthRange(0, 1);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GREATER);

	for (range10 = 0; range10 < 16; ++range10) {
		int width = piglit_width/4;
		int height = piglit_height/4;
		int tilex = range10 % 4;
		int tiley = range10 / 4;
		int x = tilex*width;
		int y = tiley*height;
		double z = pow(10, 1 + range10);

		/* Set up a new viewport for each depth we want to test */
		glViewport(x, y, width, height);

		/* Draw a red surface at given distance z */
		glColor3fv(red);
		piglit_draw_rect_z(-z, -0.5*z, -0.5*z, z, z);

		pass = piglit_probe_pixel_rgb(x + width/2, y + height/2, red) && pass;

		/* And a green one just close in front of that red one */
		glColor3fv(green);
		piglit_draw_rect_z((10*FLT_EPSILON - 1)*z, -0.25*z, -0.25*z, 0.5*z, 0.5*z);

		pass = piglit_probe_pixel_rgb(x + width/2, y + height/2, green) && pass;
	}


	/* set viewport to window size */
	glViewport(0, 0, piglit_width, piglit_height);

	/* copy the result to the back buffer */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, piglit_width, piglit_height, 0, 0, piglit_width, piglit_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteFramebuffers(1, &fb);
	glDeleteRenderbuffers(1, &cb);
	glDeleteRenderbuffers(1, &db);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
