/*
 * Copyright (C) 2012 VMware, Inc.
 * Copyright (C) 2010 Intel Corporation
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
 *    Jose Fonseca
 *    Eric Anholt <eric@anholt.net>
 *    Brian Paul
 */

/** @file fbo-blit-stretch.c
 *
 * Tests EXT_framebuffer_blit with various combinations of window system and
 * FBO objects.  Because FBOs are generally stored inverted relative to
 * window system frambuffers, this could catch flipping failures in blit paths.
 *
 * See also fbo-blit.c
 */

#include <algorithm>

#include "piglit-util-gl.h"


/*
 * XXX: Checkerboard is not a good test pattern, because OpenGL spec allows the
 * implementation to clamp against source rectangle edge, as oposed to clamp
 * against the source edges, causing different results along the edge.
 */
#define CHECKERBOARD 0

#define DSTW 200
#define DSTH 150

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = DSTW;
	config.window_height = DSTH;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

struct TestCase
{
	GLint srcW, srcH;
	GLint srcX0; GLint srcY0; GLint srcX1; GLint srcY1;
	GLint dstX0; GLint dstY0; GLint dstX1; GLint dstY1;
	GLenum filter;
};

static void
describe(const TestCase &test)
{
	GLint dstW = piglit_width;
	GLint dstH = piglit_height;

	printf("%ix%i (%i, %i)-(%i, %i) => %ix%i (%i, %i)-(%i, %i)",
	       test.srcW, test.srcH,
	       test.srcX0, test.srcY0, test.srcX1, test.srcY1,
	       dstW, dstH,
	       test.dstX0, test.dstY0, test.dstX1, test.dstY1);

	GLint srcDX = test.srcX1 - test.srcX0;
	GLint srcDY = test.srcY1 - test.srcY0;
	GLint dstDX = test.dstX1 - test.dstX0;
	GLint dstDY = test.dstY1 - test.dstY0;

	if (srcDX < 0) {
		printf(" flip_src_x");
		srcDX = -srcDX;
	}
	if (srcDY < 0) {
		printf(" flip_src_y");
		srcDY = -srcDY;
	}
	if (dstDX < 0) {
		printf(" flip_dst_x");
		dstDX = -dstDX;
	}
	if (dstDY < 0) {
		printf(" flip_dst_y");
		dstDY = -dstDY;
	}

	if (dstDX > srcDX)
		printf(" stretch_x");
	if (dstDX < srcDX)
		printf(" shrink_x");

	if (dstDY > srcDY)
		printf(" stretch_y");
	if (dstDY < srcDY)
		printf(" shrink_y");

	if (test.srcX0 < 0 || test.srcX0 > test.srcW ||
	    test.srcX1 < 0 || test.srcX1 > test.srcW)
		printf(" clamp_src_x");
	if (test.srcY0 < 0 || test.srcY0 > test.srcH ||
	    test.srcY1 < 0 || test.srcY1 > test.srcH)
		printf(" clamp_src_y");

	if (test.dstX0 < 0 || test.dstX0 > dstW ||
	    test.dstX1 < 0 || test.dstX1 > dstW)
		printf(" clamp_dst_x");
	if (test.dstY0 < 0 || test.dstY0 > dstH ||
	    test.dstY1 < 0 || test.dstY1 > dstH)
		printf(" clamp_dst_y");

	switch (test.filter) {
	case GL_NEAREST:
		printf(" nearest");
		break;
	case GL_LINEAR:
		printf(" linear");
		break;
	default:
		assert(0);
	}

	printf("\n");
}

static void
filter(const TestCase &test, float coord, GLint &coord0, GLint &coord1, float &weight)
{
	switch (test.filter) {
	case GL_NEAREST:
		coord0 = roundf(coord);
		// ambigious
		assert(fabsf(coord0 - coord) != 0.5f);
		weight = 0.0f;
		break;
	case GL_LINEAR:
		coord0 = floorf(coord);
		weight = coord - (float)coord0;
		break;
	default:
		assert(0);
		coord0 = 0;
		weight = 0.0f;
	}

	assert(weight >= 0.0f);
	assert(weight  < 1.0f);

	coord1 = coord0 + 1;
}

static void
clamp(GLint &x, GLint xmin, GLint xmax)
{
	if (x < xmin) {
		x = xmin;
	}
	if (x > xmax) {
		x = xmax;
	}
}

