/*
 * Copyright (c) The Piglit project 2007
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

#if defined(_WIN32)
#include <windows.h>
#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "piglit-util-gl-common.h"

/**
 * Generates a texture with the given internalFormat, w, h with a
 * teximage of r, g, b w quadrants.
 *
 * Note that for compressed teximages, where the blocking would be
 * problematic, we assign the whole layers at w == 4 to red, w == 2 to
 * green, and w == 1 to blue.
 */
GLuint
piglit_rgbw_texture(GLenum format, int w, int h, GLboolean mip,
		    GLboolean alpha, GLenum basetype)
{
	GLubyte *data;
	int size, x, y, level;
	GLuint tex;
	GLubyte red[4]   = {255, 0, 0, 0};
	GLubyte green[4] = {0, 255, 0, 64};
	GLubyte blue[4]  = {0, 0, 255, 128};
	GLubyte white[4] = {255, 255, 255, 255};

	if (!alpha) {
		red[3] = 255;
		green[3] = 255;
		blue[3] = 255;
		white[3] = 255;
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (mip) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_NEAREST);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
	}
	data = malloc(w * h * 4 * sizeof(GLubyte));

	/* XXX: Do we want non-square textures?  Surely some day. */
	assert(w == h);

	for (level = 0, size = w; size > 0; level++, size >>= 1) {
		for (y = 0; y < size; y++) {
			for (x = 0; x < size; x++) {
				const GLubyte *color;

				if (x < size / 2 && y < size / 2)
					color = red;
				else if (y < size / 2)
					color = green;
				else if (x < size / 2)
					color = blue;
				else
					color = white;

				memcpy(data + (y * size + x) * 4, color,
				       4 * sizeof(GLubyte));
			}
		}
		glTexImage2D(GL_TEXTURE_2D, level,
			     format,
			     size, size, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, data);

		if (!mip)
			break;

		if (w > 1)
			w >>= 1;
		if (h > 1)
			h >>= 1;
	}
	free(data);
	return tex;
}
