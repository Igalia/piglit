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
 * \file depth-stencil-texture.c
 * Create a depth-stencil texture when ARB_depth_texture is not supported.
 *
 * The EXT_packed_depth_stencil spec neglects to mention an interaction
 * (though the header of the spec says "ARB_depth_texture affects the
 * definition of this extension."), but ARB_framebuffer_object, which includes
 * EXT_packed_depth_stencil functionality says:
 *
 *     "If ARB_depth_texture or SGIX_depth_texture is supported,
 *     GL_DEPTH_STENCIL/GL_UNSIGNED_INT_24_8 data can also be used for
 *     textures;"
 *
 * In cases where neither ARB_depth_texture nor SGIX_depth_texture is
 * supported, trying to create a texture with a depth-stencil format should
 * generate an error.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

#if defined PIGLIT_USE_OPENGL
	config.supports_gl_compat_version = 12;
	config.supports_gl_core_version = 31;
#elif defined PIGLIT_USE_OPENGL_ES2
	config.supports_gl_es_version = 20;
#elif defined PIGLIT_USE_OPENGL_ES1
	config.supports_gl_es_version = 11;
#endif

	config.window_visual = PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static bool has_texture_3d = false;
static bool has_texture_cube_map = false;
static bool has_depth_texture_cube_map = false;
static bool has_depth_texture = false;

static bool
check_gl_error2_(GLenum expected_error1, GLenum expected_error2,
			 const char *file, unsigned line)
{
        const GLenum actual_error = glGetError();

        if (actual_error == expected_error1
	    || actual_error == expected_error2) {
                return true;
        }

        /*
         * If the lookup of the error's name is successful, then print
         *     Unexpected GL error: NAME 0xHEX
         * Else, print
         *     Unexpected GL error: 0xHEX
         */
        printf("Unexpected GL error: %s 0x%x\n",
               piglit_get_gl_error_name(actual_error), actual_error);
        printf("(Error at %s:%u)\n", file, line);

	if (expected_error2 != GL_NO_ERROR) {
		printf("Expected GL error: %s 0x%x or %s 0x%x\n",
		       piglit_get_gl_error_name(expected_error1), expected_error1,
		       piglit_get_gl_error_name(expected_error2), expected_error2);
	} else 	if (expected_error1 != GL_NO_ERROR) {
		printf("Expected GL error: %s 0x%x\n",
		       piglit_get_gl_error_name(expected_error1), expected_error1);
	}

        return false;
}

#define check_gl_error2(a, b) check_gl_error2_(a, b, __FILE__, __LINE__)

static bool
try_TexImage(GLenum internalFormat)
{
	bool pass = true;
	GLuint tex[4];

	GLenum expected_error = has_depth_texture
		? GL_NO_ERROR : GL_INVALID_VALUE;
	GLenum alt_error = GL_NO_ERROR;

	GLenum expected_3D_error = has_depth_texture
		? GL_INVALID_OPERATION : GL_INVALID_VALUE;
	GLenum alt_3D_error = GL_NO_ERROR;

	GLenum expected_cube_error = has_depth_texture_cube_map
		? GL_NO_ERROR : expected_3D_error;
	GLenum alt_cube_error = GL_NO_ERROR;

#if !defined PIGLIT_USE_OPENGL
	/* The OpenGL ES rules are non-obvious.
	 *
	 * In OpenGL ES 1.x and 2.x, the internal format and the format must
	 * be the same.  This even applies in OpenGL ES 2.0 when
	 * GL_OES_depth_texture is available.
	 *
	 * Section 3.7.1 (Texture Image Specification) of the OpenGL ES 1.1.12
	 * spec says:
	 *
	 *     "If internalformat does not match format, the error
	 *     INVALID_OPERATION is generated."
	 *
	 * Section 3.7.1 (Texture Image Specification) of the OpenGL ES 2.0.25
	 * spec says the same thing.
	 *
	 * As a result, in OpenGL ES 1.x or OpenGL ES 2.0 without
	 * GL_OES_depth_texture, glTexImage2D(..., GL_DEPTH24_STENCIL8, ...,
	 * GL_DEPTH_STENCIL, FLOAT_32_UNSIGNED_INT_24_8) may generate *either*
	 * GL_INVALID_VALUE or GL_INVALID_OPERATION depending on the order the
	 * implementation checks the errors.
	 *
	 * In OpenGL ES 3.0, the internal format must not be GL_DEPTH_STENCIL.
	 * Section 3.8.3 (Texture Image Specification) of the OpenGL ES 3.0.3
	 * spec says:
	 *
	 *     "Specifying a combination of values for format, type, and
	 *     internalformat that is not listed as a valid combination in
	 *     tables 3.2 or 3.3 generates the error INVALID_OPERATION."
	 *
	 * Table 3.2 contains the lines:
	 *
	 *     Format         Type                     External   Internal
	 *                                             Bytes      Format
	 *                                             Per Pixel
	 *     DEPTH_STENCIL  UNSIGNED_INT_24_8           4       DEPTH24_STENCIL8
	 *     DEPTH_STENCIL  FLOAT_32_UNSIGNED_INT_24_8  8       DEPTH32F_STENCIL8
	 *
	 * The GL_OES_packed_depth_stencil spec still says:
	 *
	 *     "Accepted by the <format> parameter of TexImage2D and
	 *     TexSubImage2D and by the <internalformat> parameter of
	 *     TexImage2D:
	 *
	 *         DEPTH_STENCIL_OES                              0x84F9"
	 *
	 * An OpenGL ES 3.0 implementation that advertises
	 * GL_OES_packed_depth_stencil should accepth both GL_DEPTH_STENCIL
	 * and GL_DEPTH24_STENCIL8 for internalformat.
	 */
	if (has_depth_texture) {
		if ((piglit_get_gl_version() < 30
		     && internalFormat != GL_DEPTH_STENCIL)
		    || (piglit_get_gl_version() >= 30
			&& internalFormat == GL_DEPTH_STENCIL
			&& !piglit_is_extension_supported("GL_OES_packed_depth_stencil"))) {

			expected_error = GL_INVALID_OPERATION;
			alt_error = GL_NO_ERROR;

			/* 3D depth textures are never supported.
			 * GL_INVALID_OPERATION is expected.  That
			 * error is already expected due to the
			 * mismatch of internalformat and format.
			 */
			expected_3D_error = GL_INVALID_OPERATION;
			alt_3D_error = GL_NO_ERROR;

			/* Cube map depth textures are only supported
			 * with GL_OES_depth_texture_cube_map.
			 * Without that extension,
			 * GL_INVALID_OPERATION is expected.  That
			 * error is already expected due to the
			 * mismatch of internalformat and format.
			 */
			expected_cube_error = GL_INVALID_OPERATION;
			alt_cube_error = GL_NO_ERROR;
		} else {
			expected_error = GL_NO_ERROR;
			alt_error = GL_NO_ERROR;

			/* 3D depth textures are never supported.
			 * GL_INVALID_OPERATION is expected.
			 */
			expected_3D_error = GL_INVALID_OPERATION;
			alt_3D_error = GL_NO_ERROR;

			/* Cube map depth textures are only supported
			 * with GL_OES_depth_texture_cube_map.
			 * Without that extension,
			 * GL_INVALID_OPERATION is expected.
			 */
			expected_cube_error = has_depth_texture_cube_map
				? GL_NO_ERROR : GL_INVALID_OPERATION;
			alt_cube_error = GL_NO_ERROR;
		}
	} else {
		assert(piglit_get_gl_version() < 30);

		if (internalFormat != GL_DEPTH_STENCIL) {
			/* For all of the cases either GL_INVALID_VALUE could
			 * be generated due to format being GL_DEPTH_STENCIL
			 * or GL_INVALID_OPERATION could be generated due to
			 * format not being the same as internalformat.
			 */
			expected_error = GL_INVALID_OPERATION;
			alt_error = GL_INVALID_VALUE;

			expected_3D_error = GL_INVALID_OPERATION;
			alt_3D_error = GL_INVALID_VALUE;

			expected_cube_error = GL_INVALID_OPERATION;
			alt_cube_error = GL_INVALID_VALUE;
		} else {
			/* For all of these cases, GL_INVALID_VALUE is the
			 * only acceptable error.  The OpenGL ES 1.x and 2.0
			 * specs make no mention of generating
			 * GL_INVALID_OPERATION for the 3D or cube map cases.
			 */
			expected_error = GL_INVALID_VALUE;
			alt_error = GL_NO_ERROR;

			expected_3D_error = GL_INVALID_VALUE;
			alt_3D_error = GL_NO_ERROR;

			expected_cube_error = GL_INVALID_VALUE;
			alt_cube_error = GL_NO_ERROR;
		}
	}
#endif /* !defined PIGLIT_USE_OPENGL */

	printf("Testing glTexImage with %s...\n",
	       piglit_get_gl_enum_name(internalFormat));

	glGenTextures(ARRAY_SIZE(tex), tex);

#if defined PIGLIT_USE_OPENGL
	glBindTexture(GL_TEXTURE_1D, tex[0]);
	glTexImage1D(GL_TEXTURE_1D, 0, internalFormat,
		     16, 0,
		     GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	pass = check_gl_error2(expected_error, alt_error) && pass;
#endif

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
		     16, 16, 0,
		     GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	pass = check_gl_error2(expected_error, alt_error) && pass;


	/* Section 3.8.1 (Texture Image Specification) of the OpenGL 2.1 spec
	 * says:
	 *
	 *     "Textures with a base internal format of DEPTH_COMPONENT are
	 *     supported by texture image specification commands only if
	 *     target is TEXTURE_1D, TEXTURE_2D, PROXY_TEXTURE_1D or
	 *     PROXY_TEXTURE_2D. Using this format in conjunction with any
	 *     other target will result in an INVALID_OPERATION error."
	 *
	 * The OpenGL 4.4 spec lists the same error, but it greatly expands
	 * the list of valid texture targets.
	 */
#if !defined PIGLIT_USE_OPENGL_ES1
	if (has_texture_3d) {
		glBindTexture(GL_TEXTURE_3D, tex[2]);
		glTexImage3D(GL_TEXTURE_3D, 0, internalFormat,
			     8, 8, 8, 0,
			     GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		pass = check_gl_error2(expected_3D_error, alt_3D_error) && pass;
	}
#else
	/* Silence "variable ‘expected_3D_error’ set but not used" warnings.
	 */
	(void) expected_3D_error;
	(void) alt_3D_error;
#endif

	if (has_texture_cube_map) {
		unsigned i;

		glBindTexture(GL_TEXTURE_CUBE_MAP, tex[3]);

		for (i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				     0, internalFormat,
				     16, 16, 0,
				     GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8,
				     NULL);
			pass = check_gl_error2(expected_cube_error,
					       alt_cube_error)
				&& pass;
		}
	}

#if defined PIGLIT_USE_OPENGL
	glBindTexture(GL_TEXTURE_1D, 0);
#endif
	glBindTexture(GL_TEXTURE_2D, 0);
	if (has_texture_3d)
		glBindTexture(GL_TEXTURE_3D, 0);
	if (has_texture_cube_map)
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glDeleteTextures(ARRAY_SIZE(tex), tex);

	printf("Done.\n\n");

	return pass;
}

#if !defined PIGLIT_USE_OPENGL_ES1
static bool
try_TexStorage(GLenum internalFormat)
{
	bool pass = true;
	GLuint tex[4];

	GLenum expected_error;
	GLenum expected_3D_error;
	GLenum expected_cube_error;

	/* The GL_ARB_texture_storage spec says:
	 *
	 *     "- If <internalformat> is one of the internal formats listed in
	 *      table 3.11, an INVALID_ENUM error is generated."
	 *
	 * Table 3.11 lists the unsized formats, including GL_DEPTH_STENCIL.
	 */
	if (internalFormat == GL_DEPTH_STENCIL) {
		expected_error = has_depth_texture
			? GL_INVALID_ENUM : GL_INVALID_VALUE;
		expected_3D_error = has_depth_texture
			? GL_INVALID_ENUM : GL_INVALID_VALUE;
		expected_cube_error = has_depth_texture_cube_map
			? GL_INVALID_ENUM : GL_INVALID_VALUE;
	} else {
		expected_error = has_depth_texture
			? GL_NO_ERROR : GL_INVALID_VALUE;
		expected_3D_error = has_depth_texture
			? GL_INVALID_OPERATION : GL_INVALID_VALUE;
		expected_cube_error = has_depth_texture_cube_map
			? GL_NO_ERROR : expected_3D_error;
	}

	printf("Testing glTexStorage with %s...\n",
	       piglit_get_gl_enum_name(internalFormat));

	glGenTextures(ARRAY_SIZE(tex), tex);

#if defined PIGLIT_USE_OPENGL
	glBindTexture(GL_TEXTURE_1D, tex[0]);
	glTexStorage1D(GL_TEXTURE_1D, 1, internalFormat, 16);
	pass = piglit_check_gl_error(expected_error) && pass;
#endif

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, 16, 16);
	pass = piglit_check_gl_error(expected_error) && pass;

#if !defined PIGLIT_USE_OPENGL_ES1
	if (has_texture_3d) {
		glBindTexture(GL_TEXTURE_3D, tex[2]);
		glTexStorage3D(GL_TEXTURE_3D, 1, internalFormat, 8, 8, 8);
		pass = piglit_check_gl_error(expected_3D_error) && pass;
	}
#else
	/* Silence "variable ‘expected_3D_error’ set but not used" warnings.
	 */
	(void) expected_3D_error;
#endif

	if (has_texture_cube_map) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex[3]);
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, internalFormat, 16, 16);
		pass = piglit_check_gl_error(expected_cube_error) && pass;
	}