static float
lerp(float x0, float x1, float w)
{
	return x0 + (x1 - x0) * w;
}

static float
lerp2d(float xy00, float xy01, float xy10, float xy11, float wx, float wy)
{
	float y0 = lerp(xy00, xy01, wx);
	float y1 = lerp(xy10, xy11, wx);
	return lerp(y0, y1, wy);
}

static float clearColor[4] = {
#if CHECKERBOARD
	0.0, 0.0, 1.0, 1.0
#else
	0.5, 0.5, 0.5, 0.5
#endif
};

static GLboolean
verify(const TestCase &test, GLuint srcFBO, GLuint dstFBO, GLuint numChannels)
{
	GLint srcX0 = test.srcX0;
	GLint srcY0 = test.srcY0;
	GLint srcX1 = test.srcX1;
	GLint srcY1 = test.srcY1;
	GLint dstX0 = test.dstX0;
	GLint dstY0 = test.dstY0;
	GLint dstX1 = test.dstX1;
	GLint dstY1 = test.dstY1;

	if (dstX1 < dstX0) {
		std::swap(srcX0, srcX1);
		std::swap(dstX0, dstX1);
	}
	if (dstY1 < dstY0) {
		std::swap(srcY0, srcY1);
		std::swap(dstY0, dstY1);
	}

	GLint srcDX = srcX1 - srcX0;
	GLint srcDY = srcY1 - srcY0;
	GLint dstDX = dstX1 - dstX0;
	GLint dstDY = dstY1 - dstY0;

	GLint dstW = piglit_width;
	GLint dstH = piglit_height;

	float *srcPixels = new float[test.srcH * test.srcW * numChannels];

	glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFBO);
	glReadPixels(0, 0, test.srcW, test.srcH, GL_RGB, GL_FLOAT, srcPixels);

	float *expectedDstPixels = new float[dstH * dstW * numChannels];

	for (GLint dstY = 0; dstY < dstH; ++dstY) {
		for (GLint dstX = 0; dstX < dstW; ++dstX) {
			float *dstPixel = expectedDstPixels + (dstY * dstW + dstX) * numChannels;
			for (GLuint c = 0; c < numChannels; ++c) {
				dstPixel[c] = clearColor[c];
			}
		}
	}

	GLint dstX0clamped = std::max(dstX0, 0);
	GLint dstY0clamped = std::max(dstY0, 0);
	GLint dstX1clamped = std::min(dstX1, dstW);
	GLint dstY1clamped = std::min(dstY1, dstH);

	for (GLint dstY = dstY0clamped; dstY < dstY1clamped; ++dstY) {
		float srcY = srcY0 + (dstY - dstY0 + 0.5) * srcDY / dstDY;
		if (srcY < 0 || srcY >= test.srcH) {
			continue;
		}

		srcY -= 0.5f;

		GLint srcPixelY0, srcPixelY1;
		float weightY;
		filter(test, srcY, srcPixelY0, srcPixelY1, weightY);
		clamp(srcPixelY0, 0, test.srcH - 1);
		clamp(srcPixelY1, 0, test.srcH - 1);

		for (GLint dstX = dstX0clamped; dstX < dstX1clamped; ++dstX) {
			float srcX = srcX0 + (dstX - dstX0 + 0.5) * srcDX / dstDX;
			if (srcX < 0 || srcX >= test.srcW) {
				continue;
			}

			srcX -= 0.5f;

			GLint srcPixelX0, srcPixelX1;
			float weightX;
			filter(test, srcX, srcPixelX0, srcPixelX1, weightX);
			clamp(srcPixelX0, 0, test.srcW - 1);
			clamp(srcPixelX1, 0, test.srcW - 1);

			float *srcPixel00 = srcPixels + (srcPixelY0 * test.srcW + srcPixelX0) * numChannels;
			float *srcPixel01 = srcPixels + (srcPixelY0 * test.srcW + srcPixelX1) * numChannels;
			float *srcPixel10 = srcPixels + (srcPixelY1 * test.srcW + srcPixelX0) * numChannels;
			float *srcPixel11 = srcPixels + (srcPixelY1 * test.srcW + srcPixelX1) * numChannels;

			float *dstPixel = expectedDstPixels
				        + (dstY * dstW + dstX) * numChannels;

			for (GLuint c = 0; c < numChannels; ++c) {
				dstPixel[c] = lerp2d(srcPixel00[c],
						     srcPixel01[c],
						     srcPixel10[c],
						     srcPixel11[c],
						     weightX, weightY);
			}
		}
	}

	delete [] srcPixels;

	float *observedDstPixels = new float[dstH * dstW * numChannels];
	glBindFramebuffer(GL_READ_FRAMEBUFFER, dstFBO);
	glReadPixels(0, 0, dstW, dstH, GL_RGB, GL_FLOAT, observedDstPixels);

	GLboolean pass;
	pass = piglit_compare_images_color(0, 0, dstW, dstH, numChannels,
			                   piglit_tolerance,
					   expectedDstPixels,
					   observedDstPixels);

	delete [] observedDstPixels;
	delete [] expectedDstPixels;

	return pass;
}

