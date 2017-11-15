/*
 * Copyright (C) 1999  Allen Akin   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

/**
 * @file tex-env.c:  Test the basic texture env modes
 * Author: Brian Paul (brianp@valinux.com)  April 2001
 *
 * Test procedure:
 *   Setup a texture with 81 columns of unique RGBA colors, 3 texels each.
 *   Draw a 81 uniquely-colored flat-shaded quads as wide horizontal bands,
 *   with the above texture.  This makes a matrix of 81*81 colored squares
 *   for which we test that the current texture environment mode and texture
 *   format produced the correct color.
 *   Finally, we blend over a gray background in order to verify that the
 *   post-texture alpha value is correct.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
	config.window_width = 256;
	config.window_height = 256;

PIGLIT_GL_TEST_CONFIG_END

#define BLEND_WITH_BACKGROUND 1

#define COLORS (3 * 3 * 3 * 3)

static float colors[COLORS][4];

static float bg_color[4] = {0.5, 0.5, 0.5, 0.5};

static const GLenum format_enums[] = {
	GL_ALPHA,
	GL_LUMINANCE,
	GL_LUMINANCE_ALPHA,
	GL_INTENSITY,
	GL_RGB,
	GL_RGBA
};

static const GLenum env_mode_enums[] = {
	GL_REPLACE,
	GL_MODULATE,
	GL_DECAL,
	GL_BLEND,
	GL_ADD
};

/* Compute expected texenv result given the texture env mode, the texture
 * base format, texture color, fragment color, and texture env color.
 * This also blends the result with the background color if that option
 * is enabled (see above). */
