/*
 * Copyright (c) 2004 Brian Paul
 * Copyright (c) 2011 VMware, Inc.
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

/*
 * Test packed pixel formats for textures.  Try a bunch of combinations
 * of various internal texture formats with combinations of packed user
 * formats/types.  Also test glPixelStore byte swapping.
 *
 * Based on mesa-demos/tests/packedpixels.c.
 * Brian Paul
 * June 2011
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

config.supports_gl_compat_version = 10;

config.window_width = 700;
config.window_height = 620;
config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

struct pixel_format {
	const char *name;
	GLenum format;
	GLenum type;
	GLint bytes;
	GLuint redTexel, greenTexel; /* with approx 51% alpha, when applicable */
};

static const struct pixel_format Formats[] = {

	{ "GL_RGBA/GL_UNSIGNED_INT_8_8_8_8",
	  GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 4, 0xff000080, 0x00ff0080 },
	{ "GL_RGBA/GL_UNSIGNED_INT_8_8_8_8_REV",
	  GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, 4, 0x800000ff, 0x8000ff00 },
	{ "GL_RGBA/GL_UNSIGNED_INT_10_10_10_2",
	  GL_RGBA, GL_UNSIGNED_INT_10_10_10_2, 4, 0xffc00002, 0x3ff002 },
	{ "GL_RGBA/GL_UNSIGNED_INT_2_10_10_10_REV",
	  GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, 4, 0xc00003ff, 0xc00ffc00 },
	{ "GL_RGBA/GL_UNSIGNED_SHORT_4_4_4_4",
	  GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 2, 0xf008, 0x0f08 },
	{ "GL_RGBA/GL_UNSIGNED_SHORT_4_4_4_4_REV",
	  GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4_REV, 2, 0x800f, 0x80f0 },
	{ "GL_RGBA/GL_UNSIGNED_SHORT_5_5_5_1",
	  GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 2, 0xf801, 0x7c1 },
	{ "GL_RGBA/GL_UNSIGNED_SHORT_1_5_5_5_REV",
	  GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, 2, 0x801f, 0x83e0 },

	{ "GL_BGRA/GL_UNSIGNED_INT_8_8_8_8",
	  GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, 4, 0x0000ff80, 0x00ff0080 },
	{ "GL_BGRA/GL_UNSIGNED_INT_8_8_8_8_REV",
	  GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, 4, 0x80ff0000, 0x8000ff00 },
	{ "GL_BGRA/GL_UNSIGNED_SHORT_4_4_4_4",
	  GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4, 2, 0x00f8, 0x0f08 },
	{ "GL_BGRA/GL_UNSIGNED_SHORT_4_4_4_4_REV",
	  GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4_REV, 2, 0x8f00, 0x80f0 },
	{ "GL_BGRA/GL_UNSIGNED_SHORT_5_5_5_1",
	  GL_BGRA, GL_UNSIGNED_SHORT_5_5_5_1, 2, 0x3f, 0x7c1 },
	{ "GL_BGRA/GL_UNSIGNED_SHORT_1_5_5_5_REV",
	  GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, 2, 0xfc00, 0x83e0 },

	{ "GL_RGB/GL_UNSIGNED_SHORT_5_6_5",
	  GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 2, 0xf800, 0x7e0 },
	{ "GL_RGB/GL_UNSIGNED_SHORT_5_6_5_REV",
	  GL_RGB, GL_UNSIGNED_SHORT_5_6_5_REV, 2, 0x1f, 0x7e0 },
	{ "GL_RGB/GL_UNSIGNED_BYTE_3_3_2",
	  GL_RGB, GL_UNSIGNED_BYTE_3_3_2, 1, 0xe0, 0x1c },
	{ "GL_RGB/GL_UNSIGNED_BYTE_2_3_3_REV",
	  GL_RGB, GL_UNSIGNED_BYTE_2_3_3_REV, 1, 0x7, 0x38 }
};

#define NUM_FORMATS (sizeof(Formats) / sizeof(Formats[0]))

struct name_format {
	const char *name;
	GLenum format;
};

static const struct name_format IntFormats[] = {
	{ "GL_RGBA", GL_RGBA },
	{ "GL_RGBA2", GL_RGBA2 },
	{ "GL_RGBA4", GL_RGBA4 },
	{ "GL_RGB5_A1", GL_RGB5_A1 },
	{ "GL_RGBA8", GL_RGBA8 },
	{ "GL_RGBA12", GL_RGBA12 },
	{ "GL_RGBA16", GL_RGBA16 },
	{ "GL_RGB10_A2", GL_RGB10_A2 },

	{ "GL_RGB", GL_RGB },
	{ "GL_R3_G3_B2", GL_R3_G3_B2 },
	{ "GL_RGB4", GL_RGB4 },
	{ "GL_RGB5", GL_RGB5 },
	{ "GL_RGB8", GL_RGB8 },
	{ "GL_RGB10", GL_RGB10 },
	{ "GL_RGB12", GL_RGB12 },
	{ "GL_RGB16", GL_RGB16 },

};

#define NUM_INT_FORMATS (sizeof(IntFormats) / sizeof(IntFormats[0]))

