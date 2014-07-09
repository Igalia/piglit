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
 * Tests texture views with data format changes. 1D textures only.
 * Uses multiple simultaneous views with different lifetimes and
 * check results via glGetTexImage().
 */

#include "piglit-util-gl.h"
#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 15;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/* Texture formats. The format_list variable has these fields: */
struct format_desc {
	const char  *name;
	GLenum      internalfmt;
	GLenum      storagefmt;
	GLenum      imagefmt;
	GLenum      imagetype;
	GLenum      getfmt;
	GLenum      gettype;
	int         red, green, blue, alpha;
};

#define FORMAT(f) #f, f
static const struct format_desc format_list[] = {
	{FORMAT(GL_RGBA8UI), GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE,
		GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, 8, 8, 8, 8},
	{FORMAT(GL_RGBA8I), GL_RGBA8I, GL_RGBA, GL_UNSIGNED_BYTE,
		GL_RGBA_INTEGER, GL_BYTE, 8, 8, 8, 8},
	{FORMAT(GL_RGB16F), GL_RGB16F, GL_RGB, GL_UNSIGNED_BYTE, GL_RGB,
		GL_HALF_FLOAT, 16, 16, 16, 0},
	{FORMAT(GL_RGB16I), GL_RGB16, GL_RGB, GL_UNSIGNED_BYTE,
		GL_RGB_INTEGER, GL_SHORT, 16, 16, 16, 0},
	{FORMAT(GL_R16UI), GL_R16, GL_RED, GL_UNSIGNED_BYTE,
		GL_RED_INTEGER, GL_UNSIGNED_SHORT, 16, 0, 0, 0},
	{FORMAT(GL_R16F), GL_R16, GL_RED, GL_UNSIGNED_BYTE,
		GL_RED, GL_HALF_FLOAT, 16, 0, 0, 0},
	{FORMAT(GL_RGBA16UI), GL_RGBA16, GL_RGBA, GL_UNSIGNED_BYTE,
		GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, 16, 16, 16, 16},
	{FORMAT(GL_RGBA16F), GL_RGBA16, GL_RGBA, GL_UNSIGNED_BYTE,
		GL_RGBA, GL_HALF_FLOAT, 16, 16, 16, 16},
};
#undef FORMAT


static bool
buffers_equal(const unsigned char * buf0, const unsigned char *buf1, GLint w)
{
	int i;
	for (i = 0; i < w; i++) {
		if (buf0[i] != buf1[i]) {
			printf("mismatched texels at index (%d)\n", i);
			printf("  Buffer0: %u\n", buf0[i]);
			printf("  Buffer1: %u\n", buf1[i]);
			return true;
		}
	}

	return false;
}

static bool
test_format_lifetime(const struct format_desc desc0,
		     const struct format_desc desc1)
{
	GLuint tex, viewTex[3];
	GLint width = 32, w, levels = 6;
	GLint l;
	int bytes;
	unsigned char *buffer0, *buffer1;
	bool pass = true;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_1D, tex);
	glTexStorage1D(GL_TEXTURE_1D, levels, desc0.storagefmt, width);
	glGenTextures(3, viewTex);
	glTextureView(viewTex[0], GL_TEXTURE_1D, tex,  desc0.internalfmt, 0,
			  levels, 0, 1);
	glTextureView(viewTex[1], GL_TEXTURE_1D, viewTex[0],
		      desc1.internalfmt, 0, levels, 0, 1);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* load each mipmap with a different color texture */
	w = width;
	bytes = (desc0.red + desc0.green + desc0.blue + desc0.alpha) / 8;
	for (l = 0; l < levels; l++) {
		GLubyte *buf = create_solid_image(width, 1, 1, bytes, l);

		if (buf != NULL) {
			glTexSubImage1D(GL_TEXTURE_1D, l, 0, w, desc0.imagefmt,
						desc0.imagetype, buf);
			free(buf);
		} else {
			piglit_report_result(PIGLIT_FAIL);
			assert(!"create_solid_image() failed\n");
		}

		if (w > 1)
			w /= 2;
	}

#if 0  /* debug */
	printf("fmt0=%s, fmt1=%s, strgfmt0=%s, gettype0=%s\n",
		   piglit_get_gl_enum_name(desc0.internalfmt),
		   piglit_get_gl_enum_name(desc1.internalfmt),
		   piglit_get_gl_enum_name(desc0.storagefmt),
		   piglit_get_gl_enum_name(desc0.gettype));
	printf("bytes=%d, width=%d, viewTex[0]=%d, [1]=%d, [2]=%d\n",bytes,
		   width, viewTex[0], viewTex[1], viewTex[2]);
#endif

	glDeleteTextures(1, &tex);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* compare view0 all level texels bytes to view1 texels bytes */
	buffer0 = malloc(width * bytes);
	buffer1 = malloc(width * bytes);
	w = width;
	for (l = 0; l < levels; l++) {
		glBindTexture(GL_TEXTURE_1D, viewTex[0]);
		glGetTexImage(GL_TEXTURE_1D, l, desc0.getfmt, desc0.gettype,
			      buffer0);

		glBindTexture(GL_TEXTURE_1D, viewTex[1]);
		glGetTexImage(GL_TEXTURE_1D, l, desc1.getfmt, desc1.gettype,
			      buffer1);
		if (buffers_equal(buffer0, buffer1, w)) {
			pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
			printf("level %d texel mismatch view0 and view1, width=%d\n",
			       l, w);
			printf("internal format0 %s, internal format1 %s\n",
			       piglit_get_gl_enum_name(desc0.internalfmt),
			       piglit_get_gl_enum_name(desc1.internalfmt));
			pass = false;
		}

		if (w > 1)
			w /= 2;
	}

	/* compare view1 base level texels to view2 after view0 and view1
	   deleted */
	glBindTexture(GL_TEXTURE_1D, viewTex[1]);
	glGetTexImage(GL_TEXTURE_1D, 0, desc1.getfmt, desc1.gettype, buffer1);

	glTextureView(viewTex[2], GL_TEXTURE_1D, viewTex[0],
		      desc0.internalfmt, 0, 1, 0, 1);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	glDeleteTextures(2, viewTex);
	glBindTexture(GL_TEXTURE_1D, viewTex[2]);
	glGetTexImage(GL_TEXTURE_1D, 0, desc0.getfmt, desc0.gettype, buffer0);

	if (buffers_equal(buffer0, buffer1, width)) {
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
		printf("Mismatched texels view1 and view2\n");
		printf("internal format0 %s, internal format1 %s\n",
			       piglit_get_gl_enum_name(desc0.internalfmt),
			       piglit_get_gl_enum_name(desc1.internalfmt));
		pass = false;
	}

	free(buffer0);
	free(buffer1);
	glDeleteTextures(1, &viewTex[2]);

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

	X(test_format_lifetime(format_list[4], format_list[5]),
	  "view compare 16 bit formats");
	X(test_format_lifetime(format_list[0], format_list[1]),
	  "view compare 32 bit formats");
	X(test_format_lifetime(format_list[2], format_list[3]),
	  "view compare 48 bit formats");
	X(test_format_lifetime(format_list[6], format_list[7]),
	  "view compare 64 bit formats");
#undef X
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
