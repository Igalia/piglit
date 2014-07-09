/*
 * Copyright © 2009 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file depth-tex-modes-common.c
 * Common framework for tests of GL_DEPTH_TEXTURE_MODE.
 *
 * \author Ben Holmes
 * \author Ian Romanick
 */

#include "piglit-util-gl.h"
#include "depth-tex-modes-common.h"

static void loadTex(void);

void
depth_tex_init(void)
{
	piglit_require_extension("GL_ARB_depth_texture");
	piglit_require_extension("GL_ARB_texture_rectangle");
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(0.2, 0.2, 0.2, 1.0);

	loadTex();
}

void
loadTex(void)
{
	#define height 2
	#define width 2
        int i, j;

	GLfloat texDepthData[width][height];

	for (i=0; i < width; ++i) {
		for (j=0; j < height; ++j) {
			if ((i+j) & 1) {
				texDepthData[i][j] = 1.0;
			}
			else {
				texDepthData[i][j] = 0.0;
			}
		}
	}

	glGenTextures(2, tex);

	// Depth texture 0: 2D
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, texDepthData);


	// Depth texture 1: rectangle
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex[1]);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_DEPTH_COMPONENT, width,
			height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, texDepthData);


	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_CONSTANT);

	#undef height
	#undef width
}


static void
calculate_expected_color(GLenum depth_mode, GLenum operand, 
			 const float *env_color, float texel,
			 float *result)
{
	float color[4];

	switch (depth_mode) {
	case GL_ALPHA:
		color[0] = 0.0;   color[1] = 0.0;   color[2] = 0.0;
		color[3] = texel;
		break;
	case GL_LUMINANCE:
		color[0] = texel; color[1] = texel; color[2] = texel;
		color[3] = 1.0;
		break;
	case GL_INTENSITY:
		color[0] = texel; color[1] = texel; color[2] = texel;
		color[3] = texel;
		break;
	case GL_RED:
		color[0] = texel; color[1] = 0.0;   color[2] = 0.0;
		color[3] = 1.0;
		break;
	default:
		assert(0);
		color[0] = color[1] = color[2] = color[3] = 0.0;
		break;
	}

	if (operand == GL_SRC_ALPHA) {
		color[0] = color[3];
		color[1] = color[3];
		color[2] = color[3];
	}

	result[0] = color[0] * env_color[0];
	result[1] = color[1] * env_color[1];
	result[2] = color[2] * env_color[2];
}


enum piglit_result
depth_tex_display(const GLenum *depth_texture_modes, unsigned num_modes,
		  unsigned box_size)
{
	static const GLfloat color2[4] = {0.0, 1.0, 0.0, 1.0};
	static const GLfloat color1[4] = {1.0, 0.0, 1.0, 1.0};
	const unsigned half = box_size / 2;
	const unsigned quarter = box_size / 4;

	GLboolean pass = GL_TRUE;
	unsigned i;
	unsigned row;

	static const struct {
		GLenum target;
		GLenum operand0_rgb;
		const GLfloat *color;
		float tex_size;
	} test_rows[4] = {
		{ GL_TEXTURE_RECTANGLE_ARB, GL_SRC_COLOR, color2, 2.0 },
		{ GL_TEXTURE_RECTANGLE_ARB, GL_SRC_ALPHA, color2, 2.0 },
		{ GL_TEXTURE_2D,            GL_SRC_COLOR, color1, 1.0 },
		{ GL_TEXTURE_2D,            GL_SRC_ALPHA, color1, 1.0 }
	};

	glClear(GL_COLOR_BUFFER_BIT);

	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);


	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex[1]);

	for (row = 0; row < ARRAY_SIZE(test_rows); row++) {
		const float y = 1.0 + ((box_size + 1)  * row);

		/* Disable both texture targets, then enable just the target
		 * used in this row.
		 */
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_RECTANGLE_ARB);
		glEnable(test_rows[row].target);

		glTexEnvfv(GL_TEXTURE_ENV, 
			   GL_TEXTURE_ENV_COLOR,
			   test_rows[row].color);
		glTexEnvi(GL_TEXTURE_ENV,
			  GL_OPERAND0_RGB,
			  test_rows[row].operand0_rgb);

		for (i = 0; i < num_modes; i++) {
			const float x = 1.0 + ((box_size + 1) * i);
			const GLenum mode = depth_texture_modes[i];
			unsigned j;

			glTexParameteri(test_rows[row].target,
					GL_DEPTH_TEXTURE_MODE,
					mode);

			piglit_draw_rect_tex(x, y, box_size, box_size,
					     0.0, 0.0,
					     test_rows[row].tex_size,
					     test_rows[row].tex_size);

			for (j = 0; j < 4; j++) {
				const float tx = x + quarter 
					+ ((j & 1) ? half : 0);
				const float ty = y + quarter
					+ ((j & 2) ? half : 0);
				float tc[3];

				calculate_expected_color(mode,
							 test_rows[row].operand0_rgb,
							 test_rows[row].color,
							 ((j == 0) || (j == 3))
							 ? 0.0 : 1.0,
							 tc);

				if (!piglit_probe_pixel_rgb(tx, ty, tc)) {
					pass = GL_FALSE;

					if (!piglit_automatic)
						printf("  Mode: 0x%04x\n",
						       mode);
				}
			}
		}
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
