/*
 * Copyright © 2011 Intel Corporation
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
 * \file internal-format-query.c
 * Check that only general-purpose fmts are listed by COMPRESSED_TEXTURE_FORMATS
 *
 * From page 117 (page 129 of the PDF) of the OpenGL 1.3 spec says:
 *
 *     "The set of specific compressed internal formats supported by the
 *     renderer can be obtained by querying the value of COMPRESSED TEXTURE
 *     FORMATS. The only values returned by this query are those corresponding
 *     to formats suitable for general-purpose usage. The renderer will not
 *     enumerate formats with restrictions that need to be specifically
 *     understood prior to use."
 *
 * All texture compression extensions have taken this to mean only linear RGB
 * and linear RGBA formats should be exposed.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 10;
	config.window_height = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

#define ENUM_AND_STRING(e) \
	# e, e

struct format_tuple {
	const char *name;
	GLenum format;
};

/**
 * Set of formats for a particular extension
 *
 * Each array of "good" formats and "bad" formats is terminated by a sentinel
 * with \c format set to zero.
 *
 * \note
 * The arrays are sized for the largest set from any extension.  They may need
 * to be expanded in the future.
 */
struct format_list {
	/**
	 * Formats that are part of the extension and should be exposed.
	 */
	struct format_tuple good[11];

	/**
	 * Formats that are part of the extension but should not be exposed.
	 */
	struct format_tuple bad[5];
};

/**
 * Formats belonging to GL_ARB_texture_comrpession_bptc
 *
 * The extension spec says nothing about whether or not these must be
 * advertised via GL_COMPRESSED_TEXTURE_FORMATS.  The OpenGL 4.2 spec also
 * requires these formats, but it says that GL_NUM_COMPRESSED_TEXTURE_FORMATS
 * must be at least 0.  NVIDIA's driver does not expose them, so we'll
 * classify them as optional.
 */
static const struct format_list bptc_formats = {
	{
		{ ENUM_AND_STRING(GL_COMPRESSED_RGBA_BPTC_UNORM_ARB) },
		{ ENUM_AND_STRING(GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB) },
		{ ENUM_AND_STRING(GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB) },
		{ ENUM_AND_STRING(GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB) },
		{ NULL, 0 },
	},
	{
		{ NULL, 0 },
	}
};

/**
 * Formats belonging to GL_ARB_texture_comrpession_rgtc
 */
static const struct format_list rgtc_formats = {
	{
		{ NULL, 0 },
	},
	{
		{ ENUM_AND_STRING(GL_COMPRESSED_RED_RGTC1) },
		{ ENUM_AND_STRING(GL_COMPRESSED_SIGNED_RED_RGTC1) },
		{ ENUM_AND_STRING(GL_COMPRESSED_RG_RGTC2) },
		{ ENUM_AND_STRING(GL_COMPRESSED_SIGNED_RG_RGTC2) },
		{ NULL, 0 },
	}
};

/**
 * Formats belonging to GL_3DFX_texture_comrpession_FXT1
 */
static const struct format_list fxt1_formats = {
	{
		{ ENUM_AND_STRING(GL_COMPRESSED_RGB_FXT1_3DFX) },
		{ ENUM_AND_STRING(GL_COMPRESSED_RGBA_FXT1_3DFX) },
		{ NULL, 0 },
	},
	{
		{ NULL, 0 },
	}
};

/**
 * Formats belonging to GL_ATI_texture_comrpession_3dc
 */
static const struct format_list ati_3dc_formats = {
	{
		{ NULL, 0 },
	},
	{
		{ "GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI", 0x8837 },
		{ NULL, 0 },
	}
};

/**
 * Formats belonging to GL_EXT_texture_comrpession_latc
 */
static const struct format_list latc_formats = {
	{
		{ NULL, 0 },
	},
	{
		{ ENUM_AND_STRING(GL_COMPRESSED_LUMINANCE_LATC1_EXT) },
		{ ENUM_AND_STRING(GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT) },
		{ ENUM_AND_STRING(GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT) },
		{ ENUM_AND_STRING(GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT) },
		{ NULL, 0 },
	}
};

