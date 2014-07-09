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
 * Verify behavior of TEXTURE_INTERNAL_FORMAT for generic compression formats
 *
 * Typically the \c GL_TEXTURE_INTERNAL_FORMAT query returns the internal
 * format specified by the application at \c glTexImage2D time.  This behavior
 * is modified for the generic compressed texture internal formats.
 * Specifically, the issues section of the GL_ARB_texture_compression spec
 * says:
 *
 *    "(10) Should functionality be provided to allow applications to save
 *    compressed images to disk and reuse them in subsequent runs without
 *    programming to specific formats?  If so, how?
 *
 *      RESOLVED:  Yes.  This can be done without knowledge of specific
 *      compression formats in the following manner:
 *
 *        * Call TexImage with an uncompressed image and a generic compressed
 *          internal format.  The texture image will be compressed by the GL, if
 *          possible.
 *
 *        * Call GetTexLevelParameteriv with a <value> of TEXTURE_COMPRESSED_ARB
 *          to determine if the GL was able to store the image in compressed
 *          form.
 *
 *        * Call GetTexLevelParameteriv with a <value> of
 *          TEXTURE_INTERNAL_FORMAT to determine the specific compressed image
 *          format in which the image is stored.
 *
 *        ..."
 *
 * The body of the spec (section 3.8.1, Texture Image Specification) also say:
 *
 *     "Generic compressed internal formats are never used directly as the
 *     internal formats of texture images.  If <internalformat> is one of the
 *     six generic compressed internal formats, its value is replaced by the
 *     symbolic constant for a specific compressed internal format of the GL's
 *     choosing with the same base internal format.  If no specific compressed
 *     format is available, <internalformat> is instead replaced by the
 *     corresponding base internal format.  If <internalformat> is given as or
 *     mapped to a specific compressed internal format, but the GL can not
 *     support images compressed in the chosen internal format for any reason
 *     (e.g., the compression format might not support 3D textures or borders),
 *     <internalformat> is replaced by the corresponding base internal format
 *     and the texture image will not be compressed by the GL."
 */
#include <stdarg.h>
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 10;
	config.window_height = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

#define ENUM_AND_STRING(e) \
	# e, e

struct test_vector {
	const char *generic_compressed_format_string;
	GLenum generic_compressed_format;
	GLenum base_format;
};

/**
 * Generic texture formats in OpenGL 1.3 and GL_ARB_texture_compression.
 */
static const struct test_vector arb_texture_compression_formats[] = {
        { ENUM_AND_STRING(GL_COMPRESSED_ALPHA), GL_ALPHA },
        { ENUM_AND_STRING(GL_COMPRESSED_LUMINANCE), GL_LUMINANCE },
        { ENUM_AND_STRING(GL_COMPRESSED_LUMINANCE_ALPHA), GL_LUMINANCE_ALPHA },
        { ENUM_AND_STRING(GL_COMPRESSED_INTENSITY), GL_INTENSITY },
        { ENUM_AND_STRING(GL_COMPRESSED_RGB), GL_RGB },
        { ENUM_AND_STRING(GL_COMPRESSED_RGBA), GL_RGBA },
};

/**
 * Generic texture formats in OpenGL 3.0 and GL_ARB_texture_rg.
 */
static const struct test_vector arb_texture_rg_formats[] = {
        { ENUM_AND_STRING(GL_COMPRESSED_RED), GL_RED },
        { ENUM_AND_STRING(GL_COMPRESSED_RG), GL_RG },
};

/**
 * Generic texture formats in OpenGL 2.1 and GL_EXT_texture_sRGB.
 */
static const struct test_vector ext_texture_srgb_formats[] = {
	{ ENUM_AND_STRING(GL_COMPRESSED_SRGB_EXT), GL_RGB },
	{ ENUM_AND_STRING(GL_COMPRESSED_SRGB_ALPHA_EXT), GL_RGBA },
	{ ENUM_AND_STRING(GL_COMPRESSED_SLUMINANCE_EXT), GL_LUMINANCE },
	{ ENUM_AND_STRING(GL_COMPRESSED_SLUMINANCE_ALPHA_EXT), GL_LUMINANCE_ALPHA },
};

static GLubyte dummy_data[16 * 16 * 4];

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

