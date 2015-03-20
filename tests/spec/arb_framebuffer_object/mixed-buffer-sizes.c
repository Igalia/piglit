/*
 * Copyright © 2015 VMware, Inc.
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
 * Check that an FBO with different renderbuffer/texture sizes works
 * as expected.  If the color and depth buffers aren't the same size,
 * the rendering in the intersection area should be valid.
 *
 * \author Brian Paul
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END


static GLubyte *RefImage = NULL;



static void
display_image(int w, int h, const GLubyte *testImage)
{
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, testImage);
	piglit_present_results();
	glEnable(GL_DEPTH_TEST);
}


/**
 * Render image with given color buffer, depth buffer and viewport sizes.
 */
static bool
render_mixed_fbo(GLsizei colorW, GLsizei colorH,
		 GLsizei depthW, GLsizei depthH,
		 GLsizei vpW, GLsizei vpH)
{
	GLubyte *results;
	GLuint fb, rb[2];
	GLenum status;
	bool pass = true;
	int sizeInBytes = vpW * vpH * 4;

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glGenRenderbuffers(2, rb);

	/* setup rb[0] for color */
	glBindRenderbuffer(GL_RENDERBUFFER, rb[0]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, colorW, colorH);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, rb[0]);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* setup rb[0] for depth */
	glBindRenderbuffer(GL_RENDERBUFFER, rb[1]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
			      depthW, depthH);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
				  GL_RENDERBUFFER, rb[1]);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr,
			"Framebuffer incomplete (status %s)\n",
			piglit_get_gl_enum_name(status));
		piglit_report_result(PIGLIT_FAIL);
	}

	glViewport(0, 0, vpW, vpH);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_TRIANGLES);
	/* large triangle */
	glColor3f(1, 0, 0);
	glVertex2f(-1, -1);
	glColor3f(0, 1, 0);
	glVertex2f(1, -1);
	glColor3f(0, 0, 1);
	glVertex2f(0, 1);
	/* intersecting triangle (to test depth testing) */
	glColor3f(1, 1, 1);
	glVertex3f(-1,-1, -1);
	glVertex3f(-1, 1, -1);
	glVertex3f(1, 0, 1);
	glEnd();

	results = malloc(vpW * vpH * 4);
	glReadPixels(0, 0, vpW, vpH, GL_RGBA, GL_UNSIGNED_BYTE, results);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteRenderbuffers(2, rb);
	glDeleteFramebuffers(1, &fb);

	if (RefImage) {
		/* compare against reference image */
		if (memcmp(RefImage, results, sizeInBytes)) {
			printf("Rendering failure with:\n");
			printf("  color buffer %d x %d\n", colorW, colorH);
			printf("  depth buffer %d x %d\n", depthW, depthH);
			pass = false;
			display_image(vpW, vpH, results);
		}
		free(results);
	}
	else {
		/* save this as the reference image */
		RefImage = results;
	}

	return pass;
}


enum piglit_result
piglit_display(void)
{
	const int w = 120, h = 100;
	bool pass = true;

	/* Create reference image */
	pass = render_mixed_fbo(w, h, w, h, w, h);

	/* Render mixed buffers: larger color, smaller depth */
	pass = render_mixed_fbo(170, 103, w, h, w, h) && pass;

	/* Render mixed buffers: smaller color, larger depth */
	pass = render_mixed_fbo(w, h, 175, 109, w, h) && pass;

	/* Render mixed buffers: taller color, wider depth */
	pass = render_mixed_fbo(w, 199, 177, h, w, h) && pass;

	if (pass)
		display_image(w, h, RefImage);

	free(RefImage);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_framebuffer_object");
	glClearColor(0.25, 0.25, 0.25, 1.0);
	glEnable(GL_DEPTH_TEST);
}
