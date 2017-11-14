/*
 * Copyright (C) 2007  Intel Corporation
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
 * @file mipmap.c:  Test the GL_ARB_point_sprite extension
 *
 * Create mipmap textures which size varies from 32x32 to 1x1, every texture
 * has different two colors: the upper half is one color and the lower half is
 * another color.
 * Draw point and polygon which mode is GL_POINT, and check that the point is
 * rendered correctly.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 14;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

/* For the 1x1 LOD, only lower part (second color in the table) is used. */
static const float tex_color[6][2][4] = {
	{{1.0, 0.0, 0.0, 1.0}, {0.0, 1.0, 0.0, 1.0}}, /* 32x32 */
	{{0.0, 0.0, 1.0, 1.0}, {1.0, 1.0, 0.0, 1.0}}, /* 16x16 */
	{{1.0, 0.0, 1.0, 1.0}, {0.0, 1.0, 1.0, 1.0}}, /* 8x8 */
	{{1.0, 1.0, 1.0, 1.0}, {1.0, 0.0, 0.0, 1.0}}, /* 4x4 */
	{{0.0, 1.0, 0.0, 1.0}, {0.0, 0.0, 1.0, 1.0}}, /* 2x2 */
	{{1.0, 1.0, 0.0, 1.0}, {1.0, 1.0, 1.0, 1.0}}, /* 1x1 */
};

static int
level(int point_size)
{
	/* Note: we use GL_NEAREST_MIPMAP_NEAREST for  GL_TEXTURE_MIN_FILTER
	 */
	if (point_size <= 1)
		return 5;
	if (point_size < 3)
		return 4;
	if (point_size < 6)
		return 3;
	if (point_size < 12)
		return 2;
	if (point_size < 24)
		return 1;
	return 0;
}

bool
draw_and_probe(int point_size, int coord_origin, int prim_type)
{
	glPointSize((float)point_size);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw GL_POINTS primitives, and draw GL_POLYGON primitives with the
	 * polygon mode set to GL_POINT.
	 */
	if (prim_type == 0) {
		glBegin(GL_POINTS);
		glVertex2i(piglit_width / 4, piglit_height / 4);
		glEnd();
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		glBegin(GL_POLYGON);
		glVertex2i(piglit_width / 4, piglit_height / 4);
		glVertex2i(piglit_width, piglit_height / 4);
		glVertex2i(piglit_width, piglit_height);
		glVertex2i(piglit_width / 4, piglit_height);
		glEnd();
	}

	/* bottom, left, right, top, width and height of sprite */
	const int b = piglit_width / 4 - point_size / 2;
	const int l = piglit_height / 4 - point_size / 2;
	const int r = piglit_height / 4 + point_size / 2;
	const int t = piglit_width / 4 + point_size / 2;
	const int w = point_size;
	const int h = point_size / 2;
	/* vertical middle of sprite */
	const int m = piglit_height / 4;

	/* width of total area to probe */
	const int W = piglit_width / 2;

	const float bk[] = {0.0, 0.0, 0.0, 0.0};

	const float *cb = tex_color[level(point_size)][coord_origin];
	const float *ct = tex_color[level(point_size)][1 - coord_origin];
	if (piglit_probe_rect_rgba(0, 0, W, b, bk) &&
	    piglit_probe_rect_rgba(0, b, l, w, bk) &&
	    piglit_probe_rect_rgba(l, b, w, h, cb) &&
	    piglit_probe_rect_rgba(l, m, w, h, ct) &&
	    piglit_probe_rect_rgba(r, b, l, w, bk) &&
	    piglit_probe_rect_rgba(0, t, W, b, bk))
		return true;

	printf("Primitive type: %s, Coord Origin at: %s\n",
	       prim_type ? "polygon" : "points",
	       coord_origin ? "bottom" : "top");
	return false;
}

enum piglit_result
piglit_display(void)
{
	float max_point_size;
	int point_size;
	float point_size_range[2], point_size_granularity;
	float n;
	const float epsilon = 1e-5;

	glGetFloatv(GL_SMOOTH_POINT_SIZE_RANGE, point_size_range);
	glGetFloatv(GL_SMOOTH_POINT_SIZE_GRANULARITY,
		    &point_size_granularity);

	/* check that point size 2.0 is supported */
	n = (2.0 - point_size_range[0]) / point_size_granularity;
	if (!(roundf(n) - n < epsilon))
		piglit_report_result(PIGLIT_SKIP);
	/* check that other even integer point sizes are supported */
	n = 2.0 / point_size_granularity;
	if (!(roundf(n) - n < epsilon))
		piglit_report_result(PIGLIT_SKIP);

	max_point_size = MIN3(point_size_range[1], piglit_width / 2,
			      piglit_height / 2);

	for (int prim_type = 0; prim_type < 2; prim_type++) {
		if (piglit_get_gl_version() >= 20)
			glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN,
					  GL_UPPER_LEFT);
		for (int coord_origin = 0; coord_origin < 2; coord_origin++) {
			for (point_size = 2;
			     (float)point_size <= max_point_size;
			     point_size += 2) {
				if (!draw_and_probe(point_size, coord_origin,
						    prim_type))
					return PIGLIT_FAIL;
			}

			/* OpenGL 2.0 adds the ability to set the texture
			 * coordinate origin to the bottom left */
			if (piglit_get_gl_version() < 20)
				break;
			glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN,
					  GL_LOWER_LEFT);
		}
	}

	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	GLboolean enable;
	GLint coordReplace;
	GLint coordOrigin;
	GLuint tex;

	piglit_require_extension("GL_ARB_point_sprite");

	/* check point sprite status, default is GL_FALSE */
	enable = glIsEnabled(GL_POINT_SPRITE);
	if (enable != GL_FALSE) {
		printf("PointSprite should be disabled by default\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* check coordinate replacement, default is GL_FALSE */
	glGetTexEnviv(GL_POINT_SPRITE, GL_COORD_REPLACE, &coordReplace);

	if (coordReplace != GL_FALSE) {
		printf("default value of COORD_REPLACE should be GL_FALSE\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* check coordinate origin, default is UPPER_LEFT */
	glEnable(GL_POINT_SPRITE);
	if (piglit_get_gl_version() >= 20) {
		glGetIntegerv(GL_POINT_SPRITE_COORD_ORIGIN, &coordOrigin);
		if (coordOrigin != GL_UPPER_LEFT) {
			printf("default value of COORD_ORIGIN should be "
			       "GL_UPPER_LEFT\n");
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	glMatrixMode(GL_PROJECTION);
	glOrtho(0, piglit_width, 0, piglit_height, -1, 1);

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int lvl = 0;
	for (int size = 32; size; size /= 2) {
		int mid = MAX2(size / 2, 1);
		piglit_quads_texture(tex, lvl, size, size, size, mid,
				     tex_color[lvl][1], NULL,
				     tex_color[lvl][0], NULL);
		lvl++;
	}

	glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
}
