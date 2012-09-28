/*
 * Copyright (c) 2011 VMware, Inc.
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
 * NON-INFRINGEMENT.  IN NO EVENT SHALL VMWARE AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * Tests GL_ARB_texture_storage
 * Note: only the glTexStorage2D() function is tested with actual rendering.
 * Brian Paul
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 100;
	config.window_height = 100;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_ALPHA | PIGLIT_GL_VISUAL_DOUBLE;

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
static GLboolean
test_one_level_errors(GLenum target)
{
	const GLint width = 64, height = 4, depth = 8;
	GLuint tex;
	GLint v;

	assert(target == GL_TEXTURE_1D ||
	       target == GL_TEXTURE_2D ||
	       target == GL_TEXTURE_3D);

	glGenTextures(1, &tex);
	glBindTexture(target, tex);

	if (target == GL_TEXTURE_1D) {
		glTexStorage1D(target, 1, GL_RGBA8, width);
	}
	else if (target == GL_TEXTURE_2D) {
		glTexStorage2D(target, 1, GL_RGBA8, width, height);
	}
	else if (target == GL_TEXTURE_3D) {
		glTexStorage3D(target, 1, GL_RGBA8, width, height, depth);
	}

	piglit_check_gl_error(GL_NO_ERROR);

	glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &v);
	if (v != width) {
		printf("%s: bad width: %d, should be %d\n", TestName, v, width);
		return GL_FALSE;
	}

	if (target != GL_TEXTURE_1D) {
		glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &v);
		if (v != height) {
			printf("%s: bad height: %d, should be %d\n", TestName, v, height);
			return GL_FALSE;
		}
	}

	if (target == GL_TEXTURE_3D) {
		glGetTexLevelParameteriv(target, 0, GL_TEXTURE_DEPTH, &v);
		if (v != depth) {
			printf("%s: bad depth: %d, should be %d\n", TestName, v, depth);
			return GL_FALSE;
		}
	}

	/*
	 * Test immutability.
	 */
	if (target == GL_TEXTURE_2D) {
		glTexImage2D(target, 0, GL_RGBA, width, height, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		if (glGetError() != GL_INVALID_OPERATION) {
			printf("%s: glTexImage2D failed to generate error\n", TestName);
			return GL_FALSE;
		}

		glTexStorage2D(target, 1, GL_RGBA8, width, height);
		if (glGetError() != GL_INVALID_OPERATION) {
			printf("%s: glTexStorage2D() failed to generate error\n", TestName);
			return GL_FALSE;
		}

		glCopyTexImage2D(target, 0, GL_RGBA, 0, 0, width, height, 0);
		if (glGetError() != GL_INVALID_OPERATION) {
			printf("%s: glCopyTexImage2D() failed to generate error\n", TestName);
			return GL_FALSE;
		}
	}

	glDeleteTextures(1, &tex);

	return GL_TRUE;
}


/**
 * Do error-check tests for a mipmapped texture.
 */
