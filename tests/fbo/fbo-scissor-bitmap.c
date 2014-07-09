/*
 * Copyright © 2008 Intel Corporation
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

/** @file fbo-scissor-bitmap.c
 *
 * Tests that clipping of glBitmap to an FBO's glScissor or bounds works correctly.
 *
 * This is a mirror of scissor-bitmap.c to windows.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 400;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

struct probes {
	struct test_position {
		const GLfloat *color;
		const char *name;
		int x;
		int y;
		int bitmap_x_off;
		int bitmap_y_off;
		int width;
		int height;
	} probes[30];
	int n_probes;
};

static void
add_probe(struct probes *probes,
	  const char *name, const GLfloat *color,
	  int x, int y, int width, int height,
	  int bitmap_x_off, int bitmap_y_off)
{
	probes->probes[probes->n_probes].color = color;
	probes->probes[probes->n_probes].name = name;
	probes->probes[probes->n_probes].x = x;
	probes->probes[probes->n_probes].y = y;
	probes->probes[probes->n_probes].bitmap_x_off = bitmap_x_off;
	probes->probes[probes->n_probes].bitmap_y_off = bitmap_y_off;
	probes->probes[probes->n_probes].width = width;
	probes->probes[probes->n_probes].height = height;
	probes->n_probes++;
}

static GLboolean
get_bitmap_bit(int x, int y)
{
	const uint8_t *row = &fdo_bitmap[y * fdo_bitmap_width / 8];

	return (row[x / 8] >> (7 - x % 8)) & 1;
}

static GLboolean
verify_bitmap_pixel(struct probes *probes, int i, int x, int y)
{
	const GLfloat black[4] = {0.0, 0.0, 0.0, 0.0};
	int x1 = probes->probes[i].x;
	int y1 = probes->probes[i].y;
	int bitmap_x1 = probes->probes[i].bitmap_x_off;
	int bitmap_y1 = probes->probes[i].bitmap_y_off;
	const GLfloat *expected;
	GLboolean pass;

	if (probes->probes[i].color == NULL) {
		expected = black;
	} else {
		GLboolean on;

		on = get_bitmap_bit(x - x1 + bitmap_x1,
				    y - y1 + bitmap_y1);
		/* Verify that the region is black if unset, or the foreground
		 * color otherwise.
		 */
		if (on)
			expected = probes->probes[i].color;
		else
			expected = black;
	}

	/* Make sure the region is black */
	pass = piglit_probe_pixel_rgb(x, y, expected);
	if (!pass)
		printf("glBitmap error in %s (test offset %d,%d)\n",
		       probes->probes[i].name,
		       x - probes->probes[i].x,
		       y - probes->probes[i].y);
	return pass;
}

static GLboolean
verify_bitmap_contents(struct probes *probes,
		       int drawable_width, int drawable_height, int i)
{
	int x, y;
	int x1 = probes->probes[i].x;
	int y1 = probes->probes[i].y;
	int x2 = probes->probes[i].x + probes->probes[i].width;
	int y2 = probes->probes[i].y + probes->probes[i].height;
	GLboolean pass = GL_TRUE;

	for (y = y1; y < y2; y++) {
		if (y < 0 || y >= drawable_height)
			continue;
		for (x = x1; x < x2; x++) {
			if (x < 0 || x >= drawable_width)
				continue;

			pass &= verify_bitmap_pixel(probes, i, x, y);
			if (!pass)
				return pass;
		}
	}

	return pass;
}

