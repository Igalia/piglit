/*
 * Copyright (c) 2011 VMware, Inc.
 * Copyright (c) 2014 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS  
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * Note: only the glTextureStorage2D() function is tested with actual 
 * rendering.
 * 
 * Original author:  Brian Paul
 * Adapted for testing ARB_direct_state_access by
 * Laura Ekstrand
 * November 2014
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 12;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "texture-storage";

static GLubyte Colors[][4] = {
	{255,	0,	0, 255},
	{  0, 255,	0, 255},
	{  0,	0, 255, 255},
	{  0, 255, 255, 255},
	{255,	0, 255, 255},
	{255, 255,	0, 255},
	{255, 255, 255, 255},
	{128,	0,	0, 255},
	{  0, 128,	0, 255},
	{  0,	0, 128, 255}
};


/**
 * Do error-check tests for a non-mipmapped texture.
 */
static bool
test_one_level_errors(GLenum target)
{
	const GLint width = 64, height = 4, depth = 8;
	GLuint tex;
	GLint v;

	assert(target == GL_TEXTURE_1D ||
	       target == GL_TEXTURE_2D ||
	       target == GL_TEXTURE_3D);

	glCreateTextures(target, 1, &tex);
	glBindTextureUnit(0, tex);

	if (target == GL_TEXTURE_1D) {
		glTextureStorage1D(tex, 1, GL_RGBA8, width);
	}
	else if (target == GL_TEXTURE_2D) {
		glTextureStorage2D(tex, 1, GL_RGBA8, width, height);
	}
	else if (target == GL_TEXTURE_3D) {
		glTextureStorage3D(tex, 1, GL_RGBA8, width, height, depth);
	}

	piglit_check_gl_error(GL_NO_ERROR);

	glGetTextureLevelParameteriv(tex, 0, GL_TEXTURE_WIDTH, &v);
	if (v != width) {
		printf("%s: bad width: %d, should be %d\n", TestName, v, width);
		return false;
	}

	if (target != GL_TEXTURE_1D) {
		glGetTextureLevelParameteriv(tex, 0, GL_TEXTURE_HEIGHT, &v);
		if (v != height) {
			printf("%s: bad height: %d, should be %d\n", TestName,
			       v, height);
			return false;
		}
	}

	if (target == GL_TEXTURE_3D) {
		glGetTextureLevelParameteriv(tex, 0, GL_TEXTURE_DEPTH, &v);
		if (v != depth) {
			printf("%s: bad depth: %d, should be %d\n", TestName,
			       v, depth);
			return false;
		}
	}

	/* The ARB_texture_storage spec says:
	 *
	 *     "Using any of the following commands with the same texture will
	 *     result in the error INVALID_OPERATION being generated, even if
	 *     it does not affect the dimensions or format:
	 *
	 *         - TexImage*
	 *         - CompressedTexImage*
	 *         - CopyTexImage*
	 *         - TexStorage*"
	 */
	if (target == GL_TEXTURE_2D) {
		glTexImage2D(target, 0, GL_RGBA, width, height, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		if (glGetError() != GL_INVALID_OPERATION) {
			printf("%s: glTexImage2D failed to generate error\n",
			       TestName);
			return false;
		}

		glTextureStorage2D(tex, 1, GL_RGBA8, width, height);
		if (glGetError() != GL_INVALID_OPERATION) {
			printf("%s: glTextureStorage2D() failed to generate "
			       "error\n", TestName);
			return false;
		}

		glCopyTexImage2D(target, 0, GL_RGBA, 0, 0, width, height, 0);
		if (glGetError() != GL_INVALID_OPERATION) {
			printf("%s: glCopyTexImage2D() failed to generate "
			       "error\n", TestName);
			return false;
		}
	}

	glDeleteTextures(1, &tex);

	return true;
}


/**
 * Do error-check tests for a mipmapped texture.
 */
static bool
test_mipmap_errors(GLenum target)
{
	GLint width = 128, height = 64, depth = 4, levels = 8;
	const char *targetString = "";
	GLuint tex;
	GLint v, l;

	assert(target == GL_TEXTURE_1D ||
	       target == GL_TEXTURE_2D ||
	       target == GL_TEXTURE_3D);

	glCreateTextures(target, 1, &tex);
	glBindTextureUnit(0, tex);

	if (target == GL_TEXTURE_1D) {
		glTextureStorage1D(tex, levels, GL_RGBA8, width);
		targetString = "GL_TEXTURE_1D";
	}
	else if (target == GL_TEXTURE_2D) {
		glTextureStorage2D(tex, levels, GL_RGBA8, width, height);
		targetString = "GL_TEXTURE_2D";
	}
	else if (target == GL_TEXTURE_3D) {
		glTextureStorage3D(tex, levels, GL_RGBA8, width, 
			height, depth);
		targetString = "GL_TEXTURE_3D";
	}

	piglit_check_gl_error(GL_NO_ERROR);

	glGetTextureParameteriv(tex, GL_TEXTURE_IMMUTABLE_FORMAT, &v);
	if (!v) {
		printf("%s: %s GL_TEXTURE_IMMUTABLE_FORMAT query returned "
		       "false\n",
		       TestName, targetString);
		return false;
	}

	for (l = 0; l < levels; l++) {
		glGetTextureLevelParameteriv(tex, l, GL_TEXTURE_WIDTH, &v);
		if (v != width) {
			printf("%s: %s level %d: bad width: %d, should be %d\n",
			       TestName, targetString, l, v, width);
			return false;
		}

		if (target != GL_TEXTURE_1D) {
			glGetTextureLevelParameteriv(tex, l, 
						     GL_TEXTURE_HEIGHT, &v);
			if (v != height) {
				printf("%s: %s level %d: bad height: %d, "
				       "should be %d\n",
				       TestName, targetString, l, v, height);
				return false;
			}
		}

		if (target == GL_TEXTURE_3D) {
			glGetTextureLevelParameteriv(tex, l, 
						     GL_TEXTURE_DEPTH, &v);
			if (v != depth) {
				printf("%s: %s level %d: bad depth: %d, "
				       "should be %d\n",
				       TestName, targetString, l, v, depth);
				return false;
			}
		}

		if (width > 1)
			width /= 2;
		if (height > 1)
			height /= 2;
		if (depth > 1)
			depth /= 2;
	}

	glDeleteTextures(1, &tex);

	return true;
}


static bool
test_cube_texture(void)
{
	const GLint width = 16, height = 16;
	const GLenum target = GL_TEXTURE_CUBE_MAP;
	GLuint tex;
	bool pass = true;

	/* Test valid cube dimensions */
	glCreateTextures(target, 1, &tex);
	glBindTextureUnit(0, tex);
	glTextureStorage2D(tex, 1, GL_RGBA8, width, height);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	glDeleteTextures(1, &tex);

	/* Test invalid cube dimensions */
	glCreateTextures(target, 1, &tex);
	glBindTextureUnit(0, tex);
	glTextureStorage2D(tex, 1, GL_RGBA8, width, height+2);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
	glDeleteTextures(1, &tex);

	return pass;
}


static bool
test_cube_array_texture(void)
{
	const GLint width = 16, height = 16;
	const GLenum target = GL_TEXTURE_CUBE_MAP_ARRAY;
	GLuint tex;
	bool pass = true;

	/* Test valid cube array dimensions */
	glCreateTextures(target, 1, &tex);
	glBindTextureUnit(0, tex);
	glTextureStorage3D(tex, 1, GL_RGBA8, width, height, 12);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	glDeleteTextures(1, &tex);

	/* Test invalid cube array width, height dimensions */
	glCreateTextures(target, 1, &tex);
	glBindTextureUnit(0, tex);
	glTextureStorage3D(tex, 1, GL_RGBA8, width, height+3, 12);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
	glDeleteTextures(1, &tex);

	/* Test invalid cube array depth */
	glCreateTextures(target, 1, &tex);
	glBindTextureUnit(0, tex);
	glTextureStorage3D(tex, 1, GL_RGBA8, width, height, 12+2);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
	glDeleteTextures(1, &tex);

	return pass;
}


/**
 * Create a single-color image.
 */
static GLubyte *
create_image(GLint w, GLint h, const GLubyte color[4])
{
	GLubyte *buf = (GLubyte *) malloc(w * h * 4);
	int i;
	for (i = 0; i < w * h; i++) {
		buf[i*4+0] = color[0];
		buf[i*4+1] = color[1];
		buf[i*4+2] = color[2];
		buf[i*4+3] = color[3];
	}
	return buf;
}


/**
 * Test a mip-mapped texture w/ rendering.
 */
static bool
test_2d_mipmap_rendering(void)
{
	GLuint tex;
	GLint width = 128, height = 64, levels = 8;
	GLint v, l;
	GLfloat vfloat;

	glCreateTextures(GL_TEXTURE_2D, 1, &tex);
	glBindTextureUnit(0, tex);

	glTextureStorage2D(tex, levels, GL_RGBA8, width, height);

	piglit_check_gl_error(GL_NO_ERROR);

	/* check that the mipmap level sizes are correct */
	for (l = 0; l < levels; l++) {
		GLubyte *buf = create_image(width, height, Colors[l]);

		glTextureSubImage2D(tex, l, 0, 0, width, height,
				GL_RGBA, GL_UNSIGNED_BYTE, buf);

		free(buf);

		glGetTextureLevelParameteriv(tex, l, GL_TEXTURE_WIDTH,
					     &v);
		if (v != width) {
			printf("%s: level %d: bad width: %d, should be %d\n",
					 TestName, l, v, width);
			return false;
		}

		glGetTextureLevelParameteriv(tex, l, GL_TEXTURE_HEIGHT,
					     &v);
		if (v != height) {
			printf("%s: level %d: bad height: %d, should be %d\n",
					 TestName, l, v, height);
			return false;
		}


		/* Added to test glGetTextureLevelParameterfv */
		glGetTextureLevelParameterfv(tex, l, GL_TEXTURE_WIDTH,
					     &vfloat);
		if (vfloat != (GLfloat) width) {
			printf("%s: level %d: bad width: %.2f, "
			       "should be %.2f\n",
				TestName, l, vfloat, (GLfloat) width);
			return false;
		}

		glGetTextureLevelParameterfv(tex, l, GL_TEXTURE_HEIGHT,
					     &vfloat);
		if (vfloat != (GLfloat) height) {
			printf("%s: level %d: bad height: %.2f, "
			       "should be %.2f\n",
				TestName, l, vfloat, (GLfloat) height);
			return false;
		}


		if (width > 1)
			width /= 2;
		if (height > 1)
			height /= 2;
	}

	/* This should generate and error (bad level) */
	{
		GLubyte *buf = create_image(width, height, Colors[l]);
		GLenum err;

		glTextureSubImage2D(tex, levels, 0, 0, width, height,
				GL_RGBA, GL_UNSIGNED_BYTE, buf);

		err = glGetError();
		if (err == GL_NO_ERROR) {
			printf("%s: glTextureSubImage2D(illegal level)"
			       " failed to generate an error.\n",
			       TestName);
			return false;
		}

		free(buf);
	}

	/* now do a rendering test */
	glEnable(GL_TEXTURE_2D);
	glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST_MIPMAP_NEAREST);
	glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	/* draw a quad using each texture mipmap level */
	for (l = 0; l < levels; l++) {
		GLfloat expected[4];
		GLint p;

		glTextureParameteri(tex, GL_TEXTURE_BASE_LEVEL, l);
		glTextureParameteri(tex, GL_TEXTURE_MAX_LEVEL, l);

		glClear(GL_COLOR_BUFFER_BIT);

		piglit_draw_rect_tex(-1.0, -1.0, 2.0, 2.0,
				     0.0, 0.0, 1.0, 1.0);

		expected[0] = Colors[l][0] / 255.0;
		expected[1] = Colors[l][1] / 255.0;
		expected[2] = Colors[l][2] / 255.0;
		expected[3] = Colors[l][3] / 255.0;

		p = piglit_probe_pixel_rgb(piglit_width/2, piglit_height/2,
					   expected);

		piglit_present_results();

		if (!p) {
			printf("%s: wrong color for mipmap level %d\n",
			       TestName, l);
			return false;
		}
	}

	glDisable(GL_TEXTURE_2D);

	glDeleteTextures(1, &tex);

	return true;
}


