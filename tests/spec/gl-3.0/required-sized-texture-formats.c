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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "piglit-util-gl.h"
#include "sized-internalformats.h"

/**
 * @file required-sized-formats.c
 *
 * Tests that the required sized internal formats for GL 3.0 are
 * exposed.  See page 180 of the GL 3.0 pdf (20080923), "Required
 * Texture Formats".  Notably:
 *
 *     "In addition, implementations are required to support the
 *      following sized internal formats. Requesting one of these
 *      internal formats for any texture type will allocate exactly
 *      the internal component sizes and types shown for that format
 *      in tables 3.16- 3.17:"
 *
 * Note that table 3.18, sized internal depth and stencil formats, is
 * excluded.
 *
 * In GL 3.1 this is changed to allow increased precision for the
 * required sized formats.  From page 118 of the GL 3.1 core spec PDF
 * (20090528):
 *
 *     "In addition, implementations are required to support the
 *      following sized and compressed internal formats. Requesting
 *      one of these sized internal formats for any texture type will
 *      allocate at least the internal component sizes, and exactly
 *      the component types shown for that format in tables 3.12-
 *      3.13: "
 */

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

static int target_version;

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

static void
GetTexLevelParameteriv(GLenum target, GLuint level, GLenum pname, GLint *value)
{
	const bool compat_profile = !piglit_is_gles() &&
		!piglit_is_core_profile;

	if (!compat_profile) {
		switch (pname) {
		case GL_TEXTURE_LUMINANCE_SIZE:
		case GL_TEXTURE_INTENSITY_SIZE:
		case GL_TEXTURE_LUMINANCE_TYPE:
		case GL_TEXTURE_INTENSITY_TYPE:
			*value = 0;
			return;
		default:
			break;
		}
	}

	glGetTexLevelParameteriv(target, level, pname, value);
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint tex;
	int i, c;

	piglit_require_gl_version(target_version);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	for (i = 0; required_formats[i].token != GL_NONE; i++) {
		GLint sizes[CHANNELS];
		GLint types[CHANNELS];
		bool format_pass = true;
		GLenum format, type;
		const struct sized_internalformat *f;

		if (!valid_for_gl_version(&required_formats[i], target_version))
			continue;

		f = get_sized_internalformat(required_formats[i].token);
		if (!f) {
			printf("Failed to get sized format for %s\n",
			       piglit_get_gl_enum_name(required_formats[i].token));
			pass = false;
			continue;
		}

		if (f->token == GL_DEPTH24_STENCIL8 ||
		    f->token == GL_DEPTH32F_STENCIL8) {
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
			printf("Unexpected error creating %s texture\n",
			       f->name);
			pass = false;
			continue;
		}

		for (c = 0; c < CHANNELS; c++) {
			GetTexLevelParameteriv(GL_TEXTURE_2D, 0,
					       size_queries[c], &sizes[c]);

			if (c != S) {
				GetTexLevelParameteriv(GL_TEXTURE_2D, 0,
						       type_queries[c],
						       &types[c]);
			} else {
				/* For stencil, there's no query for
				 * the type, so our table above
				 * records a type/size of unorm 8, and
				 * we'll just set the query result
				 * here to unorm so that we only look
				 * at the size.
				 */
				if (sizes[c] != 0)
					types[c] = GL_UNSIGNED_NORMALIZED;
				else
					types[c] = GL_NONE;
			}

			/* We use ~0 as the signal for the compressed
			 * texture formats.  While the colors being
			 * interpolated across the 4x4 blocks have 8
			 * bits in them, the spec suggests reporting
			 * some approximate value less than that.
			 * From page 319 of the GL 3.0 spec:
			 *
			 *     "Queries of value of TEXTURE RED SIZE,
			 *      TEXTURE GREEN SIZE, [...] return the
			 *      actual resolutions of the stored image
			 *      array components, not the resolutions
			 *      specified when the image array was
			 *      defined. For texture images with a
			 *      compressed internal format, the
			 *      resolutions returned specify the
			 *      component resolution of an
			 *      uncompressed internal format that
			 *      produces an image of roughly the same
			 *      quality as the compressed image in
			 *      question. Since the quality of the
			 *      implementation’s compression algorithm
			 *      is likely data-dependent, the returned
			 *      component sizes should be treated only
			 *      as rough approximations.
			 */
			if (f->bits[c] == SCMP ||
			    f->bits[c] == UCMP) {
				if (sizes[c] <= 0 || sizes[c] > 8)
					format_pass = false;
			} else if (target_version == 30) {
				if (sizes[c] != get_channel_size(f, c)) {
					if ((c == D || c == S) &&
					    get_channel_size(f, c) > 0) {
						/* any non-zero size will do */
						if (sizes[c] <= 0)
							format_pass = false;
					} else {
						format_pass = false;
					}
				}
			} else {
				if (sizes[c] < get_channel_size(f, c)) {
					format_pass = false;
				}
			}

			if (types[c] != get_channel_type(f, c))
				format_pass = false;
		}

		if (!format_pass) {
			printf("format %s:\n",
			       f->name);

			printf("  expected: ");
			for (c = 0; c < CHANNELS; c++) {
				print_bits(get_channel_size(f, c),
					   get_channel_type(f, c));
				printf(" ");
			}
			printf("\n");

			printf("  observed: ");
			for (c = 0; c < CHANNELS; c++) {
				print_bits(sizes[c], types[c]);
				printf(" ");
			}
			printf("\n");

			pass = false;
		}
	}

	glDeleteTextures(1, &tex);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

PIGLIT_GL_TEST_CONFIG_BEGIN
	setup_required_size_test(argc, argv, &config);
	target_version = MAX2(config.supports_gl_compat_version,
			      config.supports_gl_core_version);
PIGLIT_GL_TEST_CONFIG_END
