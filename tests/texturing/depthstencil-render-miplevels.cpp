/*
 * Copyright © 2012 Intel Corporation
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

/** \file depthstencil-render-miplevels.cpp
 *
 * Test that data rendered to depth and stencil textures
 * always lands at the correct miplevel.
 *
 * This test operates by creating a set of texture buffers, attaching
 * them to a framebuffer one miplevel at a time, and rendering
 * different data into each miplevel.  Then it verifies, using
 * glReadPixels, that the correct data appears at each miplevel.
 *
 * This is useful in diagnosing bugs such as:
 *
 * - Incorrect miplevels being attached to the framebuffer
 *
 * - Miplevels being laid out incorrectly in memory (e.g. in an
 *   overlapping fashion)
 *
 * Usage: depthstencil-render-miplevels <texture_size> <buffer_combination>
 *
 *   Available buffer combinations:
 *   s=z24_s8
 *   d=z24_s8
 *   d=z24
 *   d=z32f_s8
 *   d=z32f
 *   d=z16
 *   d=z24_s8_s=z24_s8
 *   d=z24_s=z24_s8
 *   s=z24_s8_d=z24_s8
 *   s=z24_s8_d=z24
 *   d=s=z24_s8
 *   s=d=z24_s8
 *   ds=z24_s8
 *   d=z32f_s8_s=z24_s8
 *   d=z32f_s=z24_s8
 *   s=z24_s8_d=z32f_s8
 *   s=z24_s8_d=z32f
 *   d=s=z32f_s8
 *   s=d=z32f_s8
 *   ds=z32f_s8
 *
 * Buffer combination is interpreted as follows:
 *   s=GL_STENCIL_ATTACHMENT
 *   d=GL_DEPTH_ATTACHMENT
 *   ds=GL_DEPTH_STENCIL_ATTACHMENT
 *   z16=GL_DEPTH_COMPONENT16
 *   z24=GL_DEPTH_COMPONENT24
 *   z32f=GL_DEPTH_COMPONENT32f
 *   z24_s8=GL_DEPTH24_STENCIL8
 *   z32f_s8=GL_DEPTH32F_STENCIL8
 *
 * So, for example:
 *
 * - d=z24_s=z24_s8 means that GL_DEPTH_ATTACHMENT is bound to a
 *   buffer of type GL_DEPTH_COMPONENT24, and then
 *   GL_STENCIL_ATTACHMENT is bound to a buffer of type
 *   GL_DEPTH24_STENCIL8.
 *
 * - s=d=z32f_s8 means that GL_STENCIL_ATTACHMENT is bound to a buffer
 *   of type GL_DEPTH32F_STENCIL8 and then GL_DEPTH_ATTACHMENT is
 *   bound to the same buffer.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 512;
	config.window_height = 512;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

namespace {

GLuint color_tex;
GLuint depth_tex;
GLuint stencil_tex;
bool attach_depth = false;
bool attach_stencil = false;
bool shared_attachment = false;
bool attach_together = false;
bool attach_stencil_first = false;
GLenum depth_format;
int miplevel0_size;
int max_miplevel;
float **depth_miplevel_data;
uint8_t **stencil_miplevel_data;

/**
 * Check if the given depth/stencil/rgba texture internal format is supported.
 */
static bool
supported_format(GLenum internal_format)
{
	switch (internal_format) {
	case GL_DEPTH_COMPONENT32F:
	case GL_DEPTH32F_STENCIL8:
		return piglit_is_extension_supported("GL_ARB_depth_buffer_float");
	case GL_RGBA:
	case GL_DEPTH_COMPONENT16:
	case GL_DEPTH_COMPONENT24:
	case GL_DEPTH24_STENCIL8:
	default:
		return true;
	}
}


/**
 * Create a mipmapped texture with the given dimensions and internal format.
 */