/**
 * Per issue 27 of the spec, only sized internalFormat values are allowed.
 * Ex: GL_RGBA8 is OK but GL_RGBA is illegal.
 * Check some common formats here.  These lists aren't exhaustive since
 * there are many extensions/versions that could effect the lists (ex:
 * integer formats, etc.)
 */
static bool
test_internal_formats(void)
{
	const GLenum target = GL_TEXTURE_2D;
	static const GLenum legal_formats[] = {
		GL_RGB4,
		GL_RGB5,
		GL_RGB8,
		GL_RGBA2,
		GL_RGBA4,
		GL_RGBA8,
		GL_DEPTH_COMPONENT16,
		GL_DEPTH_COMPONENT32
	};
	static const GLenum illegal_formats[] = {
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
		GL_COMPRESSED_SLUMINANCE_ALPHA
	};
	GLuint tex;
	bool pass = true;
	int i;

	for (i = 0; i < ARRAY_SIZE(legal_formats); i++) {
		glCreateTextures(target, 1, &tex);
		glBindTextureUnit(0, tex);

		glTextureStorage2D(tex, 1, legal_formats[i], 32, 32);

		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			printf("%s: internal format %s should be legal"
			       " but raised an error.",
			       TestName,
			       piglit_get_gl_enum_name(legal_formats[i]));
			pass = false;
		}

		glDeleteTextures(1, &tex);
	}

	for (i = 0; i < ARRAY_SIZE(illegal_formats); i++) {
		glCreateTextures(target, 1, &tex);
		glBindTextureUnit(0, tex);

		glTextureStorage2D(tex, 1, illegal_formats[i], 32, 32);

		if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
			printf("%s: internal format %s should be illegal"
			       " but didn't raised an error.",
			       TestName,
			       piglit_get_gl_enum_name(illegal_formats[i]));
			pass = false;
		}

		glDeleteTextures(1, &tex);
	}

	return pass;
}
	