static void
compute_expected_color(GLenum env_mode, GLenum tex_format,
		       const float tex_color[4], const float frag_color[4],
		       const float env_color[4], float result[4])
{
	switch (env_mode) {
	case GL_REPLACE:
		switch (tex_format) {
		case GL_ALPHA:
			result[0] = frag_color[0];
			result[1] = frag_color[1];
			result[2] = frag_color[2];
			result[3] = tex_color[3]; /* alpha */
			break;
		case GL_LUMINANCE:
			result[0] = tex_color[0]; /* lum */
			result[1] = tex_color[0];
			result[2] = tex_color[0];
			result[3] = frag_color[3];
			break;
		case GL_LUMINANCE_ALPHA:
			result[0] = tex_color[0]; /* lum */
			result[1] = tex_color[0];
			result[2] = tex_color[0];
			result[3] = tex_color[3]; /* alpha */
			break;
		case GL_INTENSITY:
			result[0] = tex_color[0]; /* intensity */
			result[1] = tex_color[0];
			result[2] = tex_color[0];
			result[3] = tex_color[0];
			break;
		case GL_RGB:
			result[0] = tex_color[0]; /* r */
			result[1] = tex_color[1]; /* g */
			result[2] = tex_color[2]; /* b */
			result[3] = frag_color[3];
			break;
		case GL_RGBA:
			result[0] = tex_color[0]; /* r */
			result[1] = tex_color[1]; /* g */
			result[2] = tex_color[2]; /* b */
			result[3] = tex_color[3]; /* a */
			break;
		default:
			abort(); /* implementation error */
		}
		break;
	case GL_MODULATE:
		switch (tex_format) {
		case GL_ALPHA:
			result[0] = frag_color[0];
			result[1] = frag_color[1];
			result[2] = frag_color[2];
			result[3] = frag_color[3] * tex_color[3];
			break;
		case GL_LUMINANCE:
			result[0] = frag_color[0] * tex_color[0];
			result[1] = frag_color[1] * tex_color[0];
			result[2] = frag_color[2] * tex_color[0];
			result[3] = frag_color[3];
			break;
		case GL_LUMINANCE_ALPHA:
			result[0] = frag_color[0] * tex_color[0];
			result[1] = frag_color[1] * tex_color[0];
			result[2] = frag_color[2] * tex_color[0];
			result[3] = frag_color[3] * tex_color[3];
			break;
		case GL_INTENSITY:
			result[0] = frag_color[0] * tex_color[0];
			result[1] = frag_color[1] * tex_color[0];
			result[2] = frag_color[2] * tex_color[0];
			result[3] = frag_color[3] * tex_color[0];
			break;
		case GL_RGB:
			result[0] = frag_color[0] * tex_color[0];
			result[1] = frag_color[1] * tex_color[1];
			result[2] = frag_color[2] * tex_color[2];
			result[3] = frag_color[3];
			break;
		case GL_RGBA:
			result[0] = frag_color[0] * tex_color[0];
			result[1] = frag_color[1] * tex_color[1];
			result[2] = frag_color[2] * tex_color[2];
			result[3] = frag_color[3] * tex_color[3];
			break;
		default:
			abort(); /* implementation error */
		}
		break;
	case GL_DECAL:
		switch (tex_format) {
		case GL_ALPHA:
			result[0] = 0; /* undefined */
			result[1] = 0;
			result[2] = 0;
			result[3] = 0;
			break;
		case GL_LUMINANCE:
			result[0] = 0; /* undefined */
			result[1] = 0;
			result[2] = 0;
			result[3] = 0;
			break;
		case GL_LUMINANCE_ALPHA:
			result[0] = 0; /* undefined */
			result[1] = 0;
			result[2] = 0;
			result[3] = 0;
			break;
		case GL_INTENSITY:
			result[0] = 0; /* undefined */
			result[1] = 0;
			result[2] = 0;
			result[3] = 0;
			break;
		case GL_RGB:
			result[0] = tex_color[0];
			result[1] = tex_color[1];
			result[2] = tex_color[2];
			result[3] = frag_color[3];
			break;
		case GL_RGBA: {
			const float a = tex_color[3];
			const float oma = 1.0 - a;
			result[0] = frag_color[0] * oma + tex_color[0] * a;
			result[1] = frag_color[1] * oma + tex_color[1] * a;
			result[2] = frag_color[2] * oma + tex_color[2] * a;
			result[3] = frag_color[3];
		} break;
		default:
			abort(); /* implementation error */
		}
		break;
	case GL_BLEND:
		switch (tex_format) {
		case GL_ALPHA:
			result[0] = frag_color[0];
			result[1] = frag_color[1];
			result[2] = frag_color[2];
			result[3] = frag_color[3] * tex_color[3];
			break;
		case GL_LUMINANCE: {
			const float l = tex_color[0];
			const float oml = 1.0 - l;
			result[0] = frag_color[0] * oml + env_color[0] * l;
			result[1] = frag_color[1] * oml + env_color[1] * l;
			result[2] = frag_color[2] * oml + env_color[2] * l;
			result[3] = frag_color[3];
		} break;
		case GL_LUMINANCE_ALPHA: {
			const float l = tex_color[0];
			const float oml = 1.0 - l;
			result[0] = frag_color[0] * oml + env_color[0] * l;
			result[1] = frag_color[1] * oml + env_color[1] * l;
			result[2] = frag_color[2] * oml + env_color[2] * l;
			result[3] = frag_color[3] * tex_color[3];
		} break;
		case GL_INTENSITY: {
			const float i = tex_color[0];
			const float omi = 1.0 - i;
			result[0] = frag_color[0] * omi + env_color[0] * i;
			result[1] = frag_color[1] * omi + env_color[1] * i;
			result[2] = frag_color[2] * omi + env_color[2] * i;
			result[3] = frag_color[3] * omi + env_color[3] * i;
		} break;
		case GL_RGB: {
			const float r = tex_color[0];
			const float omr = 1.0 - r;
			const float g = tex_color[1];
			const float omg = 1.0 - g;
			const float b = tex_color[2];
			const float omb = 1.0 - b;
			result[0] = frag_color[0] * omr + env_color[0] * r;
			result[1] = frag_color[1] * omg + env_color[1] * g;
			result[2] = frag_color[2] * omb + env_color[2] * b;
			result[3] = frag_color[3];
		} break;
		case GL_RGBA: {
			const float r = tex_color[0];
			const float omr = 1.0 - r;
			const float g = tex_color[1];
			const float omg = 1.0 - g;
			const float b = tex_color[2];
			const float omb = 1.0 - b;
			result[0] = frag_color[0] * omr + env_color[0] * r;
			result[1] = frag_color[1] * omg + env_color[1] * g;
			result[2] = frag_color[2] * omb + env_color[2] * b;
			result[3] = frag_color[3] * tex_color[3];
		} break;
		default:
			abort(); /* implementation error */
		}
		break;
	case GL_ADD:
		switch (tex_format) {
		case GL_ALPHA:
			result[0] = frag_color[0];
			result[1] = frag_color[1];
			result[2] = frag_color[2];
			result[3] = frag_color[3] * tex_color[3];
			break;
		case GL_LUMINANCE:
			result[0] = frag_color[0] + tex_color[0];
			result[1] = frag_color[1] + tex_color[0];
			result[2] = frag_color[2] + tex_color[0];
			result[3] = frag_color[3];
			break;
		case GL_LUMINANCE_ALPHA:
			result[0] = frag_color[0] + tex_color[0];
			result[1] = frag_color[1] + tex_color[0];
			result[2] = frag_color[2] + tex_color[0];
			result[3] = frag_color[3] * tex_color[3];
			break;
		case GL_INTENSITY:
			result[0] = frag_color[0] + tex_color[0];
			result[1] = frag_color[1] + tex_color[0];
			result[2] = frag_color[2] + tex_color[0];
			result[3] = frag_color[3] + tex_color[0];
			break;
		case GL_RGB:
			result[0] = frag_color[0] + tex_color[0];
			result[1] = frag_color[1] + tex_color[1];
			result[2] = frag_color[2] + tex_color[2];
			result[3] = frag_color[3];
			break;
		case GL_RGBA:
			result[0] = frag_color[0] + tex_color[0];
			result[1] = frag_color[1] + tex_color[1];
			result[2] = frag_color[2] + tex_color[2];
			result[3] = frag_color[3] * tex_color[3];
			break;
		default:
			abort(); /* implementation error */
		}
		/* clamping */
		if (result[0] > 1.0)
			result[0] = 1.0;
		if (result[1] > 1.0)
			result[1] = 1.0;
		if (result[2] > 1.0)
			result[2] = 1.0;
		if (result[3] > 1.0)
			result[3] = 1.0;
		break;
	default:
		abort(); /* implementation error */
	}

#if BLEND_WITH_BACKGROUND
	/* now blend result over a gray background */
	const float alpha = result[3];
	const float om_alpha = 1.0 - alpha;
	result[0] = result[0] * alpha + bg_color[0] * om_alpha;
	result[1] = result[1] * alpha + bg_color[1] * om_alpha;
	result[2] = result[2] * alpha + bg_color[2] * om_alpha;
	result[3] = result[3] * alpha + bg_color[3] * om_alpha;
#endif
}