GLuint
create_mipmapped_tex(GLenum internal_format)
{
	GLenum format = GL_NONE;

	if (!supported_format(internal_format))
		return 0;

	switch (internal_format) {
	case GL_RGBA:
		format = GL_RGBA;
		break;
	case GL_DEPTH_COMPONENT16:
	case GL_DEPTH_COMPONENT24:
	case GL_DEPTH_COMPONENT32F:
		format = GL_DEPTH_COMPONENT;
		break;
	case GL_DEPTH24_STENCIL8:
	case GL_DEPTH32F_STENCIL8:
		format = GL_DEPTH_STENCIL;
		break;
	default:
		printf("Unexpected internal_format in create_mipmapped_tex\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	GLenum type = format == GL_DEPTH_STENCIL
		? GL_UNSIGNED_INT_24_8 : GL_UNSIGNED_BYTE;
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	for (int level = 0; level <= max_miplevel; ++level) {
		int dim = miplevel0_size >> level;
		glTexImage2D(GL_TEXTURE_2D, level, internal_format,
			     dim, dim,
			     0,
			     format, type, NULL);
		if (!piglit_check_gl_error(GL_NO_ERROR))
			piglit_report_result(PIGLIT_FAIL);
	}
	return tex;
}

/**
 * Attach the proper miplevel of each texture to the framebuffer
 */
void
set_up_framebuffer_for_miplevel(int level)
{
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
			       GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			       color_tex, level);
	if (attach_together) {
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
				       GL_DEPTH_STENCIL_ATTACHMENT,
				       GL_TEXTURE_2D, depth_tex, level);
	} else if (attach_stencil_first) {
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
				       GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
				       stencil_tex, level);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
				       GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
				       depth_tex, level);
	} else {
		if (attach_depth) {
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
					       GL_DEPTH_ATTACHMENT,
					       GL_TEXTURE_2D,
					       depth_tex, level);
		}
		if (attach_stencil) {
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
					       GL_STENCIL_ATTACHMENT,
					       GL_TEXTURE_2D,
					       stencil_tex, level);
		}
	}

	/* Some implementations don't support certain buffer
	 * combinations, and that's ok, provided that the
	 * implementation reports GL_FRAMEBUFFER_UNSUPPORTED.
	 * However, if the buffer combination was supported at
	 * miplevel 0, it should be supported at all miplevels.
	 */
	GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (status == GL_FRAMEBUFFER_UNSUPPORTED && level == 0) {
		printf("This buffer combination is unsupported\n");
		piglit_report_result(PIGLIT_SKIP);
	} else if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FBO incomplete at miplevel %d\n", level);
		piglit_report_result(PIGLIT_FAIL);
	}
}

uint8_t
stencil_for_level(int level)
{
	float float_value = float(level + 1) / (max_miplevel + 1);
	return (uint8_t) round(float_value * 255.0);
}

/**
 * Using glClear, set the contents of the depth and stencil buffers
 * (if present) to a value that is unique to this miplevel.
 */
void
populate_miplevel(int level)
{
	float float_value = float(level + 1) / (max_miplevel + 1);
	GLbitfield clear_mask = 0;

	if (attach_depth) {
		glClearDepth(float_value);
		clear_mask |= GL_DEPTH_BUFFER_BIT;
	}
	if (attach_stencil) {
		glClearStencil(stencil_for_level(level));
		clear_mask |= GL_STENCIL_BUFFER_BIT;
	}

	glClear(clear_mask);
}

/**
 * Test that every pixel in the depth and stencil buffers (if present)
 * is equal to the value set by populate_miplevel.
 *
 * If we're going to later render our results to the screen for
 * debugging, then save off a copy of the data we read now.
 */
bool
test_miplevel(int level)
{
	bool pass = true;
	int dim = miplevel0_size >> level;
	float float_value = float(level + 1) / (max_miplevel + 1);

	if (attach_depth) {
		printf("Probing miplevel %d depth\n", level);
		pass = piglit_probe_rect_depth(0, 0, dim, dim, float_value)
			&& pass;

		if (!piglit_automatic) {
			depth_miplevel_data[level] =
				(float *)malloc(4 * dim * dim);
			glReadPixels(0, 0, dim, dim,
				     GL_DEPTH_COMPONENT, GL_FLOAT,
				     depth_miplevel_data[level]);
		}
	}

	if (attach_stencil) {
		printf("Probing miplevel %d stencil\n", level);
		pass = piglit_probe_rect_stencil(0, 0, dim, dim,
						 stencil_for_level(level))
			&& pass;

		if (!piglit_automatic) {
			stencil_miplevel_data[level] =
				(uint8_t *)malloc(dim * dim);
			glReadPixels(0, 0, dim, dim,
				     GL_STENCIL_INDEX, GL_UNSIGNED_BYTE,
				     stencil_miplevel_data[level]);
		}
	}

	return pass;
}