static bool
test_immutablity(GLenum target)
{
	GLuint tex;
	GLint level;
	GLint immutable_format;

	bool pass = true;

	glCreateTextures(target, 1, &tex);
	glBindTextureUnit(0, tex);

	glTextureStorage2D(tex, 3, GL_RGBA8, 256, 256);
	glTextureParameteri(tex, GL_TEXTURE_MAX_LEVEL, 4);
	glGetTextureParameteriv(tex, GL_TEXTURE_MAX_LEVEL, &level);
	glGetTextureParameteriv(tex, GL_TEXTURE_IMMUTABLE_FORMAT,
			    &immutable_format);

	if (immutable_format != GL_TRUE) {
		printf("%s: GL_TEXTURE_IMMUTABLE_FORMAT was not set to "
		       "GL_TRUE after glTextureStorage2D\n", TestName);
		pass = false;
	}
	if (level != 2) {
		/* The ARB_texture_storage spec says:
		 *
		 *     "However, if TEXTURE_IMMUTABLE_FORMAT is TRUE, then
		 *     level_base is clamped to the range [0, <levels> - 1]
		 *     and level_max is then clamped to the range [level_base,
		 *     <levels> - 1], where <levels> is the parameter passed
		 *     the call to TexStorage* for the texture object"
		 */
		printf("%s: GL_TEXTURE_MAX_LEVEL changed to %d, which is "
		       "outside the clamp range for immutables\n",
		       TestName, level);
		pass = false;
	}

	/* Other immutable tests happen per-format above */

	glDeleteTextures(1, &tex);
	return pass;
}

