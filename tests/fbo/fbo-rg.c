/*
 * Copyright © 2010 Intel Corporation
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

/**
 * \file fbo-rg.c
 * Validate FBO rendering to RED and RG textures.
 *
 * Select a RED or RG format based on the command line parameter.  Create an
 * RGBA FBO and a RED/RG FBO.  Draw the same scene to both textures.  Read back
 * both textures.  Validate that the red channels of both textures
 * are the same (and contain some non-zero texels).  If the base format is
 * RG, perform similar validation on the green channel.  Validate that the other
 * channels of the RED/RG texture are 0 (green and blue) and 1 (alpha).
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util-gl.h"


#define EPSILON (1.0 / 255.0)

static GLuint rgba_tex;
static GLuint other_tex;
static GLboolean pass = GL_TRUE;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const char vert_code[] =
	"attribute vec2 position;\n"
	"attribute vec4 color;\n"
	"varying vec4 fc;\n"
	"void main()\n"
	"{\n"
	"    gl_Position = vec4(position, 0.0, 1.0);\n"
	"    fc = color;\n"
	"}\n";

static const char frag_code[] =
	"uniform float scale;\n"
	"uniform float bias;\n"
	"varying vec4 fc;\n"
	"void main()\n"
	"{"
	"    gl_FragColor = (fc * scale) + bias;\n"
	"}\n"
	;

/**
 * GLSL shader program used to render to the FBO.
 */
static GLuint fbo_program;

enum piglit_result
piglit_display(void)
{
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, rgba_tex);
	piglit_draw_rect_tex(-1.0, -1.0, 1.0, 2.0,
			     0.0, 0.0, 1.0, 1.0);

	glBindTexture(GL_TEXTURE_2D, other_tex);
	piglit_draw_rect_tex(0.0, -1.0, 1.0, 2.0,
			     0.0, 0.0, 1.0, 1.0);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static int
create_fbo(unsigned width, unsigned height, GLenum internal_format)
{
	GLuint tex;
	GLuint fb;
	GLenum status;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height,
		     0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D, tex, 0);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		if (status == GL_FRAMEBUFFER_UNSUPPORTED_EXT)
			printf("FBO with 0x%04x texture is unsupported\n",
			       internal_format);
		else
			fprintf(stderr, "FBO with 0x%04x texture is incomplete"
				" (0x%04x)\n",
				internal_format, status);

		glBindFramebufferEXT(GL_FRAMEBUFFER, piglit_winsys_fbo);
		glDeleteFramebuffersEXT(1, &fb);
		glDeleteTextures(1, &tex);
		piglit_report_result((status == GL_FRAMEBUFFER_UNSUPPORTED_EXT)
				     ? PIGLIT_SKIP : PIGLIT_FAIL);
	}

	return fb;
}

static GLboolean
compare_texture(const GLfloat *orig, const GLfloat *copy,
		GLenum orig_fmt, GLenum copy_fmt, unsigned num_pix,
		GLboolean has_green)
{
	GLboolean logged = GL_FALSE;
	GLboolean pass = GL_TRUE;
	unsigned i;
	unsigned non_zero_red = 0;
	unsigned non_zero_green = 0;

	for (i = 0; i < num_pix; i++) {
		if (fabs(orig[0] - copy[0]) > EPSILON) {
			if (!logged) {
				fprintf(stderr,
					"Got bad R channel reading back "
					"0x%04x as 0x%04x\n",
					orig_fmt, copy_fmt);
				logged = GL_TRUE;
			}

			pass = GL_FALSE;
		}

		if (has_green && fabs(orig[0] - copy[0]) > EPSILON) {
			if (!logged) {
				fprintf(stderr,
					"Got bad G channel reading back "
					"0x%04x as 0x%04x\n",
					orig_fmt, copy_fmt);
				logged = GL_TRUE;
			}

			pass = GL_FALSE;
		}

		if ((!has_green && copy[1] != 0.0)
		    || copy[2] != 0.0
		    || copy[3] != 1.0) {
			if (!logged) {
				fprintf(stderr,
					"Got bad %s channel reading back "
					"0x%04x as 0x%04x\n",
					has_green ? "B/A" : "G/B/A",
					orig_fmt, copy_fmt);
				logged = GL_TRUE;
			}

			pass = GL_FALSE;
		}

		if (copy[0] != 0.0)
			non_zero_red++;

		if (copy[1] != 0.0)
			non_zero_green++;

		orig += 4;
		copy += 4;
	}

	if (non_zero_red == 0) {
		fprintf(stderr,
			"All red components are zero reading back "
			"0x%04x as 0x%04x\n",
			orig_fmt, copy_fmt);
		pass = GL_FALSE;
	}

	if (has_green && non_zero_green == 0) {
		fprintf(stderr,
			"All green components are zero reading back "
			"0x%04x as 0x%04x\n",
			orig_fmt, copy_fmt);
		pass = GL_FALSE;
	}

	return pass;
}

