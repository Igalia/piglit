/* Copyright © 2020 Intel Corporation
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
 * Check that drivers track the inputs to the fast-clear (clear color, format,
 * etc.) to correctly write to a fast-cleared block.
 */

#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN

config.supports_gl_core_version = 43;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}


union color_value {
	uint32_t uint[4];
	float flt[4];
};

/* Returns the type of values used when specifying the clear color of a
 * texture with a given format.
 */
static GLenum
format_clear_value_type(GLuint format)
{
	switch (format) {
	case GL_RGBA8:
	case GL_SRGB8_ALPHA8:
	case GL_RGBA8_SNORM:
		return GL_FLOAT;
	case GL_RGBA8UI:
		return GL_UNSIGNED_INT;
	default:
		abort();
	}
}

/* Clears a subregion of a texture starting from the origin. */
static void
tex_sub_clear(GLuint tex, GLuint format, union color_value color,
	      uint32_t w, uint32_t h)
{
	/* Perform a scissored clear through an fbo, so that the clear color
	 * is interpreted through the texture format.
	 */
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, tex, 0);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_SCISSOR_TEST);
	glScissor(0, 0, w, h);

	if (format_clear_value_type(format) == GL_UNSIGNED_INT) {
		glClearBufferuiv(GL_COLOR, 0, color.uint);
	} else {
		assert(format_clear_value_type(format) == GL_FLOAT);
		glClearBufferfv(GL_COLOR, 0, color.flt);
	}

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_FRAMEBUFFER_SRGB);
	glDeleteFramebuffers(1, &fbo);
}

struct clear_list {
	GLuint format;
	uint32_t z;
	uint32_t w;
	uint32_t h;
	union color_value color;
};

/* Clears a texture's data store according to the list then probes for a
 * specific pixel on layer 0.
 */
static bool
test_clear_list(GLuint tex_format, uint32_t tw, uint32_t th, uint32_t td,
		uint32_t num_clears, const struct clear_list *list,
		uint32_t px, uint32_t py,
		union color_value probe_pix)
{
	/* Create the texture storage. */
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, tex_format, tw, th, td);

	/* Clear the views. */
	for (int i = 0; i < num_clears; i++) {
		GLuint view;
		glGenTextures(1, &view);
		glTextureView(view, GL_TEXTURE_2D, tex, list[i].format,
			      0, 1, list[i].z, 1);
		tex_sub_clear(view, list[i].format, list[i].color,
			      list[i].w, list[i].h);
		glDeleteTextures(1, &view);
	}

	/* Inspect the texture. */
	assert(format_clear_value_type(tex_format) == GL_FLOAT);
	const bool matched_pixel =
		piglit_probe_texel_volume_rgba(GL_TEXTURE_2D_ARRAY, 0,
					       px, py, 0, 1, 1, 1,
					       probe_pix.flt);

	/* Delete the texture. */
	glDeleteTextures(1, &tex);
	return matched_pixel;
}

#define entry(fmt, layer, width, height, col) \
	{ .format = fmt, .z = layer, .w = width, . h = height, .color = col }

static bool
test_clear_after_clear(GLuint tex_format, uint32_t tw, uint32_t th,
		       union color_value tex_color,
		       GLuint view_format, uint32_t vw, uint32_t vh,
		       union color_value view_color,
		       uint32_t px, uint32_t py,
		       union color_value probe_pix)
{
	const struct clear_list list[] = {
	  entry(tex_format, 0, tw, th, tex_color),
	  entry(view_format, 0, vw, vh, view_color)
	};
	return test_clear_list(tex_format, tw, th, 1,
			       2, list, px, py, probe_pix);
}

static void
color_value_linear_to_srgb(const union color_value *linear,
                           union color_value *srgb)
{
	for (int i = 0; i < 3; i++) {
		srgb->flt[i] =
			piglit_linear_to_srgb(linear->flt[i]);
	}
	srgb->flt[3] = linear->flt[3];
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	const union color_value flt_one = { .flt = {1.0, 1.0, 1.0, 1.0} };
	const union color_value flt_half = { .flt = {0.5, 0.5, 0.5, 0.5} };
	union color_value half_linear_to_srgb;
	color_value_linear_to_srgb(&flt_half, &half_linear_to_srgb);

	/* Depending on the clear color and view format a resolve may be
	 * needed before reading a fast-cleared block. On gen7+, such a block
	 * is implicitly read when part of it is written to. On gen12, it may
	 * also be implicitly read when all of it is written to.
	 *
	 * These additional properties should be noted for test creation:
	 * * On gen7-8, the fast-clear channel values allowed are 0 for any
	 *   format, 1.0 for floats, and 1 for ints.
	 * * On gen12, all compression is lost when a texture format's
	 *   bits-per-channel changes.
	 */

	puts("Testing implicit read of partial block "
	     "(linear storage) linear ->  sRGB");
	pass &= test_clear_after_clear(GL_RGBA8, 32, 32, flt_half,
				       GL_SRGB8_ALPHA8, 1, 1, flt_one,
				       0, 1, flt_half);

	puts("Testing implicit read of partial block "
	     "(sRGB storage) linear ->  sRGB");
	{
		const struct clear_list list[] = {
		  entry(GL_RGBA8, 0, 32, 32, flt_half),
		  entry(GL_SRGB8_ALPHA8, 0, 1, 1, flt_half),
		};
		pass &= test_clear_list(GL_SRGB8_ALPHA8, 32, 32, 1,
				        2, list, 0, 1, flt_half);
	}

	puts("Testing implicit read of partial block sRGB -> linear");
	pass &= test_clear_after_clear(GL_SRGB8_ALPHA8, 32, 32, flt_half,
				       GL_RGBA8, 1, 1, flt_one,
				       0, 1, half_linear_to_srgb);

	puts("Testing implicit read of partial block sRGB -> sRGB");
	pass &= test_clear_after_clear(GL_SRGB8_ALPHA8, 32, 32, flt_half,
				       GL_SRGB8_ALPHA8, 1, 1, flt_one,
				       0, 1, half_linear_to_srgb);

	puts("Testing implicit read of partial block UNORM -> SNORM");
	pass &= test_clear_after_clear(GL_RGBA8, 32, 32, flt_one,
				       GL_RGBA8_SNORM, 1, 1, flt_one,
				       0, 1, flt_one);

	puts("Testing implicit read of full block UNORM -> SNORM");
	pass &= test_clear_after_clear(GL_RGBA8, 32, 32, flt_one,
				       GL_RGBA8_SNORM, 8, 4, flt_one,
				       0, 0, flt_half);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