#define X(f, n)								\
	do {								\
		const bool subtest_pass = (f);				\
		piglit_report_subtest_result(subtest_pass		\
					     ? PIGLIT_PASS : PIGLIT_FAIL, \
					     (n));			\
		pass = pass && subtest_pass;				\
	} while (0)

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	X(test_one_level_errors(GL_TEXTURE_1D), "1D non-mipmapped");
	X(test_one_level_errors(GL_TEXTURE_2D), "2D non-mipmapped");
	X(test_one_level_errors(GL_TEXTURE_3D), "3D non-mipmapped");
	X(test_mipmap_errors(GL_TEXTURE_1D), "1D mipmapped");
	X(test_mipmap_errors(GL_TEXTURE_2D), "2D mipmapped");
	X(test_mipmap_errors(GL_TEXTURE_3D), "3D mipmapped");
	X(test_2d_mipmap_rendering(), "2D mipmap rendering");
	X(test_internal_formats(), "internal formats");
	X(test_immutablity(GL_TEXTURE_2D), "immutability");

	if (piglit_get_gl_version() >= 13
	    || piglit_is_extension_supported("GL_ARB_texture_cube_map"))
		X(test_cube_texture(), "cube texture");
	else
		piglit_report_subtest_result(PIGLIT_SKIP,
					     "cube texture");

	if (piglit_is_extension_supported("GL_ARB_texture_cube_map_array"))
		X(test_cube_array_texture(), "cube array texture");
	else
		piglit_report_subtest_result(PIGLIT_SKIP,
					     "cube array texture");

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_ARB_direct_state_access");
}
