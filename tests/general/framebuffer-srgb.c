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

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_MAIN(
    128 /*window_width*/,
    128 /*window_height*/,
    GLUT_RGB | GLUT_DOUBLE)

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

static GLfloat
linear_to_nonlinear(GLfloat cl)
{
   /* can't have values outside [0, 1] */
   GLfloat cs;

   if (cl < 0.0031308f) {
      cs = 12.92f * cl;
   }
   else {
      cs = (GLfloat)(1.055 * pow(cl, 0.41666) - 0.055);
   }
   
   return cs;
}

static GLboolean
framebuffer_srgb_non_fbo(void)
{
	GLboolean pass = GL_TRUE;
	GLboolean boolmode;
	float green[] = {0, 0.3, 0, 0};
	float expected_green[4];
	float expected_blend[4];

	glGetBooleanv(GL_FRAMEBUFFER_SRGB_CAPABLE_EXT, &boolmode);

	/* drawing test */
	/* draw two green squares without sRGB */
	glDisable(GL_BLEND);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor4fv(green);
	piglit_draw_rect(0, 0, 20, 20);

	/* draw a green square with sRGB enabled and no blending */
	glEnable(GL_FRAMEBUFFER_SRGB_EXT);

	piglit_draw_rect(30, 30, 20, 20);

	/* enable blending and draw a green square with sRGB enabled on top of the previous green square */
	piglit_draw_rect(20, 0, 20, 20);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	piglit_draw_rect(20, 0, 20, 20);

	glDisable(GL_FRAMEBUFFER_SRGB_EXT);

	if (!piglit_probe_rect_rgb(0, 0, 20, 20, green))
		pass = GL_FALSE;

	/* check pixel path */
	glEnable(GL_FRAMEBUFFER_SRGB_EXT);
	if (!piglit_probe_rect_rgb(0, 0, 20, 20, green))
		pass = GL_FALSE;
	glDisable(GL_FRAMEBUFFER_SRGB_EXT);

	memcpy(expected_green, green, sizeof(float) * 4);
	if (boolmode)
		expected_green[1] = linear_to_nonlinear(green[1]);
	memcpy(expected_blend, green, sizeof(float) * 4);
	if (boolmode)
		expected_blend[1] = linear_to_nonlinear(green[1] * 2.0);
	else
		expected_blend[1] = green[1] * 2.0;

	if (!piglit_probe_rect_rgb(20, 0, 20, 20, expected_blend))
		pass = GL_FALSE;

	if (!piglit_probe_rect_rgb(30, 30, 20, 20, expected_green))
		pass = GL_FALSE;

	glutSwapBuffers();
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

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

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
	GLboolean boolmode;
	float green[] = {0, 0.3, 0.0, 0};
	float expected_green[4];
	float expected_blend[4];

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	fbo = make_fbo(fbo_width, fbo_height);
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, fbo);
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, fbo);

	glGetBooleanv(GL_FRAMEBUFFER_SRGB_CAPABLE_EXT, &boolmode);

	assert(glGetError() == 0);

	glViewport(0, 0, fbo_width, fbo_height);
	piglit_ortho_projection(fbo_width, fbo_height, GL_FALSE);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glColor4fv(green);
	piglit_draw_rect(0, 0, 20, 20);

	/* draw a green square with sRGB enabled and no blending */
	glEnable(GL_FRAMEBUFFER_SRGB_EXT);

	piglit_draw_rect(30, 30, 20, 20);

	/* enable blending and draw a green square with sRGB enabled on top of the previous green square */
	piglit_draw_rect(20, 0, 20, 20);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	piglit_draw_rect(20, 0, 20, 20);

	glDisable(GL_BLEND);
	glDisable(GL_FRAMEBUFFER_SRGB_EXT);

	if (!piglit_probe_rect_rgb(0, 0, 20, 20, green))
		pass = GL_FALSE;

	memcpy(expected_green, green, sizeof(float) * 4);
	if (boolmode)
		expected_green[1] = linear_to_nonlinear(green[1]);

	if (!piglit_probe_rect_rgb(30, 30, 20, 20, expected_green))
		pass = GL_FALSE;

	/* check it doesn't affect the pixel path */
	glEnable(GL_FRAMEBUFFER_SRGB_EXT);
	if (!piglit_probe_rect_rgb(30, 30, 20, 20, expected_green))
		pass = GL_FALSE;
	glDisable(GL_FRAMEBUFFER_SRGB_EXT);
	
	memcpy(expected_blend, green, sizeof(float) * 4);
	if (boolmode)
		expected_blend[1] = linear_to_nonlinear(green[1] * 2.0);
	else
		expected_blend[1] = green[1] * 2.0;

	if (!piglit_probe_rect_rgb(20, 0, 20, 20, expected_blend))
		pass = GL_FALSE;

	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	draw_fbo(0, 0);
	glutSwapBuffers();
	glDeleteFramebuffersEXT(1, &fbo);
	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLboolean have_srgb_ext = GL_FALSE;

	if (piglit_is_extension_supported("GL_EXT_framebuffer_sRGB"))
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
}
