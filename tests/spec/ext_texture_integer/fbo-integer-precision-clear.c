/*
 * Copyright (c) 2010 VMware, Inc.
 * Copyright (c) 2011 Dave Airlie
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
 * @file fbo-integer-precision-clear.c
 *
 * Tests FBO integer clearing with a value that is outside a float
 * precision.  If any part of the stack does an int->float conversion
 * this test will fail.
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "fbo-integer-precision-clear";

static GLint TexWidth = 256, TexHeight = 256;

struct format_info
{
	const char *Name;
	GLenum IntFormat, BaseFormat;
	GLuint BitsPerChannel;
	GLboolean Signed;
};

/* Only test 32-bit formats - since you won't see precision problems on lower sizes */
static const struct format_info Formats[] = {
	{ "GL_RGBA32I_EXT",  GL_RGBA32I_EXT,  GL_RGBA_INTEGER_EXT, 32, GL_TRUE  },
	{ "GL_RGBA32UI_EXT", GL_RGBA32UI_EXT, GL_RGBA_INTEGER_EXT, 32, GL_FALSE },

	{ "GL_RGB32I_EXT",  GL_RGB32I_EXT,  GL_RGB_INTEGER_EXT, 32, GL_TRUE  },
	{ "GL_RGB32UI_EXT", GL_RGB32UI_EXT, GL_RGB_INTEGER_EXT, 32, GL_FALSE },
};

#define NUM_FORMATS  (sizeof(Formats) / sizeof(Formats[0]))

static int
num_components(GLenum format)
{
	switch (format) {
	case GL_RGBA:
	case GL_RGBA_INTEGER_EXT:
		return 4;
	case GL_RGB_INTEGER_EXT:
		return 3;
	case GL_ALPHA_INTEGER_EXT:
		return 1;
	case GL_LUMINANCE_INTEGER_EXT:
		return 1;
	case GL_LUMINANCE_ALPHA_INTEGER_EXT:
		return 2;
	case GL_RED_INTEGER_EXT:
		return 1;
	default:
		assert(0);
		return 0;
	}
}


static GLenum
get_datatype(const struct format_info *info)
{
	switch (info->BitsPerChannel) {
	case 8:
		return info->Signed ? GL_BYTE : GL_UNSIGNED_BYTE;
	case 16:
		return info->Signed ? GL_SHORT : GL_UNSIGNED_SHORT;
	case 32:
		return info->Signed ? GL_INT : GL_UNSIGNED_INT;
	default:
		assert(0);
		return 0;
	}
}


static GLboolean
check_error(const char *file, int line)
{
	GLenum err = glGetError();
	if (err) {
		fprintf(stderr, "%s: error 0x%x at %s:%d\n",
			TestName, err, file, line);
		return GL_TRUE;
	}
	return GL_FALSE;
}


static enum piglit_result
test_fbo(const struct format_info *info)
{
	const int comps = num_components(info->BaseFormat);
	const GLenum type = get_datatype(info);
	GLint f;
	GLuint fbo, texObj;
	GLenum status;
	GLboolean intMode;
	GLint buf;
	static const GLint clr[4] = { 300000005, 7, 6, 5 };
	GLint pix[4], i;
	bool pass = true;

	if (0)
		fprintf(stderr, "============ Testing format = %s ========\n",
			info->Name);

	/* Create texture */
	glGenTextures(1, &texObj);
	glBindTexture(GL_TEXTURE_2D, texObj);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, info->IntFormat, TexWidth, TexHeight, 0,
		     info->BaseFormat, type, NULL);

	if (check_error(__FILE__, __LINE__))
		return PIGLIT_FAIL;

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT,
				 &f);
	assert(f == info->IntFormat);


	/* Create FBO to render to texture */
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			       GL_TEXTURE_2D, texObj, 0);

	if (check_error(__FILE__, __LINE__))
		return PIGLIT_FAIL;

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf("%s: framebuffer incomplete.\n", info->Name);
		return PIGLIT_SKIP;
	}


	glGetBooleanv(GL_RGBA_INTEGER_MODE_EXT, &intMode);
	if (check_error(__FILE__, __LINE__))
		return PIGLIT_FAIL;
	if (!intMode) {
		fprintf(stderr,
			"%s: GL_RGBA_INTEGER_MODE_EXT return GL_FALSE\n",
			TestName);
		return PIGLIT_FAIL;
	}

	glGetIntegerv(GL_READ_BUFFER, &buf);
	assert(buf == GL_COLOR_ATTACHMENT0_EXT);
	glGetIntegerv(GL_DRAW_BUFFER, &buf);
	assert(buf == GL_COLOR_ATTACHMENT0_EXT);

	glClearColorIiEXT(clr[0], clr[1], clr[2], clr[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	glReadPixels(5, 5, 1, 1, GL_RGBA_INTEGER_EXT, GL_INT, pix);

	for (i = 0; i < comps; i++) {
		if (pix[i] != clr[i]) {
			fprintf(stderr, "%s: glClear failed\n",
				TestName);
			fprintf(stderr, "  Texture format = %s\n",
				info->Name);
			fprintf(stderr, "  Expected %d, %d, %d, %d\n",
				clr[0], clr[1], clr[2], clr[3]);
			fprintf(stderr, "  Found %d, %d, %d, %d\n",
				pix[0], pix[1], pix[2], pix[3]);
			pass = false;
			break;
		}
	}

	piglit_present_results();

	glDeleteTextures(1, &texObj);
	glDeleteFramebuffers(1, &fbo);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


enum piglit_result
piglit_display(void)
{
	enum piglit_result result = PIGLIT_SKIP;
	int f;

	for (f = 0; f < NUM_FORMATS; f++) {
		piglit_merge_result(&result, test_fbo(&Formats[f]));
	}

	return result;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_texture_integer");

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