GLenum *
add_formats(GLenum *formats, GLint *total, unsigned add, ...)
{
	va_list ap;
	unsigned i;

	formats = realloc(formats, sizeof(GLenum) * (*total + add));

	va_start(ap, add);

	for (i = 0; i < add; i++) {
		formats[*total] = va_arg(ap, GLenum);
		(*total)++;
	}

	va_end(ap);
	return formats;
}

static bool
try_formats(const struct test_vector *t, unsigned num_tests,
	    const GLenum *compressed_formats, GLint num_compressed_formats)
{
	bool pass = true;
	unsigned i;

	for (i = 0; i < num_tests; i++) {
		GLuint tex;
		GLint is_compressed;
		GLenum format;

		if (!piglit_automatic) {
			printf("Trying %s/0x%04x (base format = 0x%04x)...\n",
			       t[i].generic_compressed_format_string,
			       t[i].generic_compressed_format,
			       t[i].base_format);
		}

		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0,
			     t[i].generic_compressed_format,
			     16, 16, 0,
			     /* Remember that GL_INTESITY is not a valid format
			      * for image data.
			      */
			     (t[i].base_format == GL_INTENSITY)
			     ? GL_RGBA : t[i].base_format,
			     GL_UNSIGNED_BYTE,
			     dummy_data);

		glGetTexLevelParameteriv(GL_TEXTURE_2D,
					 0,
					 GL_TEXTURE_COMPRESSED,
					 &is_compressed);

		glGetTexLevelParameteriv(GL_TEXTURE_2D,
					 0,
					 GL_TEXTURE_INTERNAL_FORMAT,
					 (GLint *) &format);

		if (!piglit_automatic) {
			printf("  is %scompressed, internal format = 0x%04x\n",
			       is_compressed ? "" : "not ",
			       format);
		}

		if (is_compressed) {
			unsigned j;

			for (j = 0; j < num_compressed_formats; j++) {
				if (format == compressed_formats[j])
					break;
			}

			if (format == t[i].generic_compressed_format) {
				fprintf(stderr,
					"%s did compress, but it got the "
					"generic\n"
					"format as the specific internal "
					"format.\n",
					t[i].generic_compressed_format_string);
				pass = false;
			} else if (format <= 4 || format == t[i].base_format) {
				fprintf(stderr,
					"%s did compress, but it got an "
					"internal\n"
					"format 0x%04x that is "
					"non-compressed\n",
					t[i].generic_compressed_format_string,
					format);
				pass = false;
			} else if (j == num_compressed_formats) {
				fprintf(stderr,
					"%s did compress, but it got an "
					"internal\n"
					"format of %s when "
					"one of the supported compressed "
					"formats was expected.\n"
					"This may just mean the test does not "
					"know about the compessed format that\n"
					"was selected by the driver.\n",
					t[i].generic_compressed_format_string,
					piglit_get_gl_enum_name(format));
			}
		} else if (format != t[i].base_format) {
			if (format == t[i].generic_compressed_format) {
				fprintf(stderr,
					"%s did not compress, but it got the "
					"generic\n"
					"format as the specific internal "
					"format.\n",
					t[i].generic_compressed_format_string);
			} else {
				fprintf(stderr,
					"%s did not compress, but it got an "
					"internal format of %s when "
					"%s was expected.\n",
					t[i].generic_compressed_format_string,
					piglit_get_gl_enum_name(format),
					piglit_get_gl_enum_name(t[i].base_format));
			}

			pass = false;
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &tex);

		if (!piglit_automatic) {
			printf("\n");
		}
	}

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	GLint num_compressed_formats;
	GLenum *compressed_formats = NULL;
	unsigned i;
	bool pass = true;

	piglit_require_extension("GL_ARB_texture_compression");

	glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS,
		      &num_compressed_formats);
	if (num_compressed_formats == 0) {
		printf("No compressed formats supported.\n");
	} else if (num_compressed_formats < 0) {
		fprintf(stderr,
			"Invalid number of compressed formats (%d) reported\n",
			num_compressed_formats);
		piglit_report_result(PIGLIT_FAIL);
	} else {
		compressed_formats = calloc(num_compressed_formats,
					    sizeof(GLenum));
		glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS,
			      (GLint *) compressed_formats);

		printf("Driver reported the following compressed formats:\n");
		for (i = 0; i < num_compressed_formats; i++) {
			printf("    0x%04x %s\n",
			       compressed_formats[i],
			       piglit_get_gl_enum_name(compressed_formats[i]));
		}
		printf("\n");
		fflush(stdout);
	}

	/* There are some specific formats that are valid for certain generic
	 * formats that are not returned by the GL_COMRPESSED_TEXTURE_FORMATS
	 * query.  That query only returns formats that have no restrictions or
	 * caveats for RGB or RGBA base formats.  We have to add these formats
	 * to the list of possible formats by hand.
	 */
	if (piglit_is_extension_supported("GL_EXT_texture_compression_latc")) {
		compressed_formats =
			add_formats(compressed_formats,
				    &num_compressed_formats,
				    4,
				    GL_COMPRESSED_LUMINANCE_LATC1_EXT,
				    GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT,
				    GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT,
				    GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT);
	}

	if (piglit_is_extension_supported("GL_ATI_texture_compression_3dc")) {
		compressed_formats =
			add_formats(compressed_formats,
				    &num_compressed_formats,
				    1,
				    0x8837);
	}

	pass = try_formats(arb_texture_compression_formats,
			   ARRAY_SIZE(arb_texture_compression_formats),
			   compressed_formats,
			   num_compressed_formats);

	/* Remove the various luminance and luminance-alpha formats from the
	 * list since they cannot be used for the later tests.
	 */
	if (piglit_is_extension_supported("GL_ATI_texture_compression_3dc")) {
		num_compressed_formats--;
	}

	if (piglit_is_extension_supported("GL_EXT_texture_compression_latc")) {
		num_compressed_formats -= 4;
	}

	/* Add the RGTC formats, then check them.
	 */
	if (piglit_is_extension_supported("GL_ARB_texture_rg")) {
		if (piglit_is_extension_supported("GL_ARB_texture_compression_rgtc")
		    || piglit_is_extension_supported("GL_EXT_texture_compression_rgtc")) {
			compressed_formats =
				add_formats(compressed_formats,
					    &num_compressed_formats,
					    4,
					    GL_COMPRESSED_RED_RGTC1,
					    GL_COMPRESSED_SIGNED_RED_RGTC1,
					    GL_COMPRESSED_RG_RGTC2,
					    GL_COMPRESSED_SIGNED_RG_RGTC2);
		}

		pass = try_formats(arb_texture_rg_formats,
				   ARRAY_SIZE(arb_texture_rg_formats),
				   compressed_formats,
				   num_compressed_formats)
			&& pass;

		/* Remove the RGTC formats from the list since they cannot be
		 * used for the later tests.
		 */
		if (piglit_is_extension_supported("GL_ARB_texture_compression_rgtc")
		    || piglit_is_extension_supported("GL_EXT_texture_compression_rgtc")) {
			num_compressed_formats -= 4;
		}
	}


	/* Add the sRGB formats, then check them.
	 */
	if (piglit_is_extension_supported("GL_EXT_texture_sRGB")) {
		compressed_formats =
			add_formats(compressed_formats,
				    &num_compressed_formats,
				    4,
				    GL_COMPRESSED_SRGB,
				    GL_COMPRESSED_SRGB_ALPHA,
				    GL_COMPRESSED_SLUMINANCE,
				    GL_COMPRESSED_SLUMINANCE_ALPHA);

		if (piglit_is_extension_supported("GL_EXT_texture_compression_s3tc")) {
			compressed_formats =
				add_formats(compressed_formats,
					    &num_compressed_formats,
					    4,
					    GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,
					    GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,
					    GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,
					    GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT);
		}

		pass = try_formats(ext_texture_srgb_formats,
				   ARRAY_SIZE(ext_texture_srgb_formats),
				   compressed_formats,
				   num_compressed_formats)
			&& pass;

		/* Remove the sRGB formats from the list since they cannot be
		 * used for the later tests.
		 */
		num_compressed_formats -= 4;
		if (piglit_is_extension_supported("GL_EXT_texture_compression_s3tc")) {
			num_compressed_formats -= 4;
		}
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
