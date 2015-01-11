/*
 * Copyright © 2010 Luca Barbieri
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
 */

/** @file spec/arb_color_buffer_float/common.h
 *
 * Common test framework for GL_ARB_color_buffer_float
 *
 * NOTE: both ATI and nVidia proprietary drivers are seriously broken, in
 * different ways!
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 128;
	config.window_height = 128;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/* use small values for pixels[0..3], so that the 0.01 tolerance is met for fp16 */
static float pixels[] = {
	7, -2.75, -0.25, 0.75,
	0.0, 1.0, 2.0, -1.0,
	0.5, 9.0 / 8.0, -156, 390,
	234, -86, -21.5, 46.5,
};

static float clamped_pixels[16];
static float signed_clamped_pixels[16];
static float pixels_mul_2[16];
static float clamped_pixels_mul_2[16];
static float clamped_pixels_mul_2_signed_clamped[16];
static float signed_clamped_pixels_mul_2_signed_clamped[16];
static float pixels_plus_half[16];
static float clamped_pixels_plus_half[16];
static float clamped_pixels_plus_half_clamped[16];
static float clamped_pixels_plus_half_signed_clamped[16];
static float signed_clamped_pixels_plus_half_signed_clamped[16];

static const char* clamp_strings[] = {"TRUE ", "FIXED", "FALSE"};
static GLenum clamp_enums[] = {GL_TRUE, GL_FIXED_ONLY_ARB, GL_FALSE};

const char* mrt_mode_strings[] = {"single target", "homogeneous framebuffer", "dishomogeneous framebuffer"};

static float clamp(float f)
{
	if (f >= 0.0f && f <= 1.0f)
		return f;
	else if (f > 0.0f)
		return 1.0f;
	else
		return 0.0f;
}

static float signed_clamp(float f)
{
	if (f >= -1.0f && f <= 1.0f)
		return f;
	else if (f > -1.0f)
		return 1.0f;
	else
		return -1.0f;
}

static unsigned ati_driver;
static unsigned nvidia_driver;

static GLuint tex, tex1, fb;
static GLenum status;
static unsigned error;

static int fbo_width = 2;
static int fbo_height = 2;

enum test_mode_type {
	TEST_NO_RT,
	TEST_SRT,
	TEST_MRT,
	TEST_SRT_MRT
};

static enum test_mode_type test_mode;

static GLboolean sanity, test_fog;
static GLenum format;
static const char* format_name;
static GLboolean fixed, fixed_snorm;
static GLboolean fixed0;
static GLboolean fixed1;
static unsigned mrt_mode;

static GLboolean test();

