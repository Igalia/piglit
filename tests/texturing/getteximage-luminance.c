/*
 * Copyright (c) 2012 VMware, Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL VMWARE AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * Test glGetTexImage for luminance formats.
 * Brian Paul
 * 8 Mar 2012
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "getteximage-luminance";
static float tolerance = 3.0 / 255.0;


static bool
rgba_equal(const float *c1, const float *c2)
{
	return ((fabs(c1[0] - c2[0]) < tolerance) &&
		(fabs(c1[1] - c2[1]) < tolerance) &&
		(fabs(c1[2] - c2[2]) < tolerance) &&
		(fabs(c1[3] - c2[3]) < tolerance));
}


static bool
lum_equal(const float *l1, const float *l2)
{
	return fabs(*l1 - *l2) < tolerance;
}


/*
 * Test reading back a luminance texture as luminance and RGBA.
 */
static bool
test_luminance(void)
{
	static const GLfloat lumImage[2*2] = { 0.25, 0.25, 0.25, 0.25 };
	static const GLfloat rgbaImage[4] = { 0.25, 0.0, 0.0, 1.0 };
	GLuint tex;
	GLfloat test[2*2*4];

	/* create 2x2 GL_LUMINANCE texture */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 2, 2, 0,
		     GL_LUMINANCE, GL_FLOAT, lumImage);

	/* Get and check luminance image */
	glGetTexImage(GL_TEXTURE_2D, 0, GL_LUMINANCE, GL_FLOAT, test);
	if (!lum_equal(lumImage, test)) {
		printf("%s: glGetTexImage(GL_LUMINANCE as"
		       " GL_LUMINANCE) failed\n", TestName);
		printf("  Expected %g  Found %g\n", lumImage[0], test[0]);
		return false;
	}

	/* Get and check rgba image */
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, &test);
	if (!rgba_equal(rgbaImage, test)) {
		printf("%s: glGetTexImage(GL_LUMINANCE as GL_RGBA) failed\n",
		       TestName);
		printf("  Expected %g, %g, %g, %g  Found %g, %g, %g, %g\n",
		       rgbaImage[0], rgbaImage[1], rgbaImage[2], rgbaImage[3],
		       test[0], test[1], test[2], test[3]);
		return false;
	}

	return true;
}


/*
 * Test reading back an RGBA texture as luminance.
 */
static bool
test_rgba(void)
{
	static const GLfloat rgbaImage[4] = { 0.5, 0.25, 0.125, 1.0 };
	static const GLfloat lumImage[1] = { 0.5 };
	GLuint tex;
	GLfloat test[2*2*4];

	/* create 1x1 GL_RGBA texture */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,
		     GL_RGBA, GL_FLOAT, rgbaImage);

	/* Get and check luminance image */
	glGetTexImage(GL_TEXTURE_2D, 0, GL_LUMINANCE, GL_FLOAT, test);
	if (!lum_equal(lumImage, test)) {
		printf("%s: glGetTexImage(GL_RGBA as GL_LUMINANCE) failed\n",
		       TestName);
		printf("  Expected %g  Found %g\n", lumImage[0], test[0]);
		return false;
	}

	return true;
}


/*
 * Test reading back a luminance texture via FBO + glReadPixels as RGBA.
 */
static bool
test_fbo_readpixels_lum_as_rgba(void)
{
	static const GLfloat lumImage[2*2] = { 0.25, 0.25, 0.25, 0.25 };
	static const GLfloat rgbaImage[4] = { 0.25, 0.0, 0.0, 1.0 };
	GLuint tex, fbo;
	GLfloat test[2*2*4];
	GLenum status;

	if (!piglit_is_extension_supported("GL_ARB_framebuffer_object"))
		return true;

	/* create 2x2 GL_LUMINANCE texture */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 2, 2, 0,
		     GL_LUMINANCE, GL_FLOAT, lumImage);

	/* create an FBO to wrap the texture so we can read it back
	 * with glReadPixels
	 */
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT,
			       GL_TEXTURE_2D, tex, 0);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		/* can't test glReadPixels from a luminance fbo/texture */
		if (!piglit_automatic) {
			printf("Skipping FBO ReadPixels test\n");
		}
		return true;
	}

	/* get rgba image (only red should have the lum value) */
	glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, test);
	if (!rgba_equal(rgbaImage, test)) {
		printf("%s: glReadPixels(GL_LUMINANCE as GL_RGBA) failed\n",
		       TestName);
		printf("  Expected %g, %g, %g, %g  Found %g, %g, %g, %g\n",
		       rgbaImage[0], rgbaImage[1], rgbaImage[2], rgbaImage[3],
		       test[0], test[1], test[2], test[3]);
		return false;
	}

	return true;
}


/*
 * Test reading back an RGBA texture via FBO + glReadPixels as luminance.
 */
static bool
test_fbo_readpixels_rgba_as_lum(void)
{
	static const GLfloat rgbaImage[4] = { 0.5, 0.25, 0.125, 1.0 };
	static const GLfloat lumImage[1] = { 0.5 + 0.25 + 0.125 };
	GLuint tex, fbo;
	GLfloat test[1];
	GLenum status;

	if (!piglit_is_extension_supported("GL_ARB_framebuffer_object"))
		return true;

	/* create 1x1 GL_RGBA texture */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,
		     GL_RGBA, GL_FLOAT, rgbaImage);

	/* create an FBO to wrap the texture so we can read it back
	 * with glReadPixels
	 */
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT,
			       GL_TEXTURE_2D, tex, 0);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		/* something failed with FBO setup, ignore it */
		if (!piglit_automatic) {
			printf("Skipping FBO ReadPixels test\n");
		}
		return true;
	}

	/* get luminance image, should be sum of RGB values */
	glReadPixels(0, 0, 1, 1, GL_LUMINANCE, GL_FLOAT, test);
	if (!lum_equal(lumImage, test)) {
		printf("%s: glReadPixels(GL_RGBA as GL_LUMINANCE) failed\n",
		       TestName);
		printf("  Expected %g  Found %g\n", lumImage[0], test[0]);
		return false;
	}

	return true;
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;

	pass = test_luminance() && pass;
	pass = test_rgba() && pass;
	pass = test_fbo_readpixels_lum_as_rgba() && pass;
	pass = test_fbo_readpixels_rgba_as_lum() && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);
}