#if defined PIGLIT_USE_OPENGL
	glBindTexture(GL_TEXTURE_1D, 0);
#endif
	glBindTexture(GL_TEXTURE_2D, 0);
	if (has_texture_3d)
		glBindTexture(GL_TEXTURE_3D, 0);
	if (has_texture_cube_map)
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glDeleteTextures(ARRAY_SIZE(tex), tex);

	printf("Done.\n\n");

	return pass;
}
#endif /* !defined PIGLIT_USE_OPENGL_ES1 */

void piglit_init(int argc, char **argv)
{
	bool pass = true;
	bool has_texture_storage;

#if defined PIGLIT_USE_OPENGL
	if (piglit_get_gl_version() < 30
	    && !piglit_is_extension_supported("GL_EXT_packed_depth_stencil")
	    && !piglit_is_extension_supported("GL_ARB_framebuffer_object")) {
		printf("OpenGL 3.0, GL_EXT_packed_depth_stencil, or "
		       "GL_ARB_framebuffer_object is required.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	has_depth_texture = piglit_get_gl_version() >= 14
		|| piglit_is_extension_supported("GL_ARB_depth_texture")
		|| piglit_is_extension_supported("GL_SGIX_depth_texture");

	has_texture_3d = true;

	has_texture_cube_map = piglit_get_gl_version() >= 13
		|| piglit_is_extension_supported("GL_ARB_texture_cube_map");

	has_depth_texture_cube_map = piglit_get_gl_version() >= 30
		|| piglit_is_extension_supported("GL_EXT_gpu_shader4");

	has_texture_storage = piglit_get_gl_version() >= 42
		|| piglit_is_extension_supported("GL_ARB_texture_storage");
#elif defined PIGLIT_USE_OPENGL_ES2
	if (piglit_get_gl_version() < 30
	    && !piglit_is_extension_supported("GL_OES_packed_depth_stencil")) {
		printf("OpenGL ES 3.0 or GL_OES_packed_depth_stencil "
		       "is required.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	has_depth_texture = piglit_get_gl_version() >= 30
		|| piglit_is_extension_supported("GL_OES_depth_texture");

	has_texture_3d = piglit_get_gl_version() >= 30
		|| piglit_is_extension_supported("GL_OES_texture_3D");

	has_texture_cube_map = true;

	has_depth_texture_cube_map = piglit_get_gl_version() >= 30
		|| piglit_is_extension_supported("GL_OES_depth_texture_cube_map");

	has_texture_storage = piglit_get_gl_version() >= 30
		|| piglit_is_extension_supported("GL_EXT_texture_storage");
#elif defined PIGLIT_USE_OPENGL_ES1
	piglit_require_extension("GL_OES_packed_depth_stencil");

	has_depth_texture = false;

	has_texture_3d = false;

	has_texture_cube_map =
		piglit_is_extension_supported("GL_OES_texture_cube_map");

	has_depth_texture_cube_map = false;

	has_texture_storage =
		piglit_is_extension_supported("GL_EXT_texture_storage");
#endif

	pass = try_TexImage(GL_DEPTH_STENCIL) && pass;
	pass = try_TexImage(GL_DEPTH24_STENCIL8) && pass;

	/* Disable this path for OpenGL ES 1.x because piglit dispatch doesn't
	 * seem to support it yet.
	 */
#if !defined PIGLIT_USE_OPENGL_ES1
	if (has_texture_storage) {
		pass = try_TexStorage(GL_DEPTH_STENCIL) && pass;
		pass = try_TexStorage(GL_DEPTH24_STENCIL8) && pass;
	}
#else
	/* Silence "variable ‘has_texture_storage’ set but not used" warnings.
	 */
	(void) has_texture_storage;
#endif

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