void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <texture_size> <buffer_combination>\n"
	       "    Available buffer combinations:\n"
	       "    s=z24_s8\n"
	       "    d=z24_s8\n"
	       "    d=z24\n"
	       "    d=z32f_s8\n"
	       "    d=z32f\n"
	       "    d=z16\n"
	       "    d=z24_s8_s=z24_s8\n"
	       "    d=z24_s=z24_s8\n"
	       "    s=z24_s8_d=z24_s8\n"
	       "    s=z24_s8_d=z24\n"
	       "    d=s=z24_s8\n"
	       "    s=d=z24_s8\n"
	       "    ds=z24_s8\n"
	       "    d=z32f_s8_s=z24_s8\n"
	       "    d=z32f_s=z24_s8\n"
	       "    s=z24_s8_d=z32f_s8\n"
	       "    s=z24_s8_d=z32f\n"
	       "    d=s=z32f_s8\n"
	       "    s=d=z32f_s8\n"
	       "    ds=z32f_s8\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

extern "C" void
piglit_init(int argc, char **argv)
{
	if (argc != 3) {
		print_usage_and_exit(argv[0]);
	}

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/* argv[1]: texture size */
	{
		char *endptr = NULL;
		miplevel0_size = strtol(argv[1], &endptr, 0);
		if (endptr != argv[1] + strlen(argv[1]))
			print_usage_and_exit(argv[0]);

		/* Now figure out the appropriate value of max_miplevel for this size. */
		max_miplevel = 0;
		while ((miplevel0_size >> (max_miplevel + 1)) > 0)
			++max_miplevel;
	}

	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_depth_texture");
	piglit_require_extension("GL_ARB_texture_non_power_of_two");

	depth_miplevel_data = (float **)calloc(max_miplevel, sizeof(float *));
	stencil_miplevel_data = (uint8_t **)calloc(max_miplevel,
						   sizeof(uint8_t *));

	/* argv[2]: buffer combination */
	if (strcmp(argv[2], "s=z24_s8") == 0) {
		attach_stencil = true;
	} else if (strcmp(argv[2], "d=z24_s8") == 0) {
		attach_depth = true;
		depth_format = GL_DEPTH24_STENCIL8;
	} else if (strcmp(argv[2], "d=z24") == 0) {
		attach_depth = true;
		depth_format = GL_DEPTH_COMPONENT24;
	} else if (strcmp(argv[2], "d=z32f_s8") == 0) {
		attach_depth = true;
		depth_format = GL_DEPTH32F_STENCIL8;
	} else if (strcmp(argv[2], "d=z32f") == 0) {
		attach_depth = true;
		depth_format = GL_DEPTH_COMPONENT32F;
	} else if (strcmp(argv[2], "d=z16") == 0) {
		attach_depth = true;
		depth_format = GL_DEPTH_COMPONENT16;
	} else if (strcmp(argv[2], "d=z24_s8_s=z24_s8") == 0) {
		attach_depth = true;
		attach_stencil = true;
		depth_format = GL_DEPTH24_STENCIL8;
	} else if (strcmp(argv[2], "d=z24_s=z24_s8") == 0) {
		attach_depth = true;
		attach_stencil = true;
		depth_format = GL_DEPTH_COMPONENT24;
	} else if (strcmp(argv[2], "s=z24_s8_d=z24_s8") == 0) {
		attach_depth = true;
		attach_stencil = true;
		attach_stencil_first = true;
		depth_format = GL_DEPTH24_STENCIL8;
	} else if (strcmp(argv[2], "s=z24_s8_d=z24") == 0) {
		attach_depth = true;
		attach_stencil = true;
		attach_stencil_first = true;
		depth_format = GL_DEPTH_COMPONENT24;
	} else if (strcmp(argv[2], "d=s=z24_s8") == 0) {
		attach_depth = true;
		attach_stencil = true;
		shared_attachment = true;
		depth_format = GL_DEPTH24_STENCIL8;
	} else if (strcmp(argv[2], "s=d=z24_s8") == 0) {
		attach_depth = true;
		attach_stencil = true;
		shared_attachment = true;
		attach_stencil_first = true;
		depth_format = GL_DEPTH24_STENCIL8;
	} else if (strcmp(argv[2], "ds=z24_s8") == 0) {
		attach_depth = true;
		attach_stencil = true;
		shared_attachment = true;
		attach_together = true;
		depth_format = GL_DEPTH24_STENCIL8;
	} else if (strcmp(argv[2], "d=z32f_s8_s=z24_s8") == 0) {
		attach_depth = true;
		attach_stencil = true;
		depth_format = GL_DEPTH32F_STENCIL8;
	} else if (strcmp(argv[2], "d=z32f_s=z24_s8") == 0) {
		attach_depth = true;
		attach_stencil = true;
		depth_format = GL_DEPTH_COMPONENT32F;
	} else if (strcmp(argv[2], "s=z24_s8_d=z32f_s8") == 0) {
		attach_depth = true;
		attach_stencil = true;
		attach_stencil_first = true;
		depth_format = GL_DEPTH32F_STENCIL8;
	} else if (strcmp(argv[2], "s=z24_s8_d=z32f") == 0) {
		attach_depth = true;
		attach_stencil = true;
		attach_stencil_first = true;
		depth_format = GL_DEPTH_COMPONENT32F;
	} else if (strcmp(argv[2], "d=s=z32f_s8") == 0) {
		attach_depth = true;
		attach_stencil = true;
		shared_attachment = true;
		depth_format = GL_DEPTH32F_STENCIL8;
	} else if (strcmp(argv[2], "s=d=z32f_s8") == 0) {
		attach_depth = true;
		attach_stencil = true;
		shared_attachment = true;
		attach_stencil_first = true;
		depth_format = GL_DEPTH32F_STENCIL8;
	} else if (strcmp(argv[2], "ds=z32f_s8") == 0) {
		attach_depth = true;
		attach_stencil = true;
		shared_attachment = true;
		attach_together = true;
		depth_format = GL_DEPTH32F_STENCIL8;
	} else {
		print_usage_and_exit(argv[0]);
	}
}

