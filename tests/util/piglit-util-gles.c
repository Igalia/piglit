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
 * Read a pixel from the given location and compare its RGBA value to the
 * given expected values.
 *
 * Print a log message if the color value deviates from the expected value.
 * \return true if the color values match, false otherwise
 */
int piglit_probe_pixel_rgba(int x, int y, const float* expected)
{
	GLubyte probe[4];
	int i;
	GLboolean pass = GL_TRUE;

	glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, probe);

	for(i = 0; i < 4; ++i) {
		if (fabs(probe[i]/255.0 - expected[i]) > piglit_tolerance[i]) {
			pass = GL_FALSE;
		}
	}

	if (pass)
		return 1;

	printf("Probe color at (%i,%i)\n", x, y);
	printf("  Expected: %f %f %f %f\n", expected[0], expected[1], expected[2], expected[3]);
	printf("  Observed: %f %f %f %f\n", probe[0]/255.0, probe[1]/255.0, probe[2]/255.0, probe[3]/255.0);

	return 0;
}

int
piglit_probe_rect_rgba(int x, int y, int w, int h, const float *expected)
{
	int i, j, p;
	GLubyte *probe;
	GLubyte *pixels = malloc(w*h*4*sizeof(GLubyte));

	glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			probe = &pixels[(j*w+i)*4];

			for (p = 0; p < 4; ++p) {
				if (fabs(probe[p]/255.0 - expected[p]) >= piglit_tolerance[p]) {
					printf("Probe color at (%i,%i)\n", x+i, y+j);
					printf("  Expected: %f %f %f %f\n",
					       expected[0], expected[1], expected[2], expected[3]);
					printf("  Observed: %f %f %f %f\n",
					       probe[0]/255.0, probe[1]/255.0, probe[2]/255.0, probe[3]/255.0);

					free(pixels);
					return 0;
				}
			}
		}
	}

	free(pixels);
	return 1;
}

/**
 * Read a pixel from the given location and compare its RGB value to the
 * given expected values.
 *
 * Print a log message if the color value deviates from the expected value.
 * \return true if the color values match, false otherwise
 */
int piglit_probe_pixel_rgb(int x, int y, const float* expected)
{
	GLubyte probe[3];
	int i;
	GLboolean pass = GL_TRUE;

	glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, probe);


	for(i = 0; i < 3; ++i) {
		if (fabs(probe[i]/255.0 - expected[i]) > piglit_tolerance[i]) {
			pass = GL_FALSE;
		}
	}

	if (pass)
		return 1;

	printf("Probe color at (%i,%i)\n", x, y);
	printf("  Expected: %f %f %f\n", expected[0], expected[1], expected[2]);
	printf("  Observed: %f %f %f\n", probe[0]/255.0, probe[1]/255.0, probe[2]/255.0);

	return 0;
}

int
piglit_probe_rect_rgb(int x, int y, int w, int h, const float *expected)
{
	int i, j, p;
	GLubyte *probe;
	GLubyte *pixels = malloc(w*h*3*sizeof(GLubyte));

	glReadPixels(x, y, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			probe = &pixels[(j*w+i)*3];

			for (p = 0; p < 3; ++p) {
				if (fabs(probe[p]/255.0 - expected[p]) >= piglit_tolerance[p]) {
					printf("Probe color at (%i,%i)\n", x+i, y+j);
					printf("  Expected: %f %f %f\n",
					       expected[0], expected[1], expected[2]);
					printf("  Observed: %f %f %f\n",
					       probe[0]/255.0, probe[1]/255.0, probe[2]/255.0);

					free(pixels);
					return 0;
				}
			}
		}
	}

	free(pixels);
	return 1;
}

void
piglit_escape_exit_key(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;
	switch (key) {
		case 27:
			exit(0);
			break;
	}
}

/**
 * Call glDrawArrays.  verts is expected to be
 *
 *   float verts[4][4];
 *
 * if not NULL; tex is expected to be
 *
 *   float tex[4][2];
 *
 * if not NULL.
 */