static void
blit(const TestCase &test)
{
	glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlitFramebuffer(test.srcX0, test.srcY0, test.srcX1, test.srcY1,
			  test.dstX0, test.dstY0, test.dstX1, test.dstY1,
			  GL_COLOR_BUFFER_BIT, test.filter);
}


static GLboolean
run_test(const TestCase &test)
{
	describe(test);

	GLboolean pass;

	GLuint tex;
	GLuint fbo;
	GLenum status;

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

#if CHECKERBOARD
	const float color1[4] = {1.0f, 0.0f, 0.0f, 1.0f};
	const float color2[4] = {0.0f, 1.0f, 0.0f, 1.0f};
	tex = piglit_checkerboard_texture(0, 0, test.srcW, test.srcH, 1, 1,  color1, color2);
#else
	tex = piglit_rgbw_texture(GL_RGBA, test.srcW, test.srcH, GL_FALSE, GL_TRUE, GL_UNSIGNED_NORMALIZED);
#endif
	glBindTexture(GL_TEXTURE_2D, tex);

	glFramebufferTexture2D(GL_FRAMEBUFFER,
			       GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D,
			       tex,
			       0);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		pass = GL_TRUE;
	} else {
		glViewport(0, 0, piglit_width, piglit_height);
		piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);

		blit(test);

		pass = verify(test, fbo, 0, 3);

		if (!piglit_automatic) {
			piglit_present_results();
			if (!pass) {
				//getchar();
			}
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &tex);

	return pass;
}

/*
 * Constants to help define several test cases.
 */

#define SRCW 45
#define SRCH 79
#define DX 17
#define DY 11
#define SRCXMIN 13
#define SRCYMIN 33
#define SRCXMAX (SRCXMIN + DX)
#define SRCYMAX (SRCYMIN + DY)

#define DSTXMIN 19
#define DSTYMIN 23
#define DSTXMAX (DSTXMIN + DX)
#define DSTYMAX (DSTYMIN + DY)

