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
 */

/**
 * Test that glCopyTexImage() into a texture with a texture border
 * gets correct non-border texels.
 */


#include "piglit-util-gl.h"

/* Size of the body of the texture, not counting border. */
#define TEX_SIZE 64

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = TEX_SIZE*2+30;
	config.window_height = TEX_SIZE+20;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	int quad_w = TEX_SIZE / 2;
	int quad_h = TEX_SIZE / 2;
	float red[]   = {1.0, 0.0, 0.0, 0.0};
	float green[] = {0.0, 1.0, 0.0, 0.0};
	float blue[]  = {0.0, 0.0, 1.0, 0.0};
	float white[] = {1.0, 1.0, 1.0, 0.0};
	GLuint tex;
	int x, y;

	piglit_ortho_projection(piglit_width, piglit_height, false);

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	x = 10;
	y = 10;

	glColor4fv(red);
	piglit_draw_rect(x, y, quad_w, quad_h);
	glColor4fv(green);
	piglit_draw_rect(x + quad_w, y, quad_w, quad_h);
	glColor4fv(blue);
	piglit_draw_rect(x, y + quad_h, quad_w, quad_h);
	glColor4fv(white);
	piglit_draw_rect(x + quad_w, y + quad_h, quad_w, quad_h);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	/* Copy the rectangle drawn to our texture, with a 1-pixel
	 * border around it.
	 */
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
			 x - 1, y - 1,
			 TEX_SIZE + 2, TEX_SIZE + 2,
			 1);

	x = 20 + TEX_SIZE;
	y = 10;

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	piglit_draw_rect_tex(x, y, TEX_SIZE, TEX_SIZE,
			     0, 0, 1, 1);
	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &tex);

	pass = piglit_probe_rect_rgba(x, y,
				      quad_w, quad_h, red) && pass;
	pass = piglit_probe_rect_rgba(x + quad_w, y,
				      quad_w, quad_h, green) && pass;
	pass = piglit_probe_rect_rgba(x, y + quad_h,
				      quad_w, quad_h, blue) && pass;
	pass = piglit_probe_rect_rgba(x + quad_w, y + quad_h,
				      quad_w, quad_h, white) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
}