static GLboolean run_test()
{
	GLboolean pass = GL_TRUE;
	fixed_snorm = format == GL_RGBA8_SNORM;
	fixed = fixed0 = format == GL_RGBA8 || fixed_snorm;
	fixed1 = -1;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, format,
		     2, 2, 0,
		     GL_RGBA, GL_FLOAT, pixels);
	error = glGetError();
	if (error)
	{
		printf("GL error after glTexImage2D 0x%04X\n", error);
		return GL_FALSE;
	}

	if (test_mode)
	{
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenFramebuffersEXT(1, &fb);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
		glViewport(0, 0, fbo_width, fbo_height);

		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					  GL_COLOR_ATTACHMENT0_EXT,
					  GL_TEXTURE_2D,
					  tex,
					  0);
		error = glGetError();
		if (error)
		{
			printf("GL error after FBO 0x%04X\n", error);
			return GL_FALSE;
		}

		status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			fprintf(stderr, "fbo incomplete (status = 0x%04x)\n", status);
			piglit_report_result(PIGLIT_SKIP);
		}
	}

	if (test_mode <= TEST_SRT)
	{
		if (!sanity)
			glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);
		pass = test();
	}
	else
	{
		enum test_mode_type mrt_modes;
		enum test_mode_type first_mrt_mode = (test_mode == TEST_MRT) ? TEST_SRT : TEST_NO_RT;

		mrt_modes = TEST_SRT_MRT;
		if (!piglit_is_extension_supported("GL_ARB_texture_float"))
			mrt_modes = TEST_MRT;
		if (!piglit_is_extension_supported("GL_ARB_draw_buffers"))
			mrt_modes = TEST_SRT;
		else {
			GLint val;
			glGetIntegerv(GL_MAX_DRAW_BUFFERS, &val);
			if (val < 2)
				mrt_modes = TEST_SRT;
		}

		for (mrt_mode = first_mrt_mode; mrt_mode < mrt_modes; ++mrt_mode)
		{
			fixed1 = fixed;
			if (mrt_mode != TEST_NO_RT)
			{
				GLenum bufs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
				unsigned format1;
				if (mrt_mode == TEST_SRT)
					format1 = format;
				else
				{
					format1 = fixed0 ? GL_RGBA32F_ARB : GL_RGBA8;
					fixed1 = !fixed0;
					fixed = GL_FALSE;
				}

				glGenTextures(1, &tex1);
				glBindTexture(GL_TEXTURE_2D, tex1);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexImage2D(GL_TEXTURE_2D, 0, format1,
					     2, 2, 0,
					     GL_RGBA, GL_FLOAT, pixels);
				error = glGetError();
				if (error)
				{
					printf("GL error after second glTexImage2D 0x%04X\n", error);
					return GL_FALSE;
				}

				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
							  GL_COLOR_ATTACHMENT1_EXT,
							  GL_TEXTURE_2D,
							  tex1,
							  0);

				status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
				if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
					if (mrt_mode == TEST_MRT)
						printf("Dishomogeneous framebuffer is incomplete, skipping dishomogeneous tests (status = 0x%04x)\n", status);
					else
					{
						printf("Framebuffer is incomplete (status = 0x%04x)\n", status);
						pass = GL_FALSE;
					}
					goto skip_mrt;
				}

				glDrawBuffers(2, bufs);

				error = glGetError();
				if (error)
				{
					printf("GL error after second glDrawBuffers 0x%04X\n", error);
					return GL_FALSE;
				}
			}

			if (!sanity)
				glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);
			pass = test() && pass;

	skip_mrt:
			if (mrt_mode != TEST_NO_RT)
			{
				glDrawBuffer(GL_COLOR_ATTACHMENT0);

				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
						  GL_COLOR_ATTACHMENT1_EXT,
						  GL_TEXTURE_2D,
						  0,
						  0);
				glDeleteTextures(1, &tex1);
			}
		}
	}

	glDeleteTextures(1, &tex);
	tex = 0;
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	if (fb)
	{
		glDeleteFramebuffersEXT(1, &fb);
		fb = 0;
	}

	error = glGetError();
	if (error)
	{
		printf("GL error after test 0x%04X\n", error);
		return GL_FALSE;
	}

	return pass;
}