/* Make a texture in which the colors vary along the length
 * according to the colors[] array.  For example, we use
 * 243 columns of the texture to store 81 colors, 3 texels each. */
static void
make_tex_image(GLenum base_format, int num_colors, const float colors[][4])
{
#define WIDTH 256
#define HEIGHT 4
	static float img[WIDTH * HEIGHT][4];

	assert(num_colors == 81);

	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			int c = j / 3;
			if (c >= num_colors) {
				img[i * WIDTH + j][0] = 0.0;
				img[i * WIDTH + j][1] = 0.0;
				img[i * WIDTH + j][2] = 0.0;
				img[i * WIDTH + j][3] = 0.0;
			} else {
				img[i * WIDTH + j][0] = colors[c][0];
				img[i * WIDTH + j][1] = colors[c][1];
				img[i * WIDTH + j][2] = colors[c][2];
				img[i * WIDTH + j][3] = colors[c][3];
			}
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, base_format, WIDTH, HEIGHT, 0, GL_RGBA,
		     GL_FLOAT, (void *)img);
}

/* Do num_colors * num_colors tests in one batch.
 * Setup a texture in which the colors vary by column.
 * Draw a quadstrip in which we draw horizontal bands of colors.
 * Drawing the textured quadstrips will fill the window with
 * num_colors * num_colors test squares.
 * Verify that they're all correct.
 * Return:  true = pass, false = fail */
