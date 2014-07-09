/*
 * Copyright (c) The Piglit project 2008
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 * Test whether 1D textures correctly ignore the T coordinate wrap mode.
 *
 * Since 1D textures are genuine one-dimensional objects, the T coordinate
 * shouldn't affect them at all. However, R300 simulates them as flat
 * 2D textures, which caused incorrect sampling of border colors.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 256;
	config.window_height = 128;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const GLfloat TextureColor[3] = { 1.0, 0.5, 0.0 };


static GLboolean test(GLenum wrapt, int cellx, int celly)
{
	int sx, sy;

	glPushMatrix();
	glTranslatef(cellx*0.25, celly*0.5, 0.0);
	glScalef(0.25, 0.5, 1.0);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, wrapt);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex2f(0, 0);
		glTexCoord2f(1, 0); glVertex2f(1, 0);
		glTexCoord2f(1, 1); glVertex2f(1, 1);
		glTexCoord2f(0, 1); glVertex2f(0, 1);
	glEnd();
	glPopMatrix();

	glReadBuffer(GL_BACK);

	/* Take more than one sample, just to be sure */
	for(sy = 0; sy < 4; ++sy) {
		for(sx = 0; sx < 4; ++sx) {
			int x = (cellx*5 + sx + 1)*piglit_width/20;
			int y = (celly*5 + sy + 1)*piglit_height/10;

			if (!piglit_probe_pixel_rgb(x, y, TextureColor)) {
				fprintf(stderr, "Fail in cell %i,%i (texwrap = 0x%x)\n", cellx, celly, wrapt);

				return GL_FALSE;
			}
		}
	}

	return GL_TRUE;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

        /* Draw eight tiles, each with a different tex wrap mode.
         * They should all look the same.
         */
	pass &= test(GL_REPEAT, 0, 0);
	pass &= test(GL_CLAMP, 1, 0);
	pass &= test(GL_CLAMP_TO_EDGE, 2, 0);
	pass &= test(GL_CLAMP_TO_BORDER, 3, 0);
	pass &= test(GL_MIRRORED_REPEAT, 0, 1);
	if (piglit_is_extension_supported("GL_EXT_texture_mirror_clamp")) {
		pass &= test(GL_MIRROR_CLAMP_EXT, 1, 1);
		pass &= test(GL_MIRROR_CLAMP_TO_EDGE_EXT, 2, 1);
		pass &= test(GL_MIRROR_CLAMP_TO_BORDER_EXT, 3, 1);
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 1, 0, GL_RGB, GL_FLOAT, TextureColor);
	glEnable(GL_TEXTURE_1D);

	piglit_ortho_projection(1.0, 1.0, GL_FALSE);

	if (!piglit_automatic)
		printf("You should see a flat orange color\n");
}
