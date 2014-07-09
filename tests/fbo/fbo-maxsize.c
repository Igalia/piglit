/*
 * Copyright © 2010 Marek Olšák <maraeo@gmail.com>
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
 */

/** @file fbo-maxsize.c
 *
 * Tests that rendering to a texture of maximum size works.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 256;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static int
find_max_texture_size(void)
{
	GLint maxsize;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxsize);

	while (maxsize > 1) {
		GLint w, h;
		glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA,
			     maxsize, maxsize, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0,
					 GL_TEXTURE_WIDTH, &w);
		glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0,
					 GL_TEXTURE_WIDTH, &h);
		if (w == maxsize && h == maxsize) {
			return maxsize;
		}
		else {
			maxsize /= 2;
		}
	}
	return maxsize;
}


static void
draw_color_sub_rect(int x, int y, int texsize)
{
	int k = texsize / 64;
	int x0 = x - k;
	int x1 = x + k;
	int y1 = y - k;
	int y2 = y + k;

	float r0 = (float) x0 / texsize;
	float r1 = (float) x1 / texsize;

	float g0 = (float) y1 / texsize;
	float g1 = (float) y2 / texsize;

	float b0 = (float) y1 / texsize;
	float b1 = (float) y2 / texsize;

	glBegin(GL_POLYGON);
	glColor3f(r0, g0, b0);  glVertex2i(x0, y1);
	glColor3f(r1, g0, b0);  glVertex2i(x1, y1);
	glColor3f(r1, g1, b1);  glVertex2i(x1, y2);
	glColor3f(r0, g1, b1);  glVertex2i(x0, y2);
	glEnd();
}


static int create_fbo(void)
{
	GLuint tex, fb;
	GLint maxsize;
	GLenum status;
	int x0, x1, y0, y1;
	GLenum glerror;

	maxsize = find_max_texture_size();
	printf("max 2D texture size: %d x %d\n", maxsize, maxsize);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxsize, maxsize,
		     0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glerror = glGetError();

	switch (glerror) {
	case GL_NO_ERROR:
		break;
	case GL_OUT_OF_MEMORY:
		puts("Got GL_OUT_OF_MEMORY.");
		piglit_report_result(PIGLIT_PASS);
	default:
		printf("Unexpected error: %s\n",
		       piglit_get_gl_enum_name(glerror));
		piglit_report_result(PIGLIT_FAIL);
	}

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D,
				  tex, 0);
	assert(glGetError() == 0);

	status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf("FBO incomplete\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	glViewport(0, 0, maxsize, maxsize);
	piglit_ortho_projection(maxsize, maxsize, GL_FALSE);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	x0 = maxsize / 4;
	x1 = maxsize * 3 / 4;
	y0 = maxsize / 4;
	y1 = maxsize * 3 / 4;
	draw_color_sub_rect(x0, y0, maxsize);
	draw_color_sub_rect(x1, y0, maxsize);
	draw_color_sub_rect(x1, y1, maxsize);
	draw_color_sub_rect(x0, y1, maxsize);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glDeleteFramebuffersEXT(1, &fb);

	return tex;
}


/** draw textured rect centered on the given pixel.
 */
static void
draw_tex_sub_rect(int x, int y)
{
	int x0 = x - 16;
	int x1 = x + 16;
	int y1 = y - 16;
	int y2 = y + 16;

	float s0 = (float) x0 / piglit_width;
	float s1 = (float) x1 / piglit_width;
	float t0 = (float) y1 / piglit_height;
	float t1 = (float) y2 / piglit_height;

	glBegin(GL_POLYGON);
	glTexCoord2f(s0, t0);  glVertex2i(x0, y1);
	glTexCoord2f(s1, t0);  glVertex2i(x1, y1);
	glTexCoord2f(s1, t1);  glVertex2i(x1, y2);
	glTexCoord2f(s0, t1);  glVertex2i(x0, y2);
	glEnd();
}


enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLuint tex;
	int x1 = piglit_width / 4;
	int x2 = (piglit_width / 4) * 3;
	int y1 = piglit_height / 4;
	int y2 = (piglit_height / 4) * 3;
	int cx = piglit_width / 2;
	int cy = piglit_height / 2;
	float c1[3] = {0.25, 0.25, 0.25};
	float c2[3] = {0.75, 0.25, 0.25};
	float c3[3] = {0.25, 0.75, 0.75};
	float c4[3] = {0.75, 0.75, 0.75};
	float white[3] = {1.0, 1.0, 1.0};

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	tex = create_fbo();
	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	draw_tex_sub_rect(x1, y1);
	draw_tex_sub_rect(x2, y1);
	draw_tex_sub_rect(x1, y2);
	draw_tex_sub_rect(x2, y2);
	draw_tex_sub_rect(cx, cy);

	pass &= piglit_probe_pixel_rgb(x1, y1, c1);
	pass &= piglit_probe_pixel_rgb(x2, y1, c2);
	pass &= piglit_probe_pixel_rgb(x1, y2, c3);
	pass &= piglit_probe_pixel_rgb(x2, y2, c4);
	pass &= piglit_probe_pixel_rgb(cx, cy, white);

	glDeleteTextures(1, &tex);
	glDisable(GL_TEXTURE_2D);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
}
