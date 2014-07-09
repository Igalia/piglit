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

/**
 * \file api-errors.c
 * Verify a handful of error conditions required by the spec.
 *
 * None of these subtests is large enough to warrant a separate test case.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const GLenum valid_targets[] = {
	GL_RENDERBUFFER,
};

static const GLenum invalid_targets[] = {
	GL_FRAMEBUFFER,
	GL_COLOR_ATTACHMENT0,
	GL_COLOR_ATTACHMENT1,
	GL_COLOR_ATTACHMENT2,
	GL_COLOR_ATTACHMENT3,
	GL_COLOR_ATTACHMENT4,
	GL_COLOR_ATTACHMENT5,
	GL_COLOR_ATTACHMENT6,
	GL_COLOR_ATTACHMENT7,
	GL_COLOR_ATTACHMENT8,
	GL_COLOR_ATTACHMENT9,
	GL_COLOR_ATTACHMENT10,
	GL_COLOR_ATTACHMENT11,
	GL_COLOR_ATTACHMENT12,
	GL_COLOR_ATTACHMENT13,
	GL_COLOR_ATTACHMENT14,
	GL_COLOR_ATTACHMENT15,
	GL_DEPTH_ATTACHMENT,
	GL_STENCIL_ATTACHMENT,
	GL_TEXTURE_4D_SGIS,
	GL_TEXTURE_RENDERBUFFER_NV,
};

static const GLenum invalid_targets_without_query2[] = {
	GL_TEXTURE_1D,
	GL_TEXTURE_1D_ARRAY,
	GL_TEXTURE_2D,
	GL_TEXTURE_2D_ARRAY,
	GL_TEXTURE_3D,
	GL_TEXTURE_CUBE_MAP,
	GL_TEXTURE_CUBE_MAP_ARRAY,
	GL_TEXTURE_RECTANGLE,
	GL_TEXTURE_BUFFER,
};

static const GLenum invalid_targets_without_tms[] = {
	GL_TEXTURE_2D_MULTISAMPLE,
	GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
};

static const GLenum valid_formats[] = {
	GL_RGBA,
};

static const GLenum invalid_formats[] = {
	GL_COMPRESSED_RGB,
	GL_COMPRESSED_RGBA,
	GL_COMPRESSED_ALPHA,
	GL_COMPRESSED_LUMINANCE,
	GL_COMPRESSED_LUMINANCE_ALPHA,
	GL_COMPRESSED_INTENSITY,
	GL_COMPRESSED_SRGB,
	GL_COMPRESSED_SRGB_ALPHA,
	GL_COMPRESSED_SLUMINANCE,
	GL_COMPRESSED_SLUMINANCE_ALPHA,
	GL_COMPRESSED_RED,
	GL_COMPRESSED_RG,
	GL_COMPRESSED_RED_RGTC1,
	GL_COMPRESSED_SIGNED_RED_RGTC1,
	GL_COMPRESSED_RG_RGTC2,
	GL_COMPRESSED_SIGNED_RG_RGTC2,
	GL_COMPRESSED_RGBA_BPTC_UNORM_ARB,
	GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB,
	GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB,
	GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB,
	GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
	GL_COMPRESSED_RGB_FXT1_3DFX,
	GL_COMPRESSED_RGBA_FXT1_3DFX,
	GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,
	GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,
	GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,
	GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,
	GL_COMPRESSED_LUMINANCE_LATC1_EXT,
	GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT,
	GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT,
	GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT,
	GL_YCBCR_MESA,
	GL_GREEN_INTEGER_EXT,
	GL_BLUE_INTEGER_EXT,
	GL_ALPHA_INTEGER_EXT,
	GL_RGB_INTEGER_EXT,
	GL_RGBA_INTEGER_EXT,
	GL_BGR_INTEGER_EXT,
	GL_BGRA_INTEGER_EXT,
	GL_LUMINANCE_INTEGER_EXT,
	GL_LUMINANCE_ALPHA_INTEGER_EXT,
	GL_RGB9_E5
};

static const GLenum valid_pnames[] = {
	GL_SAMPLES,
	GL_NUM_SAMPLE_COUNTS,
};

static const GLenum invalid_pnames[] = {
	GL_RED_BITS,
	GL_GREEN_BITS,
	GL_BLUE_BITS,
	GL_ALPHA_BITS,
	GL_DEPTH_BITS,
	GL_STENCIL_BITS,
	GL_MAX_3D_TEXTURE_SIZE,
	GL_MAX_CUBE_MAP_TEXTURE_SIZE,
	GL_TEXTURE_INTERNAL_FORMAT,
	GL_TEXTURE_WIDTH,
	GL_TEXTURE_HEIGHT,
	GL_TEXTURE_COMPONENTS,
};

static const GLenum invalid_pnames_without_query2[] = {
	GL_INTERNALFORMAT_SUPPORTED,
	GL_INTERNALFORMAT_PREFERRED,
	GL_INTERNALFORMAT_RED_SIZE,
	GL_INTERNALFORMAT_GREEN_SIZE,
	GL_INTERNALFORMAT_BLUE_SIZE,
	GL_INTERNALFORMAT_ALPHA_SIZE,
	GL_INTERNALFORMAT_DEPTH_SIZE,
	GL_INTERNALFORMAT_STENCIL_SIZE,
	GL_INTERNALFORMAT_SHARED_SIZE,
	GL_INTERNALFORMAT_RED_TYPE,
	GL_INTERNALFORMAT_GREEN_TYPE,
	GL_INTERNALFORMAT_BLUE_TYPE,
	GL_INTERNALFORMAT_ALPHA_TYPE,
	GL_INTERNALFORMAT_DEPTH_TYPE,
	GL_INTERNALFORMAT_STENCIL_TYPE,
	GL_MAX_WIDTH,
	GL_MAX_HEIGHT,
	GL_MAX_DEPTH,
	GL_MAX_LAYERS,
	GL_MAX_COMBINED_DIMENSIONS,
	GL_COLOR_COMPONENTS,
	GL_DEPTH_COMPONENTS,
	GL_STENCIL_COMPONENTS,
	GL_COLOR_RENDERABLE,
	GL_DEPTH_RENDERABLE,
	GL_STENCIL_RENDERABLE,
	GL_FRAMEBUFFER_RENDERABLE,
	GL_FRAMEBUFFER_RENDERABLE_LAYERED,
	GL_FRAMEBUFFER_BLEND,
	GL_READ_PIXELS,
	GL_READ_PIXELS_FORMAT,
	GL_READ_PIXELS_TYPE,
	GL_TEXTURE_IMAGE_FORMAT,
	GL_TEXTURE_IMAGE_TYPE,
	GL_GET_TEXTURE_IMAGE_FORMAT,
	GL_GET_TEXTURE_IMAGE_TYPE,
	GL_MIPMAP,
	GL_MANUAL_GENERATE_MIPMAP,
	GL_AUTO_GENERATE_MIPMAP,
	GL_COLOR_ENCODING,
	GL_SRGB_READ,
	GL_SRGB_WRITE,
	GL_SRGB_DECODE_ARB,
	GL_FILTER,
	GL_VERTEX_TEXTURE,
	GL_TESS_CONTROL_TEXTURE,
	GL_TESS_EVALUATION_TEXTURE,
	GL_GEOMETRY_TEXTURE,
	GL_FRAGMENT_TEXTURE,
	GL_COMPUTE_TEXTURE,
	GL_TEXTURE_SHADOW,
	GL_TEXTURE_GATHER,
	GL_TEXTURE_GATHER_SHADOW,
	GL_SHADER_IMAGE_LOAD,
	GL_SHADER_IMAGE_STORE,
	GL_SHADER_IMAGE_ATOMIC,
	GL_IMAGE_TEXEL_SIZE,
	GL_IMAGE_COMPATIBILITY_CLASS,
	GL_IMAGE_PIXEL_FORMAT,
	GL_IMAGE_PIXEL_TYPE,
	GL_IMAGE_FORMAT_COMPATIBILITY_TYPE,
	GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_TEST,
	GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_TEST,
	GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_WRITE,
	GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_WRITE,
	GL_TEXTURE_COMPRESSED,
	GL_TEXTURE_COMPRESSED_BLOCK_WIDTH,
	GL_TEXTURE_COMPRESSED_BLOCK_HEIGHT,
	GL_TEXTURE_COMPRESSED_BLOCK_SIZE,
	GL_CLEAR_BUFFER,
	GL_TEXTURE_VIEW,
	GL_VIEW_COMPATIBILITY_CLASS,
};

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static bool
try(const GLenum *targets, unsigned num_targets,
    const GLenum *formats, unsigned num_formats,
    const GLenum *pnames, unsigned num_pnames,
    GLenum expected_error)
{
	GLint params[64];
	bool pass = true;
	unsigned i;
	unsigned j;
	unsigned k;

	for (i = 0; i < num_targets; i++) {
		for (j = 0; j < num_formats; j++) {
			for (k = 0; k < num_pnames; k++) {
				bool this_test;
				glGetInternalformativ(targets[i],
						      formats[j],
						      pnames[k],
						      ARRAY_SIZE(params),
						      params);
				this_test =
					piglit_check_gl_error(expected_error);

				if (this_test)
					continue;

				fprintf(stderr,
					"    Failing case was "
					"target = %s, format = %s, "
					"pname = %s\n",
					piglit_get_gl_enum_name(targets[i]),
					piglit_get_gl_enum_name(formats[j]),
					piglit_get_gl_enum_name(pnames[k]));

				pass = false;
			}
		}
	}

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_internalformat_query");

	/* The GL_ARB_internalformat_query spec says:
	 *
	 *     "If the <internalformat> parameter to GetInternalformativ is
	 *     not color-, depth- or stencil-renderable, then an INVALID_ENUM
	 *     error is generated."
	 */
	pass = try(valid_targets, ARRAY_SIZE(valid_targets),
		   invalid_formats, ARRAY_SIZE(invalid_formats),
		   valid_pnames, ARRAY_SIZE(valid_pnames),
		   GL_INVALID_ENUM)
		&& pass;

	/* The GL_ARB_internalformat_query spec says:
	 *
	 *     "If the <target> parameter to GetInternalformativ is not one of
	 *     TEXTURE_2D_MULTISAMPLE, TEXTURE_2D_MULTISAMPLE_ARRAY or
	 *     RENDERBUFFER then an INVALID_ENUM error is generated."
	 *
	 * It also says:
	 *
	 *     "If OpenGL 3.2 or ARB_texture_multisample is not supported,
	 *     then TEXTURE_2D_MULTISAMPLE and TEXTURE_2D_MULTISAMPLE_ARRAY
	 *     are not supported <target> parameters to GetInternalformativ."
	 *
	 * However, GL_ARB_internalformat_query2 adds GL_TEXTURE_1D,
	 * GL_TEXTURE_1D_ARRAY, GL_TEXTURE_2D, GL_TEXTURE_2D_ARRAY,
	 * GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_ARRAY,
	 * GL_TEXTURE_RECTANGLE, and GL_TEXTURE_BUFFER to the list of
	 * available targets.
	 *
	 */
	if (!piglit_is_extension_supported("GL_ARB_internalformat_query2")) {
		pass = try(invalid_targets_without_query2,
			   ARRAY_SIZE(invalid_targets_without_query2),
			   valid_formats, ARRAY_SIZE(valid_formats),
			   valid_pnames, ARRAY_SIZE(valid_pnames),
			   GL_INVALID_ENUM)
			&& pass;
	}

	if (!piglit_is_extension_supported("GL_ARB_texture_multisample")) {
		pass = try(invalid_targets_without_tms,
			   ARRAY_SIZE(invalid_targets_without_tms),
			   valid_formats, ARRAY_SIZE(valid_formats),
			   valid_pnames, ARRAY_SIZE(valid_pnames),
			   GL_INVALID_ENUM)
			&& pass;
	}

	pass = try(invalid_targets, ARRAY_SIZE(invalid_targets),
		   valid_formats, ARRAY_SIZE(valid_formats),
		   valid_pnames, ARRAY_SIZE(valid_pnames),
		   GL_INVALID_ENUM)
		&& pass;

	/* The GL_ARB_internalformat_query spec says:
	 *
	 *     "If the <pname> parameter to GetInternalformativ is not SAMPLES
	 *     or NUM_SAMPLE_COUNTS, then an INVALID_ENUM error is generated."
	 *
	 * However, GL_ARB_internalformat_query2 adds a giant pile of possible
	 * enums to this list.
	 */
	if (!piglit_is_extension_supported("GL_ARB_internalformat_query2")) {
		pass = try(valid_targets, ARRAY_SIZE(valid_targets),
			   valid_formats, ARRAY_SIZE(valid_formats),
			   invalid_pnames_without_query2,
			   ARRAY_SIZE(invalid_pnames_without_query2),
			   GL_INVALID_ENUM)
			&& pass;
	}

	pass = try(valid_targets, ARRAY_SIZE(valid_targets),
		   valid_formats, ARRAY_SIZE(valid_formats),
		   invalid_pnames, ARRAY_SIZE(invalid_pnames),
		   GL_INVALID_ENUM)
		&& pass;

	/* The GL_ARB_internalformat_query spec says:
	 *
	 *     "If the <bufSize> parameter to GetInternalformativ is negative,
	 *     then an INVALID_VALUE error is generated."
	 */
	glGetInternalformativ(valid_targets[0],
			      valid_formats[0],
			      valid_pnames[0],
			      -1, NULL);
	pass = piglit_check_gl_error(GL_INVALID_VALUE)
		&& pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
