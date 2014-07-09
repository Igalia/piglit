/*
 * Copyright © 2013 LunarG, Inc.
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
 * Author: Jon Ashburn <jon@lunarg.com>
 */

/**
 *  \file
 * This (arb_texture_view-formats) tests valid and invalid new TextureView
 * formats based on the original textures format.
 *
 * Section 8.18 (Texture Views) of OpenGL 4.3 Core says:
 *     "The two textures’ internal formats must be compatible according to
 *     table 8.21 if the internal format exists in that table. The internal
 *     formats must be identical if not in that table."
 *
 */

#include "piglit-util-gl.h"
#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 15;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "arb_texture_view-formats";

#define MAX_ILLEGAL_FORMATS 17
#define VIEW_CLASS_NOT_IN_TABLE 0xfffffff

/**
 * Iterate through array of texture formats and check if call to TextureView
 * causes the gl error  "err"
 */
static bool
check_format_array(const GLenum err, const unsigned int numFormats,
		   const GLenum *formatArray, const GLenum target,
		   const GLuint tex, const GLuint levels, const
		   GLuint layers)
{
	unsigned int i;
	bool pass = true;

	for (i = 0; i < numFormats; i++) {
		GLenum format;
		GLuint newTex;
		format = formatArray[i];
		if (format == 0)
			continue;
		glGenTextures(1, &newTex);
		glTextureView(newTex, target, tex, format, 0, levels, 0,
			      layers);
		glDeleteTextures(1, &newTex);
		if (!piglit_check_gl_error(err)) {
			printf("failing texView format=%s\n",
			       piglit_get_gl_enum_name(format));
			pass = false;
			break;
		}
	}
	return pass;
}

/**
 * Do error-check tests for texture formats
 */
