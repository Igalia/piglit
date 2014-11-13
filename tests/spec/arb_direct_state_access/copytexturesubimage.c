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
 *    Laura Ekstrand <laura@jlekstrand.net> adapted this for testing
 *    CopyTextureSubImage* from ARB_direct_state_access.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 200;
	config.window_height = 200;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

/** Should GL_TEXTURE_RECTANGLE_ARB be tested? */
int have_rect = 0;

/** Should non-power-of-two textures be tested? */
int have_NPOT = 0;

static void rect(int x1, int y1, int x2, int y2)
{
	glBegin(GL_POLYGON);
	glVertex2f(x1, y1);
	glVertex2f(x1, y2);
	glVertex2f(x2, y2);
	glVertex2f(x2, y1);
	glEnd();
}
static GLboolean inrect(int x, int y, int x1, int y1, int x2, int y2)
{
	if (x >= x1 && x < x2 && y >= y1 && y < y2)
		return GL_TRUE;
	else
		return GL_FALSE;
}

static GLboolean
check_results(int dstx, int dsty, int w, int h)
{
	GLfloat *results;
	GLboolean pass = GL_TRUE;
	int x, y;

	results = malloc(w * h * 4 * sizeof(GLfloat));

	/* Check the results */
	glReadPixels(dstx, dsty, w, h, GL_RGBA, GL_FLOAT, results);
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			GLfloat expected[3];

			if (inrect(x, y, 5, h/2, w - 5, h - 5)) {
				expected[0] = 0.0;
				expected[1] = 0.0;
				expected[2] = 1.0;
			} else if (inrect(x, y, 5, 5, w - 5, h/2)) {
				expected[0] = 0.0;
				expected[1] = 1.0;
				expected[2] = 0.0;
			} else {
				expected[0] = 1.0;
				expected[1] = 0.0;
				expected[2] = 0.0;
			}

			if (results[(y * w + x) * 4 + 0] != expected[0] ||
			    results[(y * w + x) * 4 + 1] != expected[1] ||
			    results[(y * w + x) * 4 + 2] != expected[2]) {
				printf("Expected at (%d,%d): %f,%f,%f\n",
				       x, y,
				       expected[0], expected[1], expected[2]);
				printf("Probed at   (%d,%d): %f,%f,%f\n",
				       x, y,
				       results[(y * w + x) * 4 + 0],
				       results[(y * w + x) * 4 + 1],
				       results[(y * w + x) * 4 + 2]);
				pass = GL_FALSE;
			}
		}
	}

	free(results);
	return pass;
}