enum piglit_result
piglit_display(void)
{
	unsigned error;
	GLboolean pass = run_test();

	error = glGetError();
	if (error)
	{
		printf("GL error at end 0x%04X\n", error);
		return GL_FALSE;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static unsigned init();

void
piglit_init(int argc, char **argv)
{
	int i;
	GLboolean distinguish_xfails = GL_FALSE;

	/* displaying thousands of single-pixel floating point results isn't really useful, or even doable */
	piglit_automatic = GL_TRUE;

	test_mode = init();

	if (test_mode != TEST_NO_RT)
		piglit_require_extension("GL_EXT_framebuffer_object");

	for (i = 1; i < argc; ++i)
	{
		if (!strcmp(argv[i], "-xfail")) {
			distinguish_xfails = GL_TRUE;
		} else if (!strcmp(argv[i], "sanity")) {
			sanity = GL_TRUE;
		} else if (!strcmp(argv[i], "fog")) {
			test_fog = GL_TRUE;
		} else if (!strcmp(argv[i], "GL_RGBA16F")) {
			piglit_require_extension("GL_ARB_texture_float");
			format = GL_RGBA16F;
			format_name = "f16";
			printf("\n\n\nTesting 16-bit floating-point FBO\n");
		} else if (!strcmp(argv[i], "GL_RGBA32F")) {
			piglit_require_extension("GL_ARB_texture_float");
			format = GL_RGBA32F;
			format_name = "f32";
			printf("\n\n\nTesting 32-bit floating-point FBO\n");
		} else if (!strcmp(argv[i], "GL_RGBA8_SNORM")) {
			piglit_require_extension("GL_EXT_texture_snorm");
			format = GL_RGBA8_SNORM;
			format_name = "sn8";
			printf("\n\n\nTesting 8-bit signed normalized fixed-point FBO\n");
		}
	}
	if (!format) {
		format = GL_RGBA8;
		format_name = "un8";
		printf("Testing 8-bit unsigned normalized fixed-point FBO\n");
	}

	if (sanity) {
		printf("Testing default clamping rules only. This is a sanity check. GL_ARB_color_buffer_float is not required.\n");
	} else {
		piglit_require_extension("GL_ARB_color_buffer_float");
	}

	/* current ATI drivers are broken */
	if (!strcmp((char*)glGetString(GL_VENDOR), "ATI Technologies Inc."))
		ati_driver = 1;

	/* current nVidia drivers are broken at least on GeForce 7xxx */
	if (!strcmp((char*)glGetString(GL_VENDOR), "NVIDIA Corporation"))
		nvidia_driver = 1;

	if (ati_driver || nvidia_driver)
	{
		/* print both so users don't think either driver is better */
		printf("Notice: the ATI proprietary driver does NOT conform to the GL_ARB_color_buffer_float specification! (tested version was 10.6 on cypress, on Linux x86)\n");
		printf("Notice: the nVidia proprietary driver does NOT conform to the GL_ARB_color_buffer_float specification! (tested version was 256.44 on nv49, on Linux x86)\n");
		printf("Notice: the nVidia and ATI proprietary drivers are both nonconformant, in different ways!\n\n\n");
	}

	if (!distinguish_xfails)
		ati_driver = nvidia_driver = 0;

	for (i = 0; i < sizeof(pixels) / sizeof(pixels[0]); ++i)
	{
		clamped_pixels[i] = clamp(pixels[i]);
		signed_clamped_pixels[i] = signed_clamp(pixels[i]);

		pixels_mul_2[i] = pixels[i] * 2.0f;
		clamped_pixels_mul_2[i] = clamped_pixels[i] * 2.0f;
		clamped_pixels_mul_2_signed_clamped[i] = signed_clamp(clamped_pixels_mul_2[i]);
		signed_clamped_pixels_mul_2_signed_clamped[i] = signed_clamp(signed_clamped_pixels[i] * 2.0f);

		pixels_plus_half[i] = pixels[i] + 0.5f;
		clamped_pixels_plus_half[i] = clamped_pixels[i] + 0.5f;
		clamped_pixels_plus_half_clamped[i] = clamp(clamped_pixels_plus_half[i]);
		clamped_pixels_plus_half_signed_clamped[i] = signed_clamp(clamped_pixels_plus_half[i]);
		signed_clamped_pixels_plus_half_signed_clamped[i] = signed_clamp(signed_clamped_pixels[i] + 0.5);
	}
}

GLboolean compare_arrays(float* expected, float* observed, int comps, int length)
{
	GLboolean pass = GL_TRUE;
	unsigned i, j, k;
	for (i = 0; i < length; ++i)
	{
		for (j = 0; j < comps; j++) {
			if (fabs(expected[i*comps+j] - observed[i*comps+j]) > 0.01)
			{
				printf(" At %i:\n  Expected: %f", i, expected[i*comps]);
				for (k = 1; k < comps; k++)
					printf(", %f", expected[i*comps+k]);
				printf("\n  Observed: %f", observed[i*comps]);
				for (k = 1; k < comps; k++)
					printf(", %f", observed[i*comps+k]);
				printf("\n");
				pass = GL_FALSE;
				break;
			}
		}
	}

	return pass;
}