/**
 * Formats belonging to GL_EXT_texture_comrpession_s3tc
 */
static const struct format_list s3tc_formats = {
	{
		{ ENUM_AND_STRING(GL_COMPRESSED_RGB_S3TC_DXT1_EXT) },
		{ ENUM_AND_STRING(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT) },
		{ ENUM_AND_STRING(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT) },
		{ NULL, 0 },
	},
	{
		{ ENUM_AND_STRING(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) },
		{ NULL, 0 },
	}
};

/**
 * Formats belonging to GL_EXT_texture_sRGB
 *
 * These should only be exported if GL_EXT_texture_compression_s3tc is also
 * supported.
 */
static const struct format_list srgb_formats = {
	{
		{ NULL, 0 },
	},
	{
		{ ENUM_AND_STRING(GL_COMPRESSED_SRGB_S3TC_DXT1_EXT) },
		{ ENUM_AND_STRING(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT) },
		{ ENUM_AND_STRING(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT) },
		{ ENUM_AND_STRING(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT) },
		{ NULL, 0 },
	}
};

/**
 * Formats belonging to GL_OES_comrpessed_paletted_texture
 */
static const struct format_list paletted_formats = {
	{
		{ ENUM_AND_STRING(GL_PALETTE4_RGB8_OES) },
		{ ENUM_AND_STRING(GL_PALETTE4_RGBA8_OES) },
		{ ENUM_AND_STRING(GL_PALETTE4_R5_G6_B5_OES) },
		{ ENUM_AND_STRING(GL_PALETTE4_RGBA4_OES) },
		{ ENUM_AND_STRING(GL_PALETTE4_RGB5_A1_OES) },
		{ ENUM_AND_STRING(GL_PALETTE8_RGB8_OES) },
		{ ENUM_AND_STRING(GL_PALETTE8_RGBA8_OES) },
		{ ENUM_AND_STRING(GL_PALETTE8_R5_G6_B5_OES) },
		{ ENUM_AND_STRING(GL_PALETTE8_RGBA4_OES) },
		{ ENUM_AND_STRING(GL_PALETTE8_RGB5_A1_OES) },
		{ NULL, 0 },
	},
	{
		{ NULL, 0 },
	}
};

/**
 * Format belonging to GL_OES_compressed_ETC1_RGB8_texture.
 *
 * The GL_OES_compressed_ETC1_RGB8_texture spec says:
 *
 *     "New State
 *
 *         The queries for NUM_COMPRESSED_TEXTURE_FORMATS and
 *         COMPRESSED_TEXTURE_FORMATS include ETC1_RGB8_OES."
 */
static const struct format_list etc1_formats = {
	{
		{ ENUM_AND_STRING(GL_ETC1_RGB8_OES) },
		{ NULL, 0 },
	},
	{
		{ NULL, 0 },
	}
};

/**
 * Formats belonging to OpenGL ES 3.0
 *
 * These formats are dragged into desktop OpenGL via GL_ARB_ES3_compatibility
 * or OpenGL 4.3.  The extension spec says nothing about whether or not these
 * must be advertised via GL_COMPRESSED_TEXTURE_FORMATS.  The OpenGL 4.3 spec
 * requires these formats, but it says that GL_NUM_COMPRESSED_TEXTURE_FORMATS
 * must be at least 0.  NVIDIA's driver exposes them, so we'll classify them
 * as optional.
 */
static const struct format_list etc2_formats = {
	{
		{ ENUM_AND_STRING(GL_COMPRESSED_RGB8_ETC2) },
		{ ENUM_AND_STRING(GL_COMPRESSED_SRGB8_ETC2) },
		{ ENUM_AND_STRING(GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2) },
		{ ENUM_AND_STRING(GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2) },
		{ ENUM_AND_STRING(GL_COMPRESSED_RGBA8_ETC2_EAC) },
		{ ENUM_AND_STRING(GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC) },
		{ ENUM_AND_STRING(GL_COMPRESSED_R11_EAC) },
		{ ENUM_AND_STRING(GL_COMPRESSED_SIGNED_R11_EAC) },
		{ ENUM_AND_STRING(GL_COMPRESSED_RG11_EAC) },
		{ ENUM_AND_STRING(GL_COMPRESSED_SIGNED_RG11_EAC) },
		{ NULL, 0 },
	},
	{
		{ NULL, 0 },
	}
};

