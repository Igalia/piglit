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

/** @file fbo-srgb
 *
 * This test check that a SRGB8_A8 texture bound as an FBO doesn't
 * do blending and updating to the bound FBO in SRGB mode.
 * this is specified by the EXT_texture_sRGB specification,
 * To get SRGB blending EXT_framebuffer_sRGB is required.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/* size of texture/renderbuffer (power of two) */
#define FBO_SIZE 128

static GLuint
make_fbo(int w, int h, int srgb_format, GLuint *tex_p)
{
	GLuint tex;
	GLuint fb;
 	GLenum status;
	int formats[2][2] =  { { GL_SRGB8_ALPHA8_EXT, GL_RGBA },
			       { GL_SRGB8_EXT, GL_RGB } };

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, formats[srgb_format][0],
		     w, h, 0,
		     formats[srgb_format][1], GL_UNSIGNED_BYTE, NULL);

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

	*tex_p = tex;
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

	piglit_draw_rect_tex(x, y, FBO_SIZE, FBO_SIZE,
			     0, 0, 1, 1);
	glDisable(GL_TEXTURE_2D);
}

static GLboolean
framebuffer_srgb_fbo(int srgb_format)
{
	GLboolean pass = GL_TRUE;
	int fbo_width = FBO_SIZE;
	int fbo_height = FBO_SIZE;
	GLuint fbo;
	float green[] = {0, 0.3, 0.0, 0};
	float expected_green[4];
	float expected_blend[4];
	GLuint tex;

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	fbo = make_fbo(fbo_width, fbo_height, srgb_format, &tex);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

	assert(glGetError() == 0);

	glViewport(0, 0, fbo_width, fbo_height);
	piglit_ortho_projection(fbo_width, fbo_height, GL_FALSE);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glColor4fv(green);
	piglit_draw_rect(0, 0, 20, 20);

	piglit_draw_rect(30, 30, 20, 20);

	/* enable blending and draw a green square with sRGB enabled on top of the previous green square */
	piglit_draw_rect(20, 0, 20, 20);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	piglit_draw_rect(20, 0, 20, 20);

	glDisable(GL_BLEND);

	if (!piglit_probe_rect_rgb(0, 0, 20, 20, green))
		pass = GL_FALSE;

	memcpy(expected_green, green, sizeof(float) * 4);

	if (!piglit_probe_rect_rgb(30, 30, 20, 20, expected_green))
		pass = GL_FALSE;
	
	memcpy(expected_blend, green, sizeof(float) * 4);
	expected_blend[1] = green[1] * 2.0;

	if (!piglit_probe_rect_rgb(20, 0, 20, 20, expected_blend))
		pass = GL_FALSE;

	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	
	draw_fbo(0, 0);

	glDeleteFramebuffersEXT(1, &fbo);
	glDeleteTextures(1, &tex);
	piglit_present_results();

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;

	pass = framebuffer_srgb_fbo(0);
	if (pass == GL_TRUE) {
		pass = framebuffer_srgb_fbo(1);
		if (pass == GL_FALSE)
			printf("Failed on format SRGB8\n");
	} else
		printf("Failed on format SRGB8_ALPHA8\n");

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
	piglit_require_extension("GL_EXT_texture_sRGB");
	piglit_require_extension("GL_EXT_framebuffer_object");
	reshape(piglit_width, piglit_height);
}