static GLboolean
test_mipmap_errors(GLenum target)
{
	GLint width = 128, height = 64, depth = 4, levels = 8;
	const char *targetString = "";
	GLuint tex;
	GLint v, l;

	assert(target == GL_TEXTURE_1D ||
	       target == GL_TEXTURE_2D ||
	       target == GL_TEXTURE_3D);

	glGenTextures(1, &tex);
	glBindTexture(target, tex);

	if (target == GL_TEXTURE_1D) {
		glTexStorage1D(target, levels, GL_RGBA8, width);
		targetString = "GL_TEXTURE_1D";
	}
	else if (target == GL_TEXTURE_2D) {
		glTexStorage2D(target, levels, GL_RGBA8, width, height);
		targetString = "GL_TEXTURE_2D";
	}
	else if (target == GL_TEXTURE_3D) {
		glTexStorage3D(target, levels, GL_RGBA8, width, height, depth);
		targetString = "GL_TEXTURE_3D";
	}

	piglit_check_gl_error(GL_NO_ERROR);

	glGetTexParameteriv(target, GL_TEXTURE_IMMUTABLE_FORMAT, &v);
	if (!v) {
		printf("%s: %s GL_TEXTURE_IMMUTABLE_FORMAT query returned false\n",		       TestName, targetString);
		return GL_FALSE;
	}

	for (l = 0; l < levels; l++) {
		glGetTexLevelParameteriv(target, l, GL_TEXTURE_WIDTH, &v);
		if (v != width) {
			printf("%s: %s level %d: bad width: %d, should be %d\n",
			       TestName, targetString, l, v, width);
			return GL_FALSE;
		}

		if (target != GL_TEXTURE_1D) {
			glGetTexLevelParameteriv(target, l, GL_TEXTURE_HEIGHT, &v);
			if (v != height) {
				printf("%s: %s level %d: bad height: %d, should be %d\n",
				       TestName, targetString, l, v, height);
				return GL_FALSE;
			}
		}

		if (target == GL_TEXTURE_3D) {
			glGetTexLevelParameteriv(target, l, GL_TEXTURE_DEPTH, &v);
			if (v != depth) {
				printf("%s: %s level %d: bad depth: %d, should be %d\n",
				       TestName, targetString, l, v, depth);
				return GL_FALSE;
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

	return GL_TRUE;
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
static GLboolean
test_2d_mipmap_rendering(void)
{
	GLuint tex;
	GLint width = 128, height = 64, levels = 8;
	GLint v, l;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexStorage2D(GL_TEXTURE_2D, levels, GL_RGBA8, width, height);

	piglit_check_gl_error(GL_NO_ERROR);

	/* check that the mipmap level sizes are correct */
	for (l = 0; l < levels; l++) {
		GLubyte *buf = create_image(width, height, Colors[l]);

		glTexSubImage2D(GL_TEXTURE_2D, l, 0, 0, width, height,
				GL_RGBA, GL_UNSIGNED_BYTE, buf);

		free(buf);

		glGetTexLevelParameteriv(GL_TEXTURE_2D, l, GL_TEXTURE_WIDTH, &v);
		if (v != width) {
			printf("%s: level %d: bad width: %d, should be %d\n",
					 TestName, l, v, width);
			return GL_FALSE;
		}

		glGetTexLevelParameteriv(GL_TEXTURE_2D, l, GL_TEXTURE_HEIGHT, &v);
		if (v != height) {
			printf("%s: level %d: bad height: %d, should be %d\n",
					 TestName, l, v, height);
			return GL_FALSE;
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

		glTexSubImage2D(GL_TEXTURE_2D, levels, 0, 0, width, height,
				GL_RGBA, GL_UNSIGNED_BYTE, buf);

		err = glGetError();
		if (err == GL_NO_ERROR) {
			printf("%s: glTexSubImage2D(illegal level) failed to generate an error.\n",
			       TestName);
			return GL_FALSE;
		}

		free(buf);
	}

	/* now do a rendering test */
	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	/* draw a quad using each texture mipmap level */
	for (l = 0; l < levels; l++) {
		GLfloat expected[4];
		GLint p;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, l);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, l);

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
			return GL_FALSE;
		}
	}

	glDisable(GL_TEXTURE_2D);

	glDeleteTextures(1, &tex);

	return GL_TRUE;
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
		glGenTextures(1, &tex);
		glBindTexture(target, tex);

		glTexStorage2D(target, 1, legal_formats[i], 32, 32);

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
		glGenTextures(1, &tex);
		glBindTexture(target, tex);

		glTexStorage2D(target, 1, illegal_formats[i], 32, 32);

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
	


enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;

	pass = test_one_level_errors(GL_TEXTURE_1D) && pass;
	pass = test_one_level_errors(GL_TEXTURE_2D) && pass;
	pass = test_one_level_errors(GL_TEXTURE_3D) && pass;
	pass = test_mipmap_errors(GL_TEXTURE_1D) && pass;
	pass = test_mipmap_errors(GL_TEXTURE_2D) && pass;
	pass = test_mipmap_errors(GL_TEXTURE_3D) && pass;
	pass = test_2d_mipmap_rendering() && pass;
	pass = test_internal_formats() && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_texture_storage");
}