static void
render_tex_to_screen(GLuint tex, int x, int y)
{
	glBindTexture(GL_TEXTURE_2D, tex);
	glEnable(GL_TEXTURE_2D);

	for (int level = 0; level <= max_miplevel; ++level) {
		int dim = miplevel0_size >> level;

		piglit_draw_rect_tex(x, y, dim, dim,
				     0, 0, 1, 1);

		y += dim + 1;
	}
}

/**
 * Presents the results of the rendering on the screen.
 */
static void
render_results_to_screen()
{
	GLuint tex;

	printf("\n");
	printf("Depth is on the left, stencil is on the right.\n");
	printf("Colors should proceed from nearly-black to nearly-red.\n");

	/* If the miptree is too large, scale things down. We don't
	 * actually use miptrees to draw our miptree, so it'll work
	 * out.
	 */
	piglit_ortho_projection(MAX2(piglit_width, 2 * miplevel0_size),
				MAX2(piglit_height, 2 * miplevel0_size),
				false);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glClearColor(0.5, 0.5, 0.5, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	if (attach_depth) {
		for (int level = 0; level <= max_miplevel; ++level) {
			int dim = miplevel0_size >> level;

			glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA,
				     dim, dim,
				     0,
				     GL_RED, GL_FLOAT,
				     depth_miplevel_data[level]);
			if (!piglit_check_gl_error(GL_NO_ERROR))
				piglit_report_result(PIGLIT_FAIL);
		}

		render_tex_to_screen(tex, 0, 1);
	}


	if (attach_stencil) {
		for (int level = 0; level <= max_miplevel; ++level) {
			int dim = miplevel0_size >> level;

			glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA,
				     dim, dim,
				     0,
				     GL_RED, GL_UNSIGNED_BYTE,
				     stencil_miplevel_data[level]);
			if (!piglit_check_gl_error(GL_NO_ERROR))
				piglit_report_result(PIGLIT_FAIL);
		}

		render_tex_to_screen(tex, miplevel0_size + 10, 1);
	}

	piglit_present_results();
}

extern "C" enum piglit_result
piglit_display()
{
	bool pass = true;

	color_tex = create_mipmapped_tex(GL_RGBA);

	if (attach_depth) {
		depth_tex = create_mipmapped_tex(depth_format);
		if (!depth_tex) {
			/* unsupported format */
			piglit_report_result(PIGLIT_SKIP);
		}
	}

	if (attach_stencil) {
		if (shared_attachment) {
			stencil_tex = depth_tex;
		} else {
			stencil_tex = create_mipmapped_tex(GL_DEPTH24_STENCIL8);
		}
		if (!stencil_tex) {
			/* unsupported format */
			piglit_report_result(PIGLIT_SKIP);
		}
	}

	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

	for (int level = 0; level <= max_miplevel; ++level) {
		set_up_framebuffer_for_miplevel(level);
		populate_miplevel(level);
	}
	for (int level = 0; level <= max_miplevel; ++level) {
		set_up_framebuffer_for_miplevel(level);
		pass = test_miplevel(level) && pass;
	}

	if (!piglit_automatic)
		render_results_to_screen();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

}; /* Anonymous namespace */