static void
MakeTexture(GLuint dims, const struct pixel_format *format,
            GLenum intFormat, GLboolean swap)
{
	GLubyte texBuffer[1000];
	int i;

	assert(dims == 2 || dims == 3);

	glPixelStorei(GL_UNPACK_SWAP_BYTES, swap);

	if (format->bytes == 1) {
		for (i = 0; i < 8; i++) {
			texBuffer[i] = format->redTexel;
		}
		for (i = 8; i < 16; i++) {
			texBuffer[i] = format->greenTexel;
		}
	}
	else if (format->bytes == 2) {
		GLushort *us = (GLushort *) texBuffer;
		for (i = 0; i < 8; i++) {
			us[i] = format->redTexel;
		}
		for (i = 8; i < 16; i++) {
			us[i] = format->greenTexel;
		}
		if (swap) {
			for (i = 0; i < 16; i++)
				us[i] = (us[i] << 8) | (us[i] >> 8);
		}
	}
	else if (format->bytes == 4) {
		GLuint *ui = (GLuint *) texBuffer;
		for (i = 0; i < 8; i++) {
			ui[i] = format->redTexel;
		}
		for (i = 8; i < 16; i++) {
			ui[i] = format->greenTexel;
		}
		if (swap) {
			for (i = 0; i < 16; i++) {
				GLuint b = ui[i];
				ui[i] =  (b >> 24)
					| ((b >> 8) & 0xff00)
					| ((b << 8) & 0xff0000)
					| ((b << 24) & 0xff000000);
			}
		}
	}
	else {
		abort();
	}

	if (dims == 3) {
		/* 4 x 4 x 4 texture, undefined data */
		glTexImage3D(GL_TEXTURE_3D, 0, intFormat, 4, 4, 4, 0,
			     format->format, format->type, NULL);
		/* fill in Z=1 and Z=2 slices with the real texture data */
		glTexSubImage3D(GL_TEXTURE_3D, 0,
				0, 0, 1,  /* offset */
				4, 4, 1,  /* size */
				format->format, format->type, texBuffer);
		glTexSubImage3D(GL_TEXTURE_3D, 0,
				0, 0, 2,  /* offset */
				4, 4, 1,  /* size */
				format->format, format->type, texBuffer);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, intFormat, 4, 4, 0,
			     format->format, format->type, texBuffer);
	}

	if (0) {
		GLint r, g, b, a, l, i;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, &r);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_GREEN_SIZE, &g);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE, &b);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE, &a);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_LUMINANCE_SIZE, &l);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTENSITY_SIZE, &i);
		printf("IntFormat: 0x%x  R %d  G %d  B %d  A %d  L %d  I %d\n",
		       intFormat, r, g, b, a, l, i);
		glGetError();
	}
}

/**
 * Test a particular internal texture format.  2D or 3D texture.
 */
static GLboolean
Test(GLuint intFmt, GLuint dims)
{
	static const float red[4] = {1, 0, 0, 1};
	static const float green[4] = {0, 1, 0, 1};
	int w = 4, h = 4;
	int i, swap;
	GLboolean pass = GL_TRUE;

	piglit_ortho_projection(piglit_width, piglit_height, false);
	assert(NUM_FORMATS * 5 < piglit_height);

	glClear(GL_COLOR_BUFFER_BIT);

	if (dims == 3)
		glEnable(GL_TEXTURE_3D);
	else
		glEnable(GL_TEXTURE_2D);

	/* Loop over byteswapping */
	for (swap = 0; swap < 2; swap++) {
		/* Loop over texture formats */
		for (i = 0; i < NUM_FORMATS; i++) {
			int x = 5 * swap;
			int y = 5 * i;

			MakeTexture(dims, Formats + i, IntFormats[intFmt].format,
				    swap);

			if (glGetError()) {
				printf("Unexpected GL Error for %s\n",
				       IntFormats[intFmt].name);
				return GL_FALSE;
			}

			glBegin(GL_POLYGON);
			glTexCoord3f(0, 0, 0.5);  glVertex2f(x + 0, y + 0);
			glTexCoord3f(1, 0, 0.5);  glVertex2f(x + w, y + 0);
			glTexCoord3f(1, 1, 0.5);  glVertex2f(x + w, y + h);
			glTexCoord3f(0, 1, 0.5);  glVertex2f(x + 0, y + h);
			glEnd();
		}
	}

	if (dims == 3)
		glDisable(GL_TEXTURE_3D);
	else
		glDisable(GL_TEXTURE_2D);

	/* Loop over byteswapping */
	for (swap = 0; swap < 2; swap++) {
		/* Loop over texture formats */
		for (i = 0; i < NUM_FORMATS; i++) {
			int x = 5 * swap;
			int y = 5 * i;
			/* test rendering */
			if (!piglit_probe_rect_rgb(x, y, w, 2, red)) {
				printf("Failure for format=%s, swap=%u, "
				       "textureDims=%u\n",
				       Formats[i].name, swap, dims);
				pass = GL_FALSE;
			}

			if (!piglit_probe_rect_rgb(x, y + 2, w, 2, green)) {
				printf("Failure for format=%s, swap=%u, "
				       "textureDims=%u\n",
				       Formats[i].name, swap, dims);
				pass = GL_FALSE;
			}
		}
	}

	piglit_present_results();

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = 1;
	int i;

	/* 2D texture */
	for (i = 0; i < NUM_INT_FORMATS; i++) {
		pass = Test(i, 2) && pass;
		if (!pass && !piglit_automatic)
			break;
	}

	/* 3D texture */
	for (i = 0; i < NUM_INT_FORMATS; i++) {
		pass = Test(i, 3) && pass;
		if (!pass && !piglit_automatic)
			break;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	if (0) {
		printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));
		printf("GL_VERSION = %s\n", (char *) glGetString(GL_VERSION));
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

