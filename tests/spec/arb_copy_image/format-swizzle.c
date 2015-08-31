/*
 * Copyright 2015 VMware, Inc.
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
 * This test exercises some subtle format issues for GL_ARB_copy_image.
 * If a driver supports texture formats which only vary by swizzling (ex: RGBA vs.
 * BGRA) we may wind up using different hardware texture formats depending
 * on the user-specified format and type arguments to glTexImage.
 * When we try to copy between such textures, the copy-sub-image code
 * must be able to handle the swizzling.
 *
 * Brian Paul
 * 31 August 2015
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 13;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END


static bool
test_combination(GLenum intFormat,
		 GLenum srcFormat, GLenum srcType,
		 GLenum dstFormat, GLenum dstType)
{
	const int width = 16, height = 16;
	int i;
	GLuint textures[2];
	GLubyte *image, *getimage;
	bool pass = true;
	int comps;

	switch (srcFormat) {
	case GL_RGB:
	case GL_BGR:
		comps = 3;
		break;
	case GL_RGBA:
	case GL_BGRA:
	case GL_RGBA_INTEGER:
	case GL_BGRA_INTEGER:
		comps = 4;
		break;
	default:
		assert(!"Unexpected format");
		comps = 4;
	}

	getimage = malloc(width * height * comps);

	image = malloc(width * height * comps);
	if (comps == 4) {
		for (i = 0; i < width * height; i++) {
			image[i * 4 + 0] = 0xff;
			image[i * 4 + 1] = 0x80;
			image[i * 4 + 2] = 0x40;
			image[i * 4 + 3] = 0x20;
		}
	}
	else {
		for (i = 0; i < width * height; i++) {
			image[i * 3 + 0] = 0xff;
			image[i * 3 + 1] = 0x80;
			image[i * 3 + 2] = 0x40;
		}
	}

	glGenTextures(2, textures);

	/* setup tex0 */
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, intFormat, width, height, 0,
		     srcFormat, srcType, image);

	/* setup tex1 */
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, intFormat, width, height, 0,
		     dstFormat, dstType, NULL);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		/* should be no errors */
		pass = false;
	}

	/* Copy from tex0 to tex1 */
	glCopyImageSubData(textures[0], GL_TEXTURE_2D, 0, /* src image */
			   0, 0, 0,  /* src offset */
			   textures[1], GL_TEXTURE_2D, 0, /* dst image */
			   0, 0, 0,  /* dst offset */
			   width, height, 1);  /* src size */

	/* Readback tex1 */
	glGetTexImage(GL_TEXTURE_2D, 0, srcFormat, srcType, getimage);

	if (memcmp(image, getimage, width * height * comps) != 0) {
		printf("Failure:\n");
		printf("  internal tex format=%s\n",
		       piglit_get_gl_enum_name(intFormat));
		printf("  src tex format=%s type=%s\n",
		       piglit_get_gl_enum_name(srcFormat),
		       piglit_get_gl_enum_name(srcType));
		printf("  dst tex format=%s type=%s\n",
		       piglit_get_gl_enum_name(dstFormat),
		       piglit_get_gl_enum_name(dstType));
		printf("Expected %u %u %u %u\n",
		       image[0], image[1], image[2], image[3]);
		printf("Found %u %u %u %u\n",
		       getimage[0], getimage[1], getimage[2], getimage[3]);
		pass = false;
	}

	glDeleteTextures(2, textures);

	free(image);
	free(getimage);

	return pass;
}


enum piglit_result
piglit_display(void)
{
	/* nothing */
	return PIGLIT_SKIP;
}


void
piglit_init(int argc, char **argv)
{
	static const GLenum formats[] = {
		GL_RGBA,
		GL_BGRA
	};
	static const GLenum rgbFormats[] = {
		GL_RGB,
		GL_BGR
	};
	static const GLenum intFormats[] = {
		GL_RGBA_INTEGER,
		GL_BGRA_INTEGER,
	};
	static const GLenum types[] = {
		GL_UNSIGNED_BYTE,
		GL_UNSIGNED_INT_8_8_8_8,
		GL_UNSIGNED_INT_8_8_8_8_REV,
	};
	int sf, st, df, dt;
	bool pass = true;

	piglit_require_extension("GL_ARB_copy_image");

	/* Test all RGBA format/type combinations for the src/dst textures */
	for (sf = 0; sf < ARRAY_SIZE(formats); sf++) {
		for (df = 0; df < ARRAY_SIZE(formats); df++) {
			for (st = 0; st < ARRAY_SIZE(types); st++) {
				for (dt = 0; dt < ARRAY_SIZE(types); dt++) {
					if (!test_combination(GL_RGBA,
							      formats[sf],
							      types[st],
							      formats[df],
							      types[dt])) {
						pass = false;
					}
				}
			}
		}
	}

	/* RGB formats */
	for (sf = 0; sf < ARRAY_SIZE(rgbFormats); sf++) {
		for (df = 0; df < ARRAY_SIZE(rgbFormats); df++) {
			if (!test_combination(GL_RGBA,
					      rgbFormats[sf],
					      GL_UNSIGNED_BYTE,
					      rgbFormats[df],
					      GL_UNSIGNED_BYTE)) {
				pass = false;
			}
		}
	}

	/* integer formats */
        if (piglit_is_extension_supported("GL_EXT_texture_integer")) {
		for (sf = 0; sf < ARRAY_SIZE(intFormats); sf++) {
			for (df = 0; df < ARRAY_SIZE(intFormats); df++) {
				for (st = 0; st < ARRAY_SIZE(types); st++) {
					for (dt = 0; dt < ARRAY_SIZE(types); dt++) {
						if (!test_combination(GL_RGBA8UI,
								      intFormats[sf],
								      types[st],
								      intFormats[df],
								      types[dt])) {
							pass = false;
						}
					}
				}
			}
		}
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
