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

// author: Ben Holmes

/*
 * This test draws depth textures as LUMINANCE, INTENSITY, and ALPHA. These
 * textures are compared to the r component of the texture coordinate and
 * compared using all eight texture compare functions. The result of the
 * comparison is modulated with the vertex color (pink) and blended with the
 * clear color (green) using the alpha value.
 */

#include "piglit-util-gl.h"

#define BOX_SIZE 24

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 400;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static GLuint tex;

void
piglit_init(int argc, char **argv)
{
	#define height 2
	#define width 2
	int i, j;

	GLfloat texDepthData[width][height];

	(void) argc;
	(void) argv;

	piglit_require_extension("GL_ARB_depth_texture");
	piglit_require_extension("GL_ARB_shadow");
	piglit_require_extension("GL_EXT_shadow_funcs");
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0, 1.0, 0.0, 1.0);

	for (i=0; i < width; ++i) {
		for (j=0; j < height; ++j) {
			if ((i+j) & 1) {
				texDepthData[i][j] = 1.0;
			}
			else {
				texDepthData[i][j] = 0.5;
			}
		}
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
			GL_COMPARE_R_TO_TEXTURE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, texDepthData);
	#undef height
	#undef width
}


enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;

	static const GLfloat pink[3] = {1.0, 0.0, 1.0};
	static const GLfloat white[3] = {1.0, 1.0, 1.0};
	static const GLfloat black[3] = {0.0, 0.0, 0.0};
	static const GLfloat green[3] = {0.0, 1.0, 0.0};

	static const struct {
		GLenum compare;
		float r0;
		float r1;
		const GLfloat *probes[12];
	} tests[] = {
		{
			GL_LESS,     2.0, 0.0,
			{
				pink, white, white, white,
				pink, black, black, black,
				pink, green, green, green,
			}
		},
		{
			GL_LEQUAL,   2.0, 0.0,
			{
				pink, white, pink, pink,
				pink, black, pink, pink,
				pink, green, pink, pink,
			}
		},
		{
			GL_GREATER,  2.0, 0.0,
			{
				white, pink, white, white,
				black, pink, black, black,
				green, pink, green, green,
			}
		},
		{
			GL_GEQUAL,   2.0, 0.0,
			{
				white, pink, pink, pink,
				black, pink, pink, pink,
				green, pink, pink, pink,
			}
		},
		{
			GL_ALWAYS,   2.0, 0.0,
			{
				pink, pink, pink, pink,
				pink, pink, pink, pink,
				pink, pink, pink, pink,
			}
		},
		{
			GL_NEVER,    2.0, 0.0,
			{
				white, white, white, white,
				black, black, black, black,
				green, green, green, green,
			}
		},
		{
			GL_NOTEQUAL, 2.0, 0.0,
			{
				pink, pink, white, white,
				pink, pink, black, black,
				pink, pink, green, green,
			}
		},
		{
			GL_EQUAL,    2.0, 0.0,
			{
				white, white, pink, pink,
				black, black, pink, pink,
				green, green, pink, pink,
			}
		},
	};

	static const GLenum modes[3] = {
		GL_ALPHA, GL_LUMINANCE, GL_INTENSITY
	};

	unsigned row;
	unsigned col;

	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(1.0, 0.0, 1.0);

	glBindTexture(GL_TEXTURE_2D, tex);
	for (row = 0; row < ARRAY_SIZE(tests); row++) {
		const int y = 275 - (35 * row);
		const GLenum compare = tests[row].compare;

		for (col = 0; col < 3; col++) {
			const int x = 20 + (col * 50);
			unsigned i;

			glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_COMPARE_FUNC,
					compare);
			glTexParameteri(GL_TEXTURE_2D,
					GL_DEPTH_TEXTURE_MODE,
					modes[col]);

			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord3f(1.0, 0.0, tests[row].r0);
			glVertex2f(x + BOX_SIZE, y);
			glTexCoord3f(1.0, 1.0, tests[row].r0);
			glVertex2f(x + BOX_SIZE, y + BOX_SIZE);
			glTexCoord3f(0.0, 0.0, tests[row].r1);
			glVertex2f(x,            y);
			glTexCoord3f(0.0, 1.0, tests[row].r1);
			glVertex2f(x,            y + BOX_SIZE);
			glEnd();

			for (i = 0; i < 4; i++) {
				const GLfloat *const color =
					tests[row].probes[(4 * col) + i];

				if (!piglit_probe_pixel_rgb(x + 3 + (i * 6),
							    y + 10,
							    color)) {
					if (!piglit_automatic) {
						printf("compare = %s, mode = %s\n",
						       piglit_get_gl_enum_name(compare),
						       piglit_get_gl_enum_name(modes[col]));
					}

					pass = GL_FALSE;
				}
			}
		}
	}

	piglit_present_results();

	printf("Left to Right: ALPHA, LUMINANCE, INTENSITY\n");
	printf("Top to Bottom: LESS, LEQUAL, GREATER, GEQUAL, "
	       "ALWAYS, NEVER, NOTEQUAL, EQUAL\n");

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