/**
 * List of all known compression methods to test
 *
 * The dummy first element is because this list is used by \c main to replace
 * the \c argv vector when no command line parameters are supplied.
 */
const char *all_formats[] = {
	"argv[0]",
	"bptc",
	"s3tc",
	"fxt1",
	"latc",
	"3dc",
	"rgtc",
	"srgb",
	"paletted",
	"etc1",
	"etc2",
};

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static bool
reject_bad(const struct format_tuple *bad, GLenum *compressed_formats,
	   GLint num_compressed_formats, bool check_errors)
{
	bool pass = true;
	unsigned i;
	unsigned j;

	for (i = 0; bad[i].format != 0; i++) {
		for (j = 0; j < num_compressed_formats; j++) {
			if (compressed_formats[j] == 0)
				continue;

			if (compressed_formats[j] == bad[i].format)
				break;
		}

		if (j != num_compressed_formats) {
			if (check_errors) {
				fprintf(stderr,
					"%s should not be available.\n",
					bad[i].name);
				pass = false;
			}

			/* Replace formats that have been processed with
			 * zero.  This allows detection of values that don't
			 * belong to any compression extension.
			 */
			compressed_formats[j] = 0;
		}
	}

	return pass;
}

static bool
try_formats(const struct format_list *t, GLenum *compressed_formats,
	    GLint num_compressed_formats, bool check_errors, bool supported,
	    bool optional)
{
	bool pass = true;

	if (!supported) {
		pass = reject_bad(t->good, compressed_formats,
				  num_compressed_formats, check_errors)
			&& pass;
	} else {
		unsigned i;
		unsigned j;

		for (i = 0; t->good[i].format != 0; i++) {
			for (j = 0; j < num_compressed_formats; j++) {
				if (compressed_formats[j] == 0)
					continue;

				if (compressed_formats[j] == t->good[i].format)
					break;
			}

			if (j == num_compressed_formats) {
				if (check_errors && !optional) {
					fprintf(stderr,
						"%s should be available.\n",
						t->good[i].name);
					pass = false;
				}
			} else {
				/* Replace formats that have been processed
				 * with zero.  This allows detection of values
				 * that don't belong to any compression
				 * extension.
				 */
				compressed_formats[j] = 0;
			}
		}
	}

	pass = reject_bad(t->bad, compressed_formats, num_compressed_formats,
			  check_errors)
		&& pass;

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	GLint num_compressed_formats;
	GLenum *compressed_formats = NULL;
	unsigned i;
	bool pass = true;
	bool log_header = true;
	bool do_all = false;
	bool check_errors = true;

	piglit_require_extension("GL_ARB_texture_compression");

	glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS,
		      &num_compressed_formats);
	if (num_compressed_formats == 0) {
		printf("No compressed formats supported.\n");
	} else {
		compressed_formats = calloc(num_compressed_formats,
					    sizeof(GLenum));
		glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS,
			      (GLint *) compressed_formats);

		printf("Driver reported the following compressed formats:\n");
		for (i = 0; i < num_compressed_formats; i++) {
			printf("    0x%04x: %s\n",
			       compressed_formats[i],
			       piglit_get_gl_enum_name(compressed_formats[i]));
		}
		printf("\n");
		fflush(stdout);
	}

	/* First scan the list of formats looking for zeros.  We use that as
	 * magic flag later in the test.
	 */
	for (i = 0; i < num_compressed_formats; i++) {
		if (compressed_formats[i] == 0) {
			fprintf(stderr,
				"Invalid value 0x0000 in format list.\n");
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	/* The "unknown" flag instructs the test to just check for values that
	 * don't belong to any compression extension supported by this
	 * implementation.
	 */
	if (argc > 1 && strcmp("unknown", argv[1]) == 0) {
		check_errors = false;
		argv++;
		argc--;
	}

	if (argc == 1) {
		argv = (char **) all_formats;  /* cast away const */
		argc = ARRAY_SIZE(all_formats);
		do_all = true;
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "bptc") == 0) {
			pass = try_formats(&bptc_formats,
					   compressed_formats,
					   num_compressed_formats,
					   check_errors,
					   piglit_is_extension_supported("GL_ARB_texture_compression_bptc"),
					   true)
				&& pass;
		} else if (strcmp(argv[i], "s3tc") == 0) {
			pass = try_formats(&s3tc_formats,
					   compressed_formats,
					   num_compressed_formats,
					   check_errors,
					   piglit_is_extension_supported("GL_EXT_texture_compression_s3tc"),
					   false)
				&& pass;
		} else if (strcmp(argv[i], "fxt1") == 0) {
			pass = try_formats(&fxt1_formats,
					   compressed_formats,
					   num_compressed_formats,
					   check_errors,
					   piglit_is_extension_supported("GL_3DFX_texture_compression_FXT1"),
					   false)
				&& pass;
		} else if (strcmp(argv[i], "latc") == 0) {
			pass = try_formats(&latc_formats,
					   compressed_formats,
					   num_compressed_formats,
					   check_errors,
					   piglit_is_extension_supported("GL_EXT_texture_compression_latc"),
					   false)
				&& pass;
		} else if (strcmp(argv[i], "3dc") == 0) {
			pass = try_formats(&ati_3dc_formats,
					   compressed_formats,
					   num_compressed_formats,
					   check_errors,
					   piglit_is_extension_supported("GL_ATI_texture_compression_3dc"),
					   false)
				&& pass;
		} else if (strcmp(argv[i], "rgtc") == 0) {
			pass = try_formats(&rgtc_formats,
					   compressed_formats,
					   num_compressed_formats,
					   check_errors,
					   (piglit_is_extension_supported("GL_ARB_texture_compression_rgtc")
					    || piglit_is_extension_supported("GL_EXT_texture_compression_rgtc")),
					   false)
				&& pass;
		} else if (strcmp(argv[i], "srgb") == 0) {
			pass = try_formats(&srgb_formats,
					   compressed_formats,
					   num_compressed_formats,
					   check_errors,
					   (piglit_is_extension_supported("GL_EXT_texture_sRGB")
					    && piglit_is_extension_supported("GL_EXT_texture_compression_s3tc")),
					   false)
				&& pass;
		} else if (strcmp(argv[i], "paletted") == 0) {
			pass = try_formats(&paletted_formats,
					   compressed_formats,
					   num_compressed_formats,
					   check_errors,
					   piglit_is_extension_supported("GL_OES_compressed_paletted_texture"),
					   false)
				&& pass;
		} else if (strcmp(argv[i], "etc1") == 0) {
			pass = try_formats(&etc1_formats,
					   compressed_formats,
					   num_compressed_formats,
					   check_errors,
					   piglit_is_extension_supported("GL_OES_compressed_ETC1_RGB8_texture"),
					   false)
				&& pass;
		} else if (strcmp(argv[i], "etc2") == 0) {
			pass = try_formats(&etc2_formats,
					   compressed_formats,
					   num_compressed_formats,
					   check_errors,
					   piglit_is_extension_supported("GL_ARB_ES3_compatibility"),
					   true)
				&& pass;
		} else {
			fprintf(stderr,
				"Unrecognized selection `%s'\n", argv[i]);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	/* After all of the known formats have been processed, the entire
	 * format array should be zeroed out.  Any non-zero values are either
	 * errors or formats from unknown extensions... meaning that the test
	 * may need to be updated.
	 */
	if (do_all) {
		for (i = 0; i < num_compressed_formats; i++) {
			if (compressed_formats[i] != 0) {
				if (log_header) {
					fprintf(stderr,
						"Unrecognized compressed "
						"texture formats:\n");
					log_header = false;
				}

				fprintf(stderr, "    0x%04x: %s\n",
					compressed_formats[i],
					piglit_get_gl_enum_name(compressed_formats[i]));
				pass = false;
			}
		}
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