static GLboolean
do_row(int srcy, int srcw, int srch, GLenum target)
{
	int srcx = 20;
	int dstx = 80, dsty = srcy;
	int dstx2 = 140, dsty2 = srcy;
	int remain_width;
	int remain_height;
	GLuint texname;
	GLboolean pass = GL_TRUE;

	/* Rectangle textures use coordinates on the range [0..w]x[0..h],
	 * where as all other textures use coordinates on the range
	 * [0..1]x[0..1].
	 */
	const GLfloat tex_s_max = (target == GL_TEXTURE_RECTANGLE_ARB)
		? (float)srcw : 1.0;
	const GLfloat tex_t_max = (target == GL_TEXTURE_RECTANGLE_ARB)
		? (float)srch : 1.0;


	/* Draw the object we're going to copy */
	glColor3f(1.0, 0.0, 0.0);
	rect(srcx, srcy, srcx + srcw, srcy + srch);
	glColor3f(0.0, 1.0, 0.0);
	rect(srcx + 5, srcy + 5, srcx + srcw - 5, srcy + srch/2);
	glColor3f(0.0, 0.0, 1.0);
	rect(srcx + 5, srcy + srch/2, srcx + srcw - 5, srcy + srch - 5);

	/* Create a texture image and copy it in */
	glGenTextures(1, &texname);
	glBindTexture(target, texname);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	/* The default mode is GL_REPEAT, and this mode is invalid for
	 * GL_TEXTURE_RECTANGLE_ARB textures.
	 */
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(target);

	glTexImage2D(target, 0, GL_RGBA8, srcw, srch, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glCopyTextureSubImage2D(texname, 0,
				0, 0, /* offset in image */
				srcx, srcy, /* offset in readbuffer */
				srcw, srch);

	/* Draw the texture image out */
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0);
	glVertex2f(dstx, dsty);

	glTexCoord2f(0.0, tex_t_max);
	glVertex2f(dstx, dsty + srch);

	glTexCoord2f(tex_s_max, tex_t_max);
	glVertex2f(dstx + srcw, dsty + srch);

	glTexCoord2f(tex_s_max, 0.0);
	glVertex2f(dstx + srcw, dsty);
	glEnd();

	glTexImage2D(target, 0, GL_RGBA8, srcw, srch, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	remain_width = srcw - (srcw / 2);
	remain_height = srch - (srch / 2);
	glCopyTextureSubImage2D(texname, 0,
				0, 0, /* offset in image */
				srcx, srcy, /* offset in readbuffer */
				srcw / 2, srch / 2);
	glCopyTextureSubImage2D(texname, 0,
				srcw / 2, 0, /* offset in image */
				srcx + srcw / 2, srcy, /* " in readbuffer */
				remain_width, srch / 2);
	glCopyTextureSubImage2D(texname, 0,
				0, srch / 2, /* offset in image */
				srcx, srcy + srch / 2, /* " in readbuffer */
				srcw / 2, remain_height);
	glCopyTextureSubImage2D(texname, 0,
				srcw / 2, srch / 2, /* offset in image */
				srcx + srcw / 2, srcy + srch / 2, /*" in rb */
				remain_width, remain_height);

	/* Draw the texture image out */
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0);
	glVertex2f(dstx2, dsty2);

	glTexCoord2f(0.0, tex_t_max);
	glVertex2f(dstx2, dsty2 + srch);

	glTexCoord2f(tex_s_max, tex_t_max);
	glVertex2f(dstx2 + srcw, dsty2 + srch);

	glTexCoord2f(tex_s_max, 0.0);
	glVertex2f(dstx2 + srcw, dsty2);
	glEnd();

	glDisable(target);
	glDeleteTextures(1, &texname);

	printf("Checking %s, rect 1:\n", piglit_get_gl_enum_name(target));
	pass &= check_results(dstx, dsty, srcw, srch);
	printf("Checking %s, rect 2:\n", piglit_get_gl_enum_name(target));
	pass &= check_results(dstx2, dsty2, srcw, srch);

	return pass;
}


enum piglit_result
piglit_display(void)
{
	GLboolean pass;
	int srcy = 5;


	glClear(GL_COLOR_BUFFER_BIT);


	/* Test plain old 2D textures.
	 */
	pass = do_row(srcy, 32, 32, GL_TEXTURE_2D);
	srcy += 33 + 5;


	/* Test non-power-of-two 2D textures.
	 */
	if (have_NPOT) {
		pass &= do_row(srcy, 31, 13, GL_TEXTURE_2D);
		srcy += 15;
		pass &= do_row(srcy, 11, 34, GL_TEXTURE_2D);
		srcy += 35 + 5;
	}


	/* Test non-power-of-two 2D textures.
	 */
	if (have_rect) {
		pass &= do_row(srcy, 31, 13, GL_TEXTURE_RECTANGLE_ARB);
		srcy += 14;
		pass &= do_row(srcy, 11, 34, GL_TEXTURE_RECTANGLE_ARB);
		srcy += 35 + 5;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_direct_state_access");

	glDisable(GL_DITHER);

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( 0, piglit_width, 0, piglit_height, -1, 1 );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	glClearColor(0.5, 0.5, 0.5, 1.0);

	have_NPOT = (piglit_get_gl_version() >= 20
		|| (piglit_is_extension_supported("GL_ARB_texture_non_power_of_two")));

	have_rect = ((piglit_is_extension_supported("GL_ARB_texture_rectangle"))
		|| (piglit_is_extension_supported("GL_EXT_texture_rectangle"))
		|| (piglit_is_extension_supported("GL_NV_texture_rectangle")));
}