static void
draw_arrays(const GLvoid *verts, const GLvoid *tex)
{
#if defined(PIGLIT_USE_OPENGL_ES1)
	if (verts) {
		glVertexPointer(4, GL_FLOAT, 0, verts);
		glEnableClientState(GL_VERTEX_ARRAY);
	}

	if (tex) {
		glTexCoordPointer(2, GL_FLOAT, 0, tex);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	if (verts)
		glDisableClientState(GL_VERTEX_ARRAY);
	if (tex)
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#elif defined(PIGLIT_USE_OPENGL_ES2) ||defined(PIGLIT_USE_OPENGL_ES3)
	if (verts) {
		glVertexAttribPointer(PIGLIT_ATTRIB_POS, 4, GL_FLOAT, GL_FALSE, 0, verts);
		glEnableVertexAttribArray(PIGLIT_ATTRIB_POS);
	}

	if (tex) {
		glVertexAttribPointer(PIGLIT_ATTRIB_TEX, 2, GL_FLOAT, GL_FALSE, 0, tex);
		glEnableVertexAttribArray(PIGLIT_ATTRIB_TEX);
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	if (verts)
		glDisableVertexAttribArray(PIGLIT_ATTRIB_POS);
	if (tex)
		glDisableVertexAttribArray(PIGLIT_ATTRIB_TEX);
#else
#	error "don't know how to draw arrays"
#endif
}

/**
 * Convenience function to draw an axis-aligned rectangle.
 */
GLvoid
piglit_draw_rect(float x, float y, float w, float h)
{
	float verts[4][4];

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = 0.0;
	verts[0][3] = 1.0;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = 0.0;
	verts[1][3] = 1.0;
	verts[2][0] = x;
	verts[2][1] = y + h;
	verts[2][2] = 0.0;
	verts[2][3] = 1.0;
	verts[3][0] = x + w;
	verts[3][1] = y + h;
	verts[3][2] = 0.0;
	verts[3][3] = 1.0;

	draw_arrays(verts, NULL);
}


/**
 * Convenience function to draw an axis-aligned backfaced rectangle.
 */
GLvoid
piglit_draw_rect_back(float x, float y, float w, float h)
{
	float verts[4][4];

	verts[0][0] = x + w;
	verts[0][1] = y;
	verts[0][2] = 0.0;
	verts[0][3] = 1.0;
	verts[1][0] = x;
	verts[1][1] = y;
	verts[1][2] = 0.0;
	verts[1][3] = 1.0;
	verts[2][0] = x + w;
	verts[2][1] = y + h;
	verts[2][2] = 0.0;
	verts[2][3] = 1.0;
	verts[3][0] = x;
	verts[3][1] = y + h;
	verts[3][2] = 0.0;
	verts[3][3] = 1.0;

	draw_arrays(verts, NULL);
}


/**
 * Convenience function to draw an axis-aligned rectangle.
 */
GLvoid
piglit_draw_rect_z(float z, float x, float y, float w, float h)
{
	float verts[4][4];

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = z;
	verts[0][3] = 1.0;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = z;
	verts[1][3] = 1.0;
	verts[2][0] = x;
	verts[2][1] = y + h;
	verts[2][2] = z;
	verts[2][3] = 1.0;
	verts[3][0] = x + w;
	verts[3][1] = y + h;
	verts[3][2] = z;
	verts[3][3] = 1.0;

	draw_arrays(verts, NULL);
}

/**
 * Convenience function to draw an axis-aligned rectangle
 * with texture coordinates.
 */
GLvoid
piglit_draw_rect_tex(float x, float y, float w, float h,
                     float tx, float ty, float tw, float th)
{
	float verts[4][4];
	float tex[4][2];

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = 0.0;
	verts[0][3] = 1.0;
	tex[0][0] = tx;
	tex[0][1] = ty;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = 0.0;
	verts[1][3] = 1.0;
	tex[1][0] = tx + tw;
	tex[1][1] = ty;
	verts[2][0] = x;
	verts[2][1] = y + h;
	verts[2][2] = 0.0;
	verts[2][3] = 1.0;
	tex[2][0] = tx;
	tex[2][1] = ty + th;
	verts[3][0] = x + w;
	verts[3][1] = y + h;
	verts[3][2] = 0.0;
	verts[3][3] = 1.0;
	tex[3][0] = tx + tw;
	tex[3][1] = ty + th;

	draw_arrays(verts, tex);
}

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


/**
 * Generate a checkerboard texture
 *
 * \param tex                Name of the texture to be used.  If \c tex is
 *                           zero, a new texture name will be generated.
 * \param level              Mipmap level the checkerboard should be written to
 * \param width              Width of the texture image
 * \param height             Height of the texture image
 * \param horiz_square_size  Size of each checkerboard tile along the X axis
 * \param vert_square_size   Size of each checkerboard tile along the Y axis
 * \param black              RGBA color to be used for "black" tiles
 * \param white              RGBA color to be used for "white" tiles
 *
 * A texture with alternating black and white squares in a checkerboard
 * pattern is generated.  The texture data is written to LOD \c level of
 * the texture \c tex.
 *
 * If \c tex is zero, a new texture created.  This texture will have several
 * texture parameters set to non-default values:
 *
 *  - S and T wrap modes will be set to \c GL_CLAMP_TO_BORDER.
 *  - Border color will be set to { 1.0, 0.0, 0.0, 1.0 }.
 *  - Min and mag filter will be set to \c GL_NEAREST.
 *
 * \return
 * Name of the texture.  In addition, this texture will be bound to the
 * \c GL_TEXTURE_2D target of the currently active texture unit.
 */
GLuint
piglit_checkerboard_texture(GLuint tex, unsigned level,
			    unsigned width, unsigned height,
			    unsigned horiz_square_size,
			    unsigned vert_square_size,
			    const float *black, const float *white)
{
	unsigned i;
	unsigned j;

	GLubyte *const tex_data = malloc(width * height * (4 * sizeof(GLubyte)));
	GLubyte *texel = tex_data;

	for (i = 0; i < height; i++) {
		const unsigned row = i / vert_square_size;

		for (j = 0; j < width; j++) {
			const unsigned col = j / horiz_square_size;

			if ((row ^ col) & 1) {
				texel[0] = white[0] * 255;
				texel[1] = white[1] * 255;
				texel[2] = white[2] * 255;
				texel[3] = white[3] * 255;
			} else {
				texel[0] = black[0] * 255;
				texel[1] = black[1] * 255;
				texel[2] = black[2] * 255;
				texel[3] = black[3] * 255;
			}

			texel += 4;
		}
	}


	if (tex == 0) {
		glGenTextures(1, &tex);

		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
				GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
				GL_CLAMP_TO_EDGE);
	} else {
		glBindTexture(GL_TEXTURE_2D, tex);
	}

	glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, width, height, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, tex_data);

	return tex;
}

#if defined(PIGLIT_USE_OPENGL_ES1)

/**
 * Convenience function to configure an abitrary orthogonal projection matrix
 */
void
piglit_gen_ortho_projection(double left, double right, double bottom,
			    double top, double near_val, double far_val,
			    GLboolean push)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (push)
		glPushMatrix();
	glOrthof(left, right, bottom, top, near_val, far_val);

	glMatrixMode(GL_MODELVIEW);
	if (push)
		glPushMatrix();
	glLoadIdentity();
}


/**
 * Convenience function to configure projection matrix for window coordinates
 */
void
piglit_ortho_projection(int w, int h, GLboolean push)
{
	/* Set up projection matrix so we can just draw using window
	 * coordinates.
	 */
	piglit_gen_ortho_projection(0, w, 0, h, -1, 1, push);
}

#endif /* PIGLIT_USE_OPENGL_ES1 */