const TestCase
tests[] = {
	/*
	 * Basic 1:1 copy
	 */

	{
		SRCW, SRCH,
		SRCXMIN, SRCYMIN, SRCXMAX, SRCYMAX,
		DSTXMIN, DSTYMIN, DSTXMAX, DSTYMAX,
		GL_NEAREST,
	},

	/*
	 * Flip tests
	 */

	{
		SRCW, SRCH,
		SRCXMAX, SRCYMAX, SRCXMIN, SRCYMIN, // flip xy
		DSTXMAX, DSTYMAX, DSTXMIN, DSTYMIN, // flip xy
		GL_NEAREST,
	},
	{
		SRCW, SRCH,
		SRCXMAX, SRCYMIN, SRCXMIN, SRCYMAX, // fliped x
		DSTXMIN, DSTYMAX, DSTXMAX, DSTYMIN, // fliped y
		GL_NEAREST,
	},
	{
		SRCW, SRCH,
		SRCXMIN, SRCYMAX, SRCXMAX, SRCYMIN, // fliped y
		DSTXMAX, DSTYMIN, DSTXMIN, DSTYMAX, // fliped x
		GL_NEAREST,
	},

	/*
	 * Stretch.
	 */

	{
		SRCW, SRCH,
		SRCXMIN, SRCYMIN, SRCXMAX, SRCYMAX,
		DSTXMIN, DSTYMIN, DSTXMAX + 3*DX, DSTYMAX + 3*DY, // stretch x y
		GL_NEAREST,
	},
	{
		SRCW, SRCH,
		SRCXMIN, SRCYMIN, SRCXMAX, SRCYMAX,
		DSTXMIN, DSTYMIN, DSTXMAX + 3*DX, DSTYMAX + 3*DY, // stretch x y
		GL_NEAREST,
	},

	/*
	 * Stretch of a single pixel.
	 */

	{
		SRCW, SRCH,
		SRCXMIN, SRCYMIN, SRCXMIN + 1, SRCYMIN + 1,
		DSTXMIN, DSTYMIN, DSTXMIN + 7, DSTYMIN + 7, // stretch x y
		GL_NEAREST,
	},


	/*
	 * Clip
	 */

	{
		SRCW, SRCH,
		SRCXMIN, SRCYMIN, SRCXMAX, SRCYMAX,
		-DX/2, -DY/2, -DX/2 + DX, -DY/2 + DY, // clip dst left bottom
		GL_NEAREST,
	},
	{
		SRCW, SRCH,
		SRCXMIN, SRCYMIN, SRCXMAX, SRCYMAX,
		DSTW - DX/2, DSTH - DY/2, DSTW - DX/2 + DX, DSTH - DY/2 + DY, // clip dst top right
		GL_NEAREST,
	},
	{
		SRCW, SRCH,
		-DX/2, -DY/2, -DX/2 + DX, -DY/2 + DY, // clip src left bottom
		DSTXMIN, DSTYMIN, DSTXMAX, DSTYMAX,
		GL_NEAREST,
	},
	{
		SRCW, SRCH,
		SRCW - DX/2, SRCH - DY/2, SRCW - DX/2 + DX, SRCH - DY/2 + DY, // clip src top right
		DSTXMIN, DSTYMIN, DSTXMAX, DSTYMAX,
		GL_NEAREST,
	},

	/*
	 * Clip & stretch.
	 *
	 * XXX: These tests are disabled for now, because Mesa clips in integer
	 * coordinates, instead of floats, which ends up affecting how the
	 * whole surface is interpolated, which goes against the spec.
	 */
#if 0
	{
		SRCW, SRCH,
		SRCXMIN, SRCYMIN, SRCXMAX, SRCYMAX,
		-DSTXMIN, -DSTYMIN, DSTXMAX, DSTYMAX, // clip dst left bottom
		GL_NEAREST,
	},
	{
		SRCW, SRCH,
		SRCXMIN, SRCYMIN, SRCXMAX, SRCYMAX,
		DSTXMIN, DSTYMIN, DSTW + DSTXMIN, DSTH + DSTYMIN, // clip dst top right
		GL_NEAREST,
	},
	{
		SRCW, SRCH,
		-SRCXMIN, -SRCYMIN, SRCXMAX, SRCYMAX, // clip src left bottom
		DSTXMIN, DSTYMIN, DSTXMAX, DSTYMAX,
		GL_NEAREST,
	},
	{
		SRCW, SRCH,
		SRCXMIN, SRCYMIN, SRCW + SRCXMIN, SRCH + SRCYMIN, // clip src top right
		DSTXMIN, DSTYMIN, DSTXMAX, DSTYMAX,
		GL_NEAREST,
	},
	{
		SRCW, SRCH,
		SRCXMAX, SRCYMIN, SRCXMIN, SRCYMAX, // fliped x
		-DSTXMIN, DSTH + DSTYMIN, DSTW + DSTXMIN, -DSTYMIN, // fliped y, cliped x y
		GL_NEAREST,
	},
#endif

	/*
	 * Full stretch
	 */

	{
		SRCW, SRCH,
		0, 0, SRCW, SRCH,
		0, 0, DSTW, DSTH,
		GL_NEAREST,
	},
};

static int test_index = -1;

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	for (unsigned i = 0; i < ARRAY_SIZE(tests); i++) {
		if (test_index != -1 &&
		    test_index != (int) i)
			continue;

		TestCase test = tests[i];

		test.filter = GL_NEAREST;
		pass = run_test(test) && pass;

		test.filter = GL_LINEAR;
		pass = run_test(test) && pass;
	}
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	piglit_require_extension("GL_ARB_framebuffer_object");

	if (argc == 2)
		sscanf(argv[1], "%d", &test_index);
}