static const float positions[] = {
	-1.0, -1.0,
	 1.0, -1.0,
	 1.0,  1.0,
	-1.0,  1.0,
};

static const float colors[] = {
	1.0, 0.2, 0.9, 1.0,
	0.8, 0.4, 0.9, 1.0,
	0.4, 0.8, 0.9, 1.0,
	0.2, 1.0, 0.9, 1.0,
};

static GLboolean
render_and_check_textures(GLenum internal_format)
{
	GLuint rgba_fb;
	GLuint other_fb;
	float rgba_image[4 * 64 * 64];
	float other_image[4 * 64 * 64];
	GLboolean has_green;
	GLuint vs;
	GLuint fs;
	GLint scale_loc;
	GLint bias_loc;
	float scale;
	float bias;

	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_ARB_texture_rg");

	has_green = GL_FALSE;
	scale = 1.0;
	bias = 0.0;
	switch (internal_format) {
	case GL_RG:
	case GL_RG8:
	case GL_RG16:
		has_green = GL_TRUE;
		/* FALLTHROUGH */
	case GL_RED:
	case GL_R8:
	case GL_R16:
		break;

	case GL_RG16F:
		has_green = GL_TRUE;
		/* FALLTHROUGH */
	case GL_R16F:
		piglit_require_extension("GL_ARB_half_float_pixel");
		/* FALLTHROUGH */
	case GL_RG32F:
		has_green = GL_TRUE;
		/* FALLTHROUGH */
	case GL_R32F:
		scale = 511.0;
		piglit_require_extension("GL_ARB_texture_float");
		break;

	case GL_RG_INTEGER:
	case GL_RG8I:
	case GL_RG16I:
	case GL_RG32I:
		has_green = GL_TRUE;
		/* FALLTHROUGH */
	case GL_R8I:
	case GL_R16I:
	case GL_R32I:
		bias = -100.0;
		scale = 511.0;
		piglit_require_extension("GL_EXT_texture_integer");
		break;

	case GL_RG8UI:
	case GL_RG16UI:
	case GL_RG32UI:
		has_green = GL_TRUE;
		/* FALLTHROUGH */
	case GL_R16UI:
	case GL_R32UI:
		scale = 511.0;
		piglit_require_extension("GL_EXT_texture_integer");
		break;

	case GL_RG_SNORM:
	case GL_RG8_SNORM:
	case GL_RG16_SNORM:
		has_green = GL_TRUE;
		/* FALLTHROUGH */
	case GL_RED_SNORM:
	case GL_R8_SNORM:
	case GL_R16_SNORM:
		scale = 0.5;
		bias = -0.5;
		piglit_require_extension("GL_EXT_texture_snorm");
		break;
	default:
		fprintf(stderr, "invalid format 0x%04x\n", internal_format);
		piglit_report_result(PIGLIT_FAIL);
		break;
	}

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
			      2 * sizeof(GLfloat), positions);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
			      4 * sizeof(GLfloat), colors);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert_code);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag_code);
	fbo_program = piglit_link_simple_program(vs, fs);

	glBindAttribLocation(fbo_program, 0, "position");
	glBindAttribLocation(fbo_program, 1, "color");
	glLinkProgram(fbo_program);
	if (!piglit_link_check_status(fbo_program))
		piglit_report_result(PIGLIT_FAIL);

	scale_loc = glGetUniformLocation(fbo_program, "scale");
	if (scale_loc < 0) {
		fprintf(stderr,
			"couldn't get uniform location for \"scale\"\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	bias_loc = glGetUniformLocation(fbo_program, "bias");
	if (bias_loc < 0) {
		fprintf(stderr,
			"couldn't get uniform location for \"bias\"\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glUseProgram(fbo_program);
	glUniform1f(scale_loc, scale);
	glUniform1f(bias_loc, bias);

	/* Draw the reference image to the RGBA texture.
	 */
	rgba_fb = create_fbo(64, 64, GL_RGBA);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rgba_fb);
	glViewport(0, 0, 64, 64);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT,
						 GL_COLOR_ATTACHMENT0_EXT,
						 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT,
						 (GLint *) &rgba_tex);
	glBindTexture(GL_TEXTURE_2D, rgba_tex);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, rgba_image);


	/* Draw the comparison image to the other texture.
	 */
	other_fb = create_fbo(64, 64, internal_format);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, other_fb);
	glViewport(0, 0, 64, 64);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT,
						 GL_COLOR_ATTACHMENT0_EXT,
						 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT,
						 (GLint *) &other_tex);
	glBindTexture(GL_TEXTURE_2D, other_tex);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, other_image);

	glUseProgram(0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glViewport(0, 0, piglit_width, piglit_height);

	return compare_texture(rgba_image, other_image,
			       internal_format, GL_RGBA,
			       64 * 64, has_green);
}

