/* Copyright © 2011 Intel Corporation
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
 * LIABILITY, WHETHER INd AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "piglit-util-gl.h"
#include "sized-internalformats.h"

/**
 * @file sized-texture-format-channels.c
 *
 * Tests that sized internalformats of textures have the correct set
 * of channels exposed.
 *
 * From the GL 2.1 specification page 153 (page 167 of the PDF):
 *
 *     "If a sized internal format is specified, the mapping of the R,
 *      G, B, A, and depth values to texture components is equivalent
 *      to the mapping of the correspond- ing base internal format’s
 *      components, as specified in table 3.15, and the memory
 *      allocation per texture component is assigned by the GL to
 *      match the allocations listed in table 3.16 as closely as
 *      possible. (The definition of closely is left up to the
 *      implementation. However, a non-zero number of bits must be
 *      allocated for each component whose desired allocation in table
 *      3.16 is non-zero, and zero bits must be allocated for all
 *      other components)."
 *
 * This test will fail if for the zero vs non-zero cases above, and
 * will informationally print the channel sizes when they don't
 * exactly match.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

GLenum type_queries[CHANNELS] = {
	GL_TEXTURE_RED_TYPE,
	GL_TEXTURE_GREEN_TYPE,
	GL_TEXTURE_BLUE_TYPE,
	GL_TEXTURE_ALPHA_TYPE,
	GL_TEXTURE_LUMINANCE_TYPE,
	GL_TEXTURE_INTENSITY_TYPE,
	GL_TEXTURE_DEPTH_TYPE,
	GL_NONE,
};

GLenum size_queries[CHANNELS] = {
	GL_TEXTURE_RED_SIZE,
	GL_TEXTURE_GREEN_SIZE,
	GL_TEXTURE_BLUE_SIZE,
	GL_TEXTURE_ALPHA_SIZE,
	GL_TEXTURE_LUMINANCE_SIZE,
	GL_TEXTURE_INTENSITY_SIZE,
	GL_TEXTURE_DEPTH_SIZE,
	GL_TEXTURE_STENCIL_SIZE,
};

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint tex;
	int i, c;
	const struct sized_internalformat *f;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	for (i = 0; sized_internalformats[i].token != GL_NONE; i++) {
		GLint sizes[CHANNELS];
		bool format_fail = false, format_print = false;
		GLenum format, type;
		f = &sized_internalformats[i];

		if (get_channel_size(f, D) && get_channel_size(f, S)) {
			format = GL_DEPTH_STENCIL;
			type = GL_UNSIGNED_INT_24_8;
		} else if (get_channel_size(f, D)) {
			format = GL_DEPTH_COMPONENT;
			type = GL_FLOAT;
		} else {
			format = GL_RGBA;
			type = GL_FLOAT;

			/* Have to specify integer data for integer textures. */
			for (c = R; c <= I; c++) {
				if (get_channel_type(f, c) == GL_UNSIGNED_INT ||
				    get_channel_type(f, c) == GL_INT) {
					format = GL_RGBA_INTEGER;
					type = GL_UNSIGNED_INT;
					break;
				}
			}
		}

		glTexImage2D(GL_TEXTURE_2D, 0, f->token,
			     1, 1, 0,
			     format, type, NULL);

		if (glGetError() != 0) {
			/* We aren't checking for particular
			 * extensions before trying to create the
			 * texture, so don't complain about formats
			 * producing errors.
			 */
			continue;
		}

		for (c = 0; c < CHANNELS; c++) {
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
						 size_queries[c], &sizes[c]);

			if ((f->bits[c] == NONE) != (sizes[c] == 0))
				format_fail = true;

			if (f->bits[c] != SCMP && f->bits[c] != UCMP) {
				if (get_channel_size(f, c) != sizes[c])
					format_print = true;
			}
		}

		if (format_fail || format_print) {
			printf("format %s%s:\n",
			       f->name, format_fail ? " failure" : "");

			printf("  expected: ");
			for (c = 0; c < CHANNELS; c++) {
				if (sizes[c] == ~0)
					printf("?? ");
				else
					printf("%2d ", get_channel_size(f, c));
			}
			printf("\n");

			printf("  observed: ");
			for (c = 0; c < CHANNELS; c++) {
				printf("%2d ", sizes[c]);
			}
			printf("\n");

			if (format_fail)
				pass = false;
		}
	}

	glDeleteTextures(1, &tex);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
