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

#include "piglit-util.h"

#ifndef GL_COLOR_ATTACHMENT0
#define GL_COLOR_ATTACHMENT0 0x8CE0
#endif
#ifndef GL_COLOR_ATTACHMENT1
#define GL_COLOR_ATTACHMENT1 0x8CE1
#endif

int piglit_width = 128;
int piglit_height = 128;
int piglit_window_mode = GLUT_RGB | GLUT_ALPHA | GLUT_DOUBLE;

/* use small values for pixels[0..3], so that the 0.01 tolerance is met for fp16 */
float pixels[] = {
	7, -2.75, -0.25, 0.75,
	0.0, 1.0, 2.0, -1.0,
	0.5, 9.0 / 8.0, -156, 390,
	234, -86, -21.5, 46.5,
};

float clamped_pixels[16];
float pixels_mul_2[16];
float clamped_pixels_mul_2[16];
float pixels_plus_half[16];
float clamped_pixels_plus_half[16];
float clamped_pixels_plus_half_clamped[16];

const char* clamp_strings[] = {"TRUE", "FIXED_ONLY", "FALSE"};
GLenum clamp_enums[] = {GL_TRUE, GL_FIXED_ONLY_ARB, GL_FALSE};

const char* mrt_mode_strings[] = {"single target", "homogeneous framebuffer", "dishomogeneous framebuffer"};

static float clamp(float f)
{
	if(f >= 0.0f && f <= 1.0f)
		return f;
	else if(f > 0.0f)
		return 1.0f;
	else
		return 0.0f;
}

unsigned ati_driver;
unsigned nvidia_driver;

char test_name[4096];
float observed[16];
unsigned vert_clamp, frag_clamp, read_clamp;
float* expected;
float* expected1;
GLuint tex, tex1, fb;
GLenum status;
unsigned error;

int fbo_width = 2;
int fbo_height = 2;

#define TEST_NO_RT 0
#define TEST_SRT 1
#define TEST_MRT 2
#define TEST_SRT_MRT 3

int test_mode = TEST_SRT;

GLenum format;
const char* format_name;
GLboolean fixed;
GLboolean fixed0, fixed1;
unsigned mrt_mode;

GLboolean test();

GLboolean run_test()
{
	GLboolean pass = GL_TRUE;
	fixed = fixed0 = format == GL_RGBA8;
	fixed1 = -1;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, format,
		     2, 2, 0,
		     GL_RGBA, GL_FLOAT, pixels);
	error = glGetError();
	if(error)
	{
		printf("GL error after glTexImage2D 0x%04X\n", error);
		return GL_FALSE;
	}

	if(test_mode)
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
		if(error)
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

	if(test_mode <= TEST_SRT)
	{
		glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);
		pass = test();
	}
	else
	{
		unsigned mrt_modes = GLEW_ARB_draw_buffers ? (GLEW_ARB_texture_float ? 3 : 2) : 1;
		unsigned first_mrt_mode = (test_mode == TEST_MRT) ? 1 : 0;

		for(mrt_mode = first_mrt_mode; mrt_mode < mrt_modes; ++mrt_mode)
		{
			fixed1 = fixed;
			if(mrt_mode)
			{
				GLenum bufs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
				unsigned format1;
				if(mrt_mode == 1)
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
				if(error)
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
					if(mrt_mode == 2)
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
				if(error)
				{
					printf("GL error after second glDrawBuffers 0x%04X\n", error);
					return GL_FALSE;
				}
			}

			glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);
			pass = test() && pass;

	skip_mrt:
			if(mrt_mode)
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
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	if(fb)
	{
		glDeleteFramebuffersEXT(1, &fb);
		fb = 0;
	}

	error = glGetError();
	if(error)
	{
		printf("GL error after test 0x%04X\n", error);
		return GL_FALSE;
	}

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	unsigned error;
	printf("Testing 8-bit fixed-point FBO\n");
	format = GL_RGBA8;
	format_name = "un8";
	pass = run_test() && pass;

	if(GLEW_ARB_texture_float)
	{
		printf("\n\n\nTesting 32-bit floating point FBO\n");
		format = GL_RGBA32F_ARB;
		format_name = "f32";
		pass = run_test() && pass;
		printf("\n\n\nTesting 16-bit floating point FBO\n");
		format = GL_RGBA16F_ARB;
		format_name = "f16";
		pass = run_test() && pass;
	}
	else
		printf("\n\n\nSkipping floating point FBO tests because of no ARB_texture_float.\n");

	error = glGetError();
	if(error)
	{
		printf("GL error at end 0x%04X\n", error);
		return GL_FALSE;
	}

	return pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE;
}

unsigned init();

void
piglit_init(int argc, char **argv)
{
	int i;
	GLboolean distinguish_xfails = GL_FALSE;

	(void)i;
	/* displaying thousands of single-pixel floating point results isn't really useful, or even doable */
	piglit_automatic = GL_TRUE;

	piglit_require_extension("GL_ARB_color_buffer_float");

	test_mode = init();

	if(test_mode != TEST_NO_RT)
		piglit_require_extension("GL_EXT_framebuffer_object");

	for(i = 1; i < argc; ++i)
	{
		if(!strcmp(argv[i], "-xfail"))
			distinguish_xfails = GL_TRUE;
	}

	/* current ATI drivers are broken */
	if(!strcmp((char*)glGetString(GL_VENDOR), "ATI Technologies Inc."))
		ati_driver = 1;

	/* current nVidia drivers are broken at least on GeForce 7xxx */
	if(!strcmp((char*)glGetString(GL_VENDOR), "NVIDIA Corporation"))
		nvidia_driver = 1;

	if(ati_driver || nvidia_driver)
	{
		/* print both so users don't think either driver is better */
		printf("Notice: the ATI proprietary driver does NOT conform to the GL_ARB_color_buffer_float specification! (tested version was 10.6 on cypress, on Linux x86)\n");
		printf("Notice: the nVidia proprietary driver does NOT conform to the GL_ARB_color_buffer_float specification! (tested version was 256.44 on nv49, on Linux x86)\n");
		printf("Notice: the nVidia and ATI proprietary drivers are both nonconformant, in different ways!\n\n\n");
	}

	if(!distinguish_xfails)
		ati_driver = nvidia_driver = 0;

	for(i = 0; i < sizeof(pixels) / sizeof(pixels[0]); ++i)
	{
		clamped_pixels[i] = clamp(pixels[i]);
		pixels_mul_2[i] = pixels[i] * 2.0f;
		clamped_pixels_mul_2[i] = clamped_pixels[i] * 2.0f;
		pixels_plus_half[i] = pixels[i] + 0.5f;
		clamped_pixels_plus_half[i] = clamped_pixels[i] + 0.5f;
		clamped_pixels_plus_half_clamped[i] = clamp(clamped_pixels_plus_half[i]);
	}
}

GLboolean compare_arrays(float* expected, float* observed, int length)
{
	GLboolean pass = GL_TRUE;
	unsigned i;
	for(i = 0; i < length; ++i)
	{
		if(fabs(expected[i] - observed[i]) > 0.01)
		{
			printf("At %i Expected: %f Observed: %f\n", i, expected[i], observed[i]);
			pass = GL_FALSE;
		}
	}

	return pass;
}
