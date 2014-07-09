/*
 * Copyright © 2010 Red Hat
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
 *    Dave Airlie
 *
 */

/** @file framebuffer-srgb
 *
 * Test the EXT_framebuffer_sRGB API changes
 * this test enables even when the extension isn't available, and confirms
 * the API acts correctly in that case.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/* size of texture/renderbuffer (power of two) */
#define FBO_SIZE 128

/* do some basic API tests */
static GLboolean
framebuffer_srgb_api_no_ext(void)
{	
	GLboolean pass = GL_TRUE;
	GLboolean boolmode = GL_FALSE;
	GLenum ret;

	glGetBooleanv(GL_FRAMEBUFFER_SRGB_CAPABLE_EXT, &boolmode);
	ret = glGetError();
	if (ret != GL_INVALID_ENUM || boolmode) {
		printf("no extension but no enum error or enabled\n");
		pass = GL_FALSE;
	}

	(void)glIsEnabled(GL_FRAMEBUFFER_SRGB_EXT);
	ret = glGetError();
	if (ret != GL_INVALID_ENUM) {
		printf("no invalid enum on glIsEnabled(GL_FRAMEBUFFER_SRGB_EXT)\n");
		pass = GL_FALSE;
	}

	glEnable(GL_FRAMEBUFFER_SRGB_EXT);
	ret = glGetError();
	if (ret != GL_INVALID_ENUM) {
		printf("no invalid enum on glEnable(GL_FRAMEBUFFER_SRGB_EXT)\n");
		pass = GL_FALSE;
	}

	glDisable(GL_FRAMEBUFFER_SRGB_EXT);
	ret = glGetError();
	if (ret != GL_INVALID_ENUM) {
		printf("no invalid enum on glDisable(GL_FRAMEBUFFER_SRGB_EXT)\n");
		pass = GL_FALSE;
	}
	return pass;
}

/* do some basic API tests */
static GLboolean
framebuffer_srgb_api_ext(void)
{	
	GLboolean pass = GL_TRUE;
	GLboolean boolmode;
	GLenum ret;
	GLboolean is_enabled;
	/* check if visual supports API */
	glGetBooleanv(GL_FRAMEBUFFER_SRGB_CAPABLE_EXT, &boolmode);
	ret = glGetError();
	if (ret != 0) {
		printf("unexpected error glGetBooleanv(GL_FRAMEBUFFER_SRGB_CAPABLE_EXT) %d\n", ret);
		pass = GL_FALSE;
	}

	is_enabled = glIsEnabled(GL_FRAMEBUFFER_SRGB_EXT);
	ret = glGetError();
	if (ret != 0) {
		printf("unexpected error getting IsEnabled %d\n", ret);
		pass = GL_FALSE;
	}

	/* should be possible to enable/disable independent of capable bit */
	glEnable(GL_FRAMEBUFFER_SRGB_EXT);
	is_enabled = glIsEnabled(GL_FRAMEBUFFER_SRGB_EXT);
	if (!is_enabled) {
		printf("SRGB not enabled after calling glEnable\n");
		pass = GL_FALSE;
	}

	glDisable(GL_FRAMEBUFFER_SRGB_EXT);
	is_enabled = glIsEnabled(GL_FRAMEBUFFER_SRGB_EXT);
	if (is_enabled) {
		printf("SRGB not disabled after calling glDisable\n");
		pass = GL_FALSE;
	}
	return pass;
}

/**
 * Common code for framebuffer and FBO tests.
 */