static void
usage(const char *name)
{
	fprintf(stderr,
		"Usage: %s internal_format\n"
		"where internal_format is one of GL_RED, GL_R8, GL_RG, "
		"GL_RG8, etc.\n",
		name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	GLenum internal_format = GL_NONE;
	const char *fmt;

	if ((argc == 1) || (strncmp(argv[1], "GL_R", 4) != 0))
		usage(argv[0]);

	fmt = argv[1];
	if (fmt[4] == 'G') {
		if (strcmp("GL_RG", fmt) == 0) {
			internal_format = GL_RG;
		} else if (strcmp("GL_RG_INTEGER", fmt) == 0) {
			internal_format = GL_RG_INTEGER;
		} else if (strcmp("GL_RG8", fmt) == 0) {
			internal_format = GL_RG8;
		} else if (strcmp("GL_RG8I", fmt) == 0) {
			internal_format = GL_RG8I;
		} else if (strcmp("GL_RG8UI", fmt) == 0) {
			internal_format = GL_RG8UI;
		} else if (strcmp("GL_RG16", fmt) == 0) {
			internal_format = GL_RG16;
		} else if (strcmp("GL_RG16F", fmt) == 0) {
			internal_format = GL_RG16F;
		} else if (strcmp("GL_RG16I", fmt) == 0) {
			internal_format = GL_RG16I;
		} else if (strcmp("GL_RG16UI", fmt) == 0) {
			internal_format = GL_RG16UI;
		} else if (strcmp("GL_RG32F", fmt) == 0) {
			internal_format = GL_RG32F;
		} else if (strcmp("GL_RG32I", fmt) == 0) {
			internal_format = GL_RG32I;
		} else if (strcmp("GL_RG32UI", fmt) == 0) {
			internal_format = GL_RG32UI;
		} else if (strcmp("GL_RG_SNORM", fmt) == 0) {
			internal_format = GL_RG_SNORM;
		} else if (strcmp("GL_RG8_SNORM", fmt) == 0) {
			internal_format = GL_RG8_SNORM;
		} else if (strcmp("GL_RG16_SNORM", fmt) == 0) {
			internal_format = GL_RG16_SNORM;
		} else
			usage(argv[0]);
	} else {
		if (strcmp("GL_RED", fmt) == 0) {
			internal_format = GL_RED;
		} else if (strcmp("GL_R8", fmt) == 0) {
			internal_format = GL_R8;
		} else if (strcmp("GL_R16", fmt) == 0) {
			internal_format = GL_R16;
		} else if (strcmp("GL_R16F", fmt) == 0) {
			internal_format = GL_R16F;
		} else if (strcmp("GL_R32F", fmt) == 0) {
			internal_format = GL_R32F;
		} else if (strcmp("GL_R8I", fmt) == 0) {
			internal_format = GL_R8I;
		} else if (strcmp("GL_R8UI", fmt) == 0) {
			internal_format = GL_R8UI;
		} else if (strcmp("GL_R16I", fmt) == 0) {
			internal_format = GL_R16I;
		} else if (strcmp("GL_R16UI", fmt) == 0) {
			internal_format = GL_R16UI;
		} else if (strcmp("GL_R32I", fmt) == 0) {
			internal_format = GL_R32I;
		} else if (strcmp("GL_R32UI", fmt) == 0) {
			internal_format = GL_R32UI;
		} else if (strcmp("GL_RED_SNORM", fmt) == 0) {
			internal_format = GL_RED_SNORM;
		} else if (strcmp("GL_R8_SNORM", fmt) == 0) {
			internal_format = GL_R8_SNORM;
		} else if (strcmp("GL_R16_SNORM", fmt) == 0) {
			internal_format = GL_R16_SNORM;
		} else
			usage(argv[0]);
	}

	pass = render_and_check_textures(internal_format);

	if (piglit_automatic)
		piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