static bool
matrix_test(GLenum env_mode, GLenum tex_format, int num_colors,
	    const float colors[][4], const float env_color[4])
{
	const char *format_name = piglit_get_gl_enum_name(tex_format);
	const char *env_name = piglit_get_gl_enum_name(env_mode);

	if (env_mode == GL_DECAL &&
	    (tex_format != GL_RGB && tex_format != GL_RGBA)) {
		/* undefined mode */
		return true;
	}

	glClear(GL_COLOR_BUFFER_BIT);

	/* The texture colors are the columns */
	make_tex_image(tex_format, num_colors, colors);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, env_mode);
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, env_color);

	/* The fragment colors are the rows */
	float W = num_colors * 3;
	float S = (float)(num_colors * 3) / (float)256;
	glBegin(GL_QUAD_STRIP);
	glTexCoord2f(0, 0);  glVertex2f(0, 0);
	glTexCoord2f(S, 0);  glVertex2f(W, 0);
	for (int i = 0; i < num_colors; i++) {
		glColor4fv(colors[i]);
		float y = i * 3 + 3;
		float t = y / (num_colors * 3);
		glTexCoord2f(0, t);  glVertex2f(0, y);
		glTexCoord2f(S, t);  glVertex2f(W, y);
	}
	glEnd();

	/* Check results */
	for (int row = 0; row < num_colors; row++) {
		for (int col = 0; col < num_colors; col++) {

			/* compute expected */
			float expected[4];
			compute_expected_color(env_mode, tex_format,
					       colors[col], colors[row],
					       env_color, expected);

			/* fetch actual pixel */
			int x = col * 3 + 1;
			int y = row * 3 + 1;

			/* compare */
			if (!piglit_probe_pixel_rgba(x, y, expected)) {
				/* Report the error */
				printf("GL_TEXTURE_ENV_MODE = %s\n"
				       "Texture Format = %s\n"
				       "Fragment Color = (%f, %f, %f, %f)\n"
				       "Texture Color = (%f, %f, %f, %f)\n"
				       "Tex Env Color = (%f, %f, %f, %f)\n",
				       env_name, format_name,
				       colors[row][0], colors[row][1],
				       colors[row][2], colors[row][3],
				       colors[col][0], colors[col][1],
				       colors[col][2], colors[col][3],
				       env_color[0], env_color[1],
				       env_color[2], env_color[3]);
#if BLEND_WITH_BACKGROUND
				printf("Blend over = (%f, %f, %f, %f)\n",
				       bg_color[0], bg_color[1],
				       bg_color[2], bg_color[3]);
#endif
				return false;
			}
		}
	}
	return true;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	for (int fmt = 0; fmt < 6; fmt++) {
		const GLenum format = format_enums[fmt];
		for (int mode = 0; mode < 5; mode++) {
			const GLenum env_mode = env_mode_enums[mode];
			if (env_mode == GL_BLEND && format != GL_ALPHA) {
				/* also vary texenv color, every 5th is OK. */
				for (int i = 0; i < COLORS; i += 5) {
					const float *env_color = colors[i];
					if (!matrix_test(
						    env_mode, format, COLORS,
						    (const float(*)[4])colors,
						    env_color)) {
						pass = false;
						break;
					}
				}
			} else {
				/* texenv color not significant */
				if (!matrix_test(env_mode, format, COLORS,
						 (const float(*)[4])colors,
						 colors[0])) {
					pass = false;
				}
			}
		}
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint texture;

	/* colors[] is an array of all possible RGBA colors with component
	 * values of 0, 0.5, and 1.0 */
	for (int i = 0; i < COLORS; i++) {
		int r = i % 3;
		int g = (i / 3) % 3;
		int b = (i / 9) % 3;
		int a = (i / 27) % 3;
		colors[i][0] = (float)r / 2.0;
		colors[i][1] = (float)g / 2.0;
		colors[i][2] = (float)b / 2.0;
		colors[i][3] = (float)a / 2.0;
	}

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glEnable(GL_TEXTURE_2D);

#if BLEND_WITH_BACKGROUND
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
#endif

	glClearColor(bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
	glShadeModel(GL_FLAT);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