static GLboolean
test_srgb(void)
{
	GLboolean pass = GL_TRUE;
	GLboolean srgb_capable;
	float green[] = {0, 0.3, 0, 0};
	float expected_green[4];
	float expected_blend[4];

	/*
	 * Note: the window-system framebuffer may or may not be sRGB capable.
	 * But the user-created FBO should be sRGB capable.
	 */
	glGetBooleanv(GL_FRAMEBUFFER_SRGB_CAPABLE_EXT, &srgb_capable);

	glDisable(GL_BLEND);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_FRAMEBUFFER_SRGB_EXT);
	glColor4fv(green);

	/*
	 * First square: draw green square without sRGB and no blending
	 */
	piglit_draw_rect(0, 0, 20, 20);

	/*
	 * Second square: draw a green square with sRGB enabled and no blending
	 */
	glEnable(GL_FRAMEBUFFER_SRGB_EXT);
	piglit_draw_rect(20, 0, 20, 20);

	/*
	 * Third square: draw green square, then blend/add another on top of it
	 */
	glEnable(GL_FRAMEBUFFER_SRGB_EXT);
	piglit_draw_rect(40, 0, 20, 20);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	piglit_draw_rect(40, 0, 20, 20);
	glDisable(GL_BLEND);
	glDisable(GL_FRAMEBUFFER_SRGB_EXT);

	/*
	 * Check first square.
	 */
	if (!piglit_probe_rect_rgb(0, 0, 20, 20, green))
		pass = GL_FALSE;
	/* check pixel path */
	glEnable(GL_FRAMEBUFFER_SRGB_EXT);
	if (!piglit_probe_rect_rgb(0, 0, 20, 20, green))
		pass = GL_FALSE;
	glDisable(GL_FRAMEBUFFER_SRGB_EXT);

	/*
	 * Check second square
	 */
	memcpy(expected_green, green, sizeof(float) * 4);
	if (srgb_capable)
		expected_green[1] = piglit_linear_to_srgb(green[1]);
	if (!piglit_probe_rect_rgb(20, 0, 20, 20, expected_green))
		pass = GL_FALSE;
	/* check it doesn't affect the pixel path */
	glEnable(GL_FRAMEBUFFER_SRGB_EXT);
	if (!piglit_probe_rect_rgb(20, 0, 20, 20, expected_green))
		pass = GL_FALSE;
	glDisable(GL_FRAMEBUFFER_SRGB_EXT);

	/*
	 * Check third square
	 */
	memcpy(expected_blend, green, sizeof(float) * 4);
	if (srgb_capable)
		expected_blend[1] = piglit_linear_to_srgb(green[1] * 2.0);
	else
		expected_blend[1] = green[1] * 2.0;
	if (!piglit_probe_rect_rgb(40, 0, 20, 20, expected_blend))
		pass = GL_FALSE;

	return pass;
}

static GLboolean
framebuffer_srgb_non_fbo(void)
{
	GLboolean pass;

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	pass = test_srgb();
	piglit_present_results();
	return pass;
}

static GLuint
make_fbo(int w, int h)
{
	GLuint tex;
	GLuint fb;
 	GLenum status;

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8_EXT,
		     w, h, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D,
				  tex,
				  0);
	assert(glGetError() == 0);

	status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "fbo incomplete (status = 0x%04x)\n", status);
		piglit_report_result(PIGLIT_SKIP);
	}

	return fb;
}

static void
draw_fbo(int x, int y)
{
	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glClear(GL_COLOR_BUFFER_BIT);
	piglit_draw_rect_tex(x, y, FBO_SIZE, FBO_SIZE,
			     0, 0, 1, 1);
	glDisable(GL_TEXTURE_2D);
}

static GLboolean
framebuffer_srgb_fbo(void)
{
	GLboolean pass = GL_TRUE;
	int fbo_width = FBO_SIZE;
	int fbo_height = FBO_SIZE;
	GLuint fbo;

	fbo = make_fbo(fbo_width, fbo_height);
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, fbo);
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, fbo);
	glViewport(0, 0, fbo_width, fbo_height);

	piglit_ortho_projection(fbo_width, fbo_height, GL_FALSE);

	pass = test_srgb();

	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

	draw_fbo(0, 0);
	piglit_present_results();
	glDeleteFramebuffersEXT(1, &fbo);
	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLboolean have_srgb_ext = GL_FALSE;

	if (piglit_is_extension_supported("GL_ARB_framebuffer_sRGB"))
		have_srgb_ext = GL_TRUE;

	if (!have_srgb_ext) {
		pass = framebuffer_srgb_api_no_ext();
		goto out;
	}

	pass = framebuffer_srgb_api_ext();
	if (pass == GL_TRUE)
		pass = framebuffer_srgb_non_fbo();

	if (pass == GL_TRUE)
		pass = framebuffer_srgb_fbo();

 out:
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


static void reshape(int width, int height)
{
	piglit_width = width;
	piglit_height = height;

	piglit_ortho_projection(width, height, GL_FALSE);
}

void
piglit_init(int argc, char **argv)
{
	reshape(piglit_width, piglit_height);
	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_EXT_framebuffer_blit");

	glClearColor(0.5, 0.5, 0.5, 1.0);
}