static bool
test_format_errors(GLenum format_class)
{
	const GLint width = 16, height = 16;
	const GLsizei levels = 5, layers = 6;
	GLenum target = GL_TEXTURE_CUBE_MAP;
	GLuint tex;
	bool pass = true;
	GLenum legalFormats[MAX_ILLEGAL_FORMATS];
	unsigned int numFormats;
	GLenum illegalFormats[] = {
		/* skip compressed sized formats */
		/* 128 bit */
		GL_RGBA32F,
		GL_RGBA32UI,
		GL_RGBA32I,
		/* 96 bit */
		GL_RGB32F,
		GL_RGB32UI,
		GL_RGB32I,
		/* 64 bit */
		GL_RGBA16F,
		GL_RG32F,
		GL_RGBA16UI,
		GL_RG32UI,
		GL_RGBA16I,
		GL_RG32I,
		GL_RGBA16,
		GL_RGBA16_SNORM,
		/* 48 bit */
		GL_RGB16,
		GL_RGB16_SNORM,
		GL_RGB16F,
		GL_RGB16UI,
		GL_RGB16I,
		/* 32 bits */
		GL_RG16F,
		GL_R11F_G11F_B10F,
		GL_R32F,
		GL_RGB10_A2UI,
		GL_RGBA8UI,
		GL_RG16UI,
		GL_R32UI,
		GL_RGBA8I,
		GL_RG16I,
		GL_R32I,
		GL_RGB10_A2,
		GL_RGBA8,
		GL_RG16,
		GL_RGBA8_SNORM,
		GL_RG16_SNORM,
		GL_SRGB8_ALPHA8,
		GL_RGB9_E5,
		/* 24 bits */
		GL_RGB8,
		GL_RGB8_SNORM,
		GL_SRGB8,
		GL_RGB8UI,
		GL_RGB8I,
		/* 16 bits */
		GL_R16F,
		GL_RG8UI,
		GL_R16UI,
		GL_RG8I,
		GL_R16I,
		GL_RG8,
		GL_R16,
		GL_RG8_SNORM,
		GL_R16_SNORM,
		/* 8 bits */
		GL_R8UI,
		GL_R8I,
		GL_R8,
		GL_R8_SNORM,
		/* a sampling of unsized formats */
		GL_ALPHA,
		GL_LUMINANCE,
		GL_LUMINANCE_ALPHA,
		GL_INTENSITY,
		GL_RGB,
		GL_RGBA,
		GL_DEPTH_COMPONENT,
		GL_COMPRESSED_ALPHA,
		GL_COMPRESSED_LUMINANCE_ALPHA,
		GL_COMPRESSED_LUMINANCE,
		GL_COMPRESSED_INTENSITY,
		GL_COMPRESSED_RGB,
		GL_COMPRESSED_RGBA,
		GL_COMPRESSED_RGBA,
		GL_COMPRESSED_SRGB,
		GL_COMPRESSED_SRGB_ALPHA,
		GL_COMPRESSED_SLUMINANCE,
		GL_COMPRESSED_SLUMINANCE_ALPHA,
		/* format that is legal for TexStorage but not in table */
		GL_RGB12
	};

	glGenTextures(1, &tex);   /* orig tex */
	glBindTexture(target, tex);

	switch (format_class) {
	case GL_VIEW_CLASS_128_BITS:
		glTexStorage2D(target, levels, GL_RGBA32F, width, height);
		numFormats = update_valid_arrays(legalFormats, illegalFormats,
				    ARRAY_SIZE(illegalFormats),
				    GL_RGBA32F, GL_RGBA32UI, GL_RGBA32I, 0);
		break;
	case GL_VIEW_CLASS_96_BITS:
		glTexStorage2D(target, levels, GL_RGB32F, width, height);
		numFormats = update_valid_arrays(legalFormats, illegalFormats,
				    ARRAY_SIZE(illegalFormats),
				    GL_RGB32F, GL_RGB32UI, GL_RGB32I, 0);
		break;
	case GL_VIEW_CLASS_64_BITS:
		glTexStorage2D(target, levels, GL_RGBA16F, width, height);
		numFormats = update_valid_arrays(legalFormats, illegalFormats,
				    ARRAY_SIZE(illegalFormats),
				    GL_RGBA16F, GL_RG32F, GL_RGBA16UI,
				    GL_RG32UI, GL_RGBA16I, GL_RG32I, GL_RGBA16,
				    GL_RGBA16_SNORM, 0);
		break;
	case GL_VIEW_CLASS_48_BITS:
		glTexStorage2D(target, levels, GL_RGB16, width, height);
		numFormats = update_valid_arrays(legalFormats, illegalFormats,
				    ARRAY_SIZE(illegalFormats),
				    GL_RGB16, GL_RGB16_SNORM, GL_RGB16F,
				    GL_RGB16UI, GL_RGB16I, 0);
		break;
	case GL_VIEW_CLASS_32_BITS:
		glTexStorage2D(target, levels, GL_RG16F, width, height);
		numFormats = update_valid_arrays(legalFormats, illegalFormats,
				    ARRAY_SIZE(illegalFormats),
				    GL_RG16F, GL_R11F_G11F_B10F, GL_R32F,
				    GL_RGB10_A2UI, GL_RGBA8UI, GL_RG16UI,
				    GL_R32UI, GL_RGBA8I, GL_RG16I,
				    GL_R32I, GL_RGB10_A2, GL_RGBA8, GL_RG16,
				    GL_RGBA8_SNORM, GL_RG16_SNORM,
				    GL_SRGB8_ALPHA8, GL_RGB9_E5, 0);
		break;
	case GL_VIEW_CLASS_24_BITS:
		glTexStorage2D(target, levels, GL_RGB8, width, height);
		numFormats = update_valid_arrays(legalFormats, illegalFormats,
				    ARRAY_SIZE(illegalFormats),
				    GL_RGB8, GL_RGB8_SNORM, GL_SRGB8,
				    GL_RGB8UI, GL_RGB8I, 0);
		break;
	case GL_VIEW_CLASS_16_BITS:
		glTexStorage2D(target, levels, GL_R16F, width, height);
		numFormats = update_valid_arrays(legalFormats, illegalFormats,
				    ARRAY_SIZE(illegalFormats),
				    GL_R16F, GL_RG8UI, GL_R16UI, GL_RG8I,
				    GL_R16I, GL_RG8, GL_R16, GL_RG8_SNORM,
				    GL_R16_SNORM, 0);
		break;
	case GL_VIEW_CLASS_8_BITS:
		glTexStorage2D(target, levels, GL_R8I, width, height);
		numFormats = update_valid_arrays(legalFormats, illegalFormats,
				    ARRAY_SIZE(illegalFormats),
				    GL_R8UI, GL_R8I, GL_R8, GL_R8_SNORM, 0);
		break;
	case VIEW_CLASS_NOT_IN_TABLE:
		glTexStorage2D(target, levels, GL_RGB12, width, height);
		numFormats = update_valid_arrays(legalFormats, illegalFormats,
				    ARRAY_SIZE(illegalFormats),
				    GL_RGB12, 0);
		break;
	default:
	    assert(!"Invalid format_class\n");
	    numFormats = 0;
	}

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("%s Found gl errors prior to testing glTextureView\n",
				   TestName);
		pass = false;
		goto err_out;
	}

	/* ensure TextureView of legal formats gives no gl error */
	pass = check_format_array(GL_NO_ERROR, numFormats, legalFormats,
			   target, tex, levels, layers) && pass;

	/* ensure TextureView  of illegal formats returns an error */
	pass = check_format_array(GL_INVALID_OPERATION,
				  ARRAY_SIZE(illegalFormats), illegalFormats,
				  target, tex, levels, layers) && pass;
err_out:
	glDeleteTextures(1, &tex);

	return pass;
}


enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

#define X(f, desc)					     	\
	do {							\
		const bool subtest_pass = (f);			\
		piglit_report_subtest_result(subtest_pass	\
					     ? PIGLIT_PASS : PIGLIT_FAIL, \
					     (desc));		\
		pass = pass && subtest_pass;			\
	} while (0)

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_ARB_texture_view");
	piglit_require_extension("GL_EXT_texture_integer");
	piglit_require_extension("GL_ARB_texture_float");
	if (piglit_get_gl_version() < 31)
	    piglit_require_extension("GL_ARB_texture_cube_map");

	X(test_format_errors(GL_VIEW_CLASS_128_BITS), "Format 128 bits validity");
	X(test_format_errors(GL_VIEW_CLASS_96_BITS), "Format 96 bits validity");
	X(test_format_errors(GL_VIEW_CLASS_64_BITS), "Format 64 bits validity");
	X(test_format_errors(GL_VIEW_CLASS_48_BITS), "Format 48 bits validity");
	X(test_format_errors(GL_VIEW_CLASS_32_BITS), "Format 32 bits validity");
	X(test_format_errors(GL_VIEW_CLASS_24_BITS), "Format 24 bits validity");
	X(test_format_errors(GL_VIEW_CLASS_16_BITS), "Format 16 bits validity");
	X(test_format_errors(GL_VIEW_CLASS_8_BITS), "Format 8 bits validity");
	X(test_format_errors(VIEW_CLASS_NOT_IN_TABLE), "Format misc validity");
#undef X
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);

}