static GLboolean
draw_and_test(const char *destination, int drawable_width, int drawable_height)
{
	const GLfloat red[4] = {1.0, 0.0, 0.0, 0.0};
	const GLfloat green[4] = {0.0, 1.0, 0.0, 0.0};
	const GLfloat blue[4] = {0.0, 0.0, 1.0, 0.0};
	int i;
	int center_x_start = (drawable_width - fdo_bitmap_width) / 2;
	int center_y_start = (drawable_height - fdo_bitmap_height) / 2;
	int start_x, start_y;
	struct probes probes;
	GLboolean pass = GL_TRUE;

	memset(&probes, 0, sizeof(probes));

	/* Set up projection matrix so we can just draw using window
	 * coordinates.
	 */
	glViewport(0, 0, drawable_width, drawable_height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, drawable_width, 0, drawable_height, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* Clear to black */
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glColor4fv(red);
	/* Center: full image */
	glRasterPos2f(center_x_start, center_y_start);
	glBitmap(fdo_bitmap_width, fdo_bitmap_height, 0, 0, 0, 0, fdo_bitmap);
	add_probe(&probes, "center full", red,
		  center_x_start, center_y_start,
		  fdo_bitmap_width, fdo_bitmap_height,
		  0, 0);

	glEnable(GL_SCISSOR_TEST);
	glColor4fv(green);
	/* Left clipped */
	start_x = center_x_start - fdo_bitmap_width - 10;
	start_y = center_y_start;
	glScissor(start_x + fdo_bitmap_width / 4,
		  start_y,
		  fdo_bitmap_width,
		  fdo_bitmap_height);
	glRasterPos2f(start_x, start_y);
	glBitmap(fdo_bitmap_width, fdo_bitmap_height, 0, 0, 0, 0, fdo_bitmap);
	add_probe(&probes, "left glscissor clipped area", NULL,
		  start_x, start_y,
		  fdo_bitmap_width / 4, fdo_bitmap_height,
		  0, 0);
	add_probe(&probes, "left glscissor unclipped area", green,
		  start_x + fdo_bitmap_width / 4, start_y,
		  fdo_bitmap_width * 3 / 4, fdo_bitmap_height,
		  fdo_bitmap_width / 4, 0);

	/* Right clipped */
	start_x = center_x_start + fdo_bitmap_width + 10;
	start_y = center_y_start;
	glScissor(start_x,
		  start_y,
		  fdo_bitmap_width * 3 / 4,
		  fdo_bitmap_height);
	glRasterPos2f(start_x, start_y);
	glBitmap(fdo_bitmap_width, fdo_bitmap_height, 0, 0, 0, 0, fdo_bitmap);
	add_probe(&probes, "right glscissor clipped area", NULL,
		  start_x + fdo_bitmap_width * 3 /4,
		  start_y,
		  fdo_bitmap_width / 4,
		  fdo_bitmap_height,
		  0, 0);
	add_probe(&probes, "right glscissor unclipped area", green,
		  start_x, start_y,
		  fdo_bitmap_width * 3 / 4, fdo_bitmap_height,
		  0, 0);

	/* Top clipped */
	start_x = center_x_start;
	start_y = center_y_start + fdo_bitmap_height + 10;
	glScissor(start_x,
		  start_y,
		  fdo_bitmap_width,
		  fdo_bitmap_height * 3 / 4);
	glRasterPos2f(start_x, start_y);
	glBitmap(fdo_bitmap_width, fdo_bitmap_height, 0, 0, 0, 0, fdo_bitmap);
	add_probe(&probes, "top glscissor clipped area", NULL,
		  start_x,
		  start_y + fdo_bitmap_height * 3 /4,
		  fdo_bitmap_width,
		  fdo_bitmap_height / 4,
		  0, 0);
	add_probe(&probes, "top glscissor unclipped area", green,
		  start_x, start_y,
		  fdo_bitmap_width, fdo_bitmap_height * 3 / 4,
		  0, 0);

	/* Bottom clipped */
	start_x = center_x_start;
	start_y = center_y_start - fdo_bitmap_height - 10;
	glScissor(start_x,
		  start_y + fdo_bitmap_height / 4,
		  fdo_bitmap_width,
		  fdo_bitmap_height);
	glRasterPos2f(start_x, start_y);
	glBitmap(fdo_bitmap_width, fdo_bitmap_height, 0, 0, 0, 0, fdo_bitmap);
	add_probe(&probes, "bottom glscissor clipped area", NULL,
		  start_x, start_y,
		  fdo_bitmap_width, fdo_bitmap_height / 4,
		  0, 0);
	add_probe(&probes, "bottom glscissor unclipped area", green,
		  start_x, start_y + fdo_bitmap_height / 4,
		  fdo_bitmap_width, fdo_bitmap_height * 3 / 4,
		  0, fdo_bitmap_height / 4);

	glDisable(GL_SCISSOR_TEST);
	glColor4fv(blue);
	/* Left side of drawable (not drawn due to invalid pos) */
	start_x = -fdo_bitmap_width / 4;
	start_y = center_y_start;
	glRasterPos2f(start_x, start_y);
	glBitmap(fdo_bitmap_width, fdo_bitmap_height, 0, 0, 0, 0, fdo_bitmap);
	add_probe(&probes, "left drawable clipped area", NULL,
		  start_x + fdo_bitmap_width / 4, start_y,
		  fdo_bitmap_width * 3 / 4, fdo_bitmap_height,
		  fdo_bitmap_width / 4, 0);
	/* Right side of drawable */
	start_x = drawable_width - fdo_bitmap_width * 3 / 4;
	start_y = center_y_start;
	glRasterPos2f(start_x, start_y);
	glBitmap(fdo_bitmap_width, fdo_bitmap_height, 0, 0, 0, 0, fdo_bitmap);
	add_probe(&probes, "right drawable unclipped area", blue,
		  start_x, start_y,
		  fdo_bitmap_width * 3 / 4, fdo_bitmap_height,
		  0, 0);

	/* Top of drawable */
	start_x = center_x_start;
	start_y = drawable_height - fdo_bitmap_height * 3 / 4;
	glRasterPos2f(start_x, start_y);
	glBitmap(fdo_bitmap_width, fdo_bitmap_height, 0, 0, 0, 0, fdo_bitmap);
	add_probe(&probes, "top drawable unclipped area", blue,
		  start_x, start_y,
		  fdo_bitmap_width, fdo_bitmap_height * 3 / 4,
		  0, 0);
	/* Bottom of drawable (not drawn due to invalid pos) */
	start_x = center_x_start;
	start_y = -fdo_bitmap_height / 4;
	glRasterPos2f(start_x, start_y);
	glBitmap(fdo_bitmap_width, fdo_bitmap_height, 0, 0, 0, 0, fdo_bitmap);
	add_probe(&probes, "bottom drawable clipped area", NULL,
		  start_x, start_y,
		  fdo_bitmap_width, fdo_bitmap_height * 3 / 4,
		  0, 0);

	for (i = 0; i < probes.n_probes; i++)
		pass &= verify_bitmap_contents(&probes,
					       drawable_width, drawable_height,
					       i);

	return pass;
}

static void display_texture_to_window(GLint texture)
{
	glViewport(0, 0, piglit_width, piglit_height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1.0, 0, 1.0, -1, 1);

	/* Draw the result to the screen for inspection */
	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.0, 0.0); glVertex2f(0.0, 0.0);
	glTexCoord2f(1.0, 0.0); glVertex2f(1.0, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex2f(1.0, 1.0);
	glTexCoord2f(0.0, 1.0); glVertex2f(0.0, 1.0);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	piglit_present_results();
}

enum piglit_result
piglit_display()
{
	GLboolean pass = GL_TRUE;

	/* Use different width/height from window to catch mistakes */
	int fbo_width = 512;
	int fbo_height = 512;
	GLuint fbo, texture;

	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fbo_width, fbo_height,
		     0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D, texture, 0);

	pass &= draw_and_test("FBO", fbo_width, fbo_height);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

	display_texture_to_window(texture);

	glDeleteTextures(1, &texture);
	glDeleteFramebuffersEXT(1, &fbo);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	piglit_require_extension("GL_EXT_framebuffer_object");
}
