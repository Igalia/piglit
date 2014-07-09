/*
 * Copyright © 2011 Intel Corporation
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
 * @file fbo-blending.c
 *
 * From the GL_EXT_texture_integer spec:
 *
 *     "Per-fragment operations that require floating-point color
 *      components, including multisample alpha operations, alpha test,
 *      blending, and dithering, have no effect when the corresponding
 *      colors are written to an integer color buffer."
 *
 * This test covers alpha test, blending, and dithering.  All formats
 * tested due to failures in i965 differing based on render target
 * format.
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint tex;
static int color_loc;
/* Numbers chosen to always avoid clamping -- we should test that in
 * some other test.
 */
static const uint32_t color[4] = {
	0x5,
	0x4,
	0x3,
	0x2,
};

struct format_info {
	const char *name;
	GLenum internal_format, base_format;
	int size;
	bool sign;
};


/* Only test 32-bit formats - since you won't see precision problems on lower sizes */
static const struct format_info formats[] = {
	{ "GL_RGBA8I",   GL_RGBA8I,   GL_RGBA_INTEGER, 8,  true  },
	{ "GL_RGBA8UI",  GL_RGBA8UI , GL_RGBA_INTEGER, 8,  false },
	{ "GL_RGBA16I",  GL_RGBA16I,  GL_RGBA_INTEGER, 16, true  },
	{ "GL_RGBA16UI", GL_RGBA16UI, GL_RGBA_INTEGER, 16, false },
	{ "GL_RGBA32I",  GL_RGBA32I,  GL_RGBA_INTEGER, 32, true  },
	{ "GL_RGBA32UI", GL_RGBA32UI, GL_RGBA_INTEGER, 32, false },

	{ "GL_RGBA8I (bgra)",   GL_RGBA8I,   GL_BGRA_INTEGER, 8,  true  },
	{ "GL_RGBA8UI (bgra)",  GL_RGBA8UI , GL_BGRA_INTEGER, 8,  false },
	{ "GL_RGBA16I (bgra)",  GL_RGBA16I,  GL_BGRA_INTEGER, 16, true  },
	{ "GL_RGBA16UI (bgra)", GL_RGBA16UI, GL_BGRA_INTEGER, 16, false },
	{ "GL_RGBA32I (bgra)",  GL_RGBA32I,  GL_BGRA_INTEGER, 32, true  },
	{ "GL_RGBA32UI (bgra)", GL_RGBA32UI, GL_BGRA_INTEGER, 32, false },

	{ "GL_RGB8I",   GL_RGB8I,   GL_RGB_INTEGER, 8,  true  },
	{ "GL_RGB8UI",  GL_RGB8UI , GL_RGB_INTEGER, 8,  false },
	{ "GL_RGB16I",  GL_RGB16I,  GL_RGB_INTEGER, 16, true  },
	{ "GL_RGB16UI", GL_RGB16UI, GL_RGB_INTEGER, 16, false },
	{ "GL_RGB32I",  GL_RGB32I,  GL_RGB_INTEGER, 32, true  },
	{ "GL_RGB32UI", GL_RGB32UI, GL_RGB_INTEGER, 32, false },

	{ "GL_ALPHA8I_EXT",   GL_ALPHA8I_EXT,   GL_ALPHA_INTEGER_EXT, 8,  true  },
	{ "GL_ALPHA8UI_EXT",  GL_ALPHA8UI_EXT , GL_ALPHA_INTEGER_EXT, 8,  false },
	{ "GL_ALPHA16I_EXT",  GL_ALPHA16I_EXT,  GL_ALPHA_INTEGER_EXT, 16, true  },
	{ "GL_ALPHA16UI_EXT", GL_ALPHA16UI_EXT, GL_ALPHA_INTEGER_EXT, 16, false },
	{ "GL_ALPHA32I_EXT",  GL_ALPHA32I_EXT,  GL_ALPHA_INTEGER_EXT, 32, true  },
	{ "GL_ALPHA32UI_EXT", GL_ALPHA32UI_EXT, GL_ALPHA_INTEGER_EXT, 32, false },

	{ "GL_LUMINANCE8I_EXT",   GL_LUMINANCE8I_EXT,   GL_LUMINANCE_INTEGER_EXT, 8,  true  },
	{ "GL_LUMINANCE8UI_EXT",  GL_LUMINANCE8UI_EXT , GL_LUMINANCE_INTEGER_EXT, 8,  false },
	{ "GL_LUMINANCE16I_EXT",  GL_LUMINANCE16I_EXT,  GL_LUMINANCE_INTEGER_EXT, 16, true  },
	{ "GL_LUMINANCE16UI_EXT", GL_LUMINANCE16UI_EXT, GL_LUMINANCE_INTEGER_EXT, 16, false },
	{ "GL_LUMINANCE32I_EXT",  GL_LUMINANCE32I_EXT,  GL_LUMINANCE_INTEGER_EXT, 32, true  },
	{ "GL_LUMINANCE32UI_EXT", GL_LUMINANCE32UI_EXT, GL_LUMINANCE_INTEGER_EXT, 32, false },

	{ "GL_LUMINANCE_ALPHA8I_EXT",   GL_LUMINANCE_ALPHA8I_EXT,   GL_LUMINANCE_ALPHA_INTEGER_EXT, 8,  true  },
	{ "GL_LUMINANCE_ALPHA8UI_EXT",  GL_LUMINANCE_ALPHA8UI_EXT , GL_LUMINANCE_ALPHA_INTEGER_EXT, 8,  false },
	{ "GL_LUMINANCE_ALPHA16I_EXT",  GL_LUMINANCE_ALPHA16I_EXT,  GL_LUMINANCE_ALPHA_INTEGER_EXT, 16, true  },
	{ "GL_LUMINANCE_ALPHA16UI_EXT", GL_LUMINANCE_ALPHA16UI_EXT, GL_LUMINANCE_ALPHA_INTEGER_EXT, 16, false },
	{ "GL_LUMINANCE_ALPHA32I_EXT",  GL_LUMINANCE_ALPHA32I_EXT,  GL_LUMINANCE_ALPHA_INTEGER_EXT, 32, true  },
	{ "GL_LUMINANCE_ALPHA32UI_EXT", GL_LUMINANCE_ALPHA32UI_EXT, GL_LUMINANCE_ALPHA_INTEGER_EXT, 32, false },

	{ "GL_INTENSITY8I_EXT",   GL_INTENSITY8I_EXT,   GL_RED_INTEGER_EXT, 8,  true  },
	{ "GL_INTENSITY8UI_EXT",  GL_INTENSITY8UI_EXT , GL_RED_INTEGER_EXT, 8,  false },
	{ "GL_INTENSITY16I_EXT",  GL_INTENSITY16I_EXT,  GL_RED_INTEGER_EXT, 16, true  },
	{ "GL_INTENSITY16UI_EXT", GL_INTENSITY16UI_EXT, GL_RED_INTEGER_EXT, 16, false },
	{ "GL_INTENSITY32I_EXT",  GL_INTENSITY32I_EXT,  GL_RED_INTEGER_EXT, 32, true  },
	{ "GL_INTENSITY32UI_EXT", GL_INTENSITY32UI_EXT, GL_RED_INTEGER_EXT, 32, false },
};

static const struct format_info rg_formats[] = {
	{ "GL_RG8I",   GL_RG8I,   GL_RG_INTEGER, 8,  true  },
	{ "GL_RG8UI",  GL_RG8UI , GL_RG_INTEGER, 8,  false },
	{ "GL_RG16I",  GL_RG16I,  GL_RG_INTEGER, 16, true  },
	{ "GL_RG16UI", GL_RG16UI, GL_RG_INTEGER, 16, false },
	{ "GL_RG32I",  GL_RG32I,  GL_RG_INTEGER, 32, true  },
	{ "GL_RG32UI", GL_RG32UI, GL_RG_INTEGER, 32, false },
	{ "GL_R8I",   GL_R8I,   GL_RED_INTEGER, 8,  true  },
	{ "GL_R8UI",  GL_R8UI , GL_RED_INTEGER, 8,  false },
	{ "GL_R16I",  GL_R16I,  GL_RED_INTEGER, 16, true  },
	{ "GL_R16UI", GL_R16UI, GL_RED_INTEGER, 16, false },
	{ "GL_R32I",  GL_R32I,  GL_RED_INTEGER, 32, true  },
	{ "GL_R32UI", GL_R32UI, GL_RED_INTEGER, 32, false },
};

static const struct format_info rgb10_formats[] = {
	{ "GL_RGB10_A2UI", GL_RGB10_A2UI, GL_RGBA_INTEGER, 10, false },
	{ "GL_RGB10_A2UI (bgra)", GL_RGB10_A2UI, GL_BGRA_INTEGER, 10, false },
	{ "GL_RGB10_A2UI (rev)", GL_RGB10_A2UI, GL_RGBA_INTEGER, 10, true },
	{ "GL_RGB10_A2UI (rev bgra)", GL_RGB10_A2UI, GL_BGRA_INTEGER, 10, true },
};

static GLenum
get_datatype(const struct format_info *info)
{
	switch (info->size) {
	case 8:
		return info->sign ? GL_BYTE : GL_UNSIGNED_BYTE;
	case 16:
		return info->sign ? GL_SHORT : GL_UNSIGNED_SHORT;
	case 32:
		return info->sign ? GL_INT : GL_UNSIGNED_INT;
	case 10:
		return GL_UNSIGNED_INT_2_10_10_10_REV;
	default:
		assert(0);
		return 0;
	}
}

static void
usage(void)
{
	fprintf(stderr, "usage:\n");
	fprintf(stderr,
		"ext_texture_integer-fbo-blending "
		"[GL_ARB_texture_rg | GL_ARB_texture_rgb10_a2ui]\n");
	exit(1);
}

static enum piglit_result
test_format(const struct format_info *info)
{
	const GLenum type = get_datatype(info);
	GLenum status;
	uint32_t expected_color[4];

	printf("%s:\n", info->name);

	/* Create texture */
	glTexImage2D(GL_TEXTURE_2D, 0, info->internal_format, 1, 1, 0,
		     info->base_format, type, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			       GL_TEXTURE_2D, tex, 0);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf("  framebuffer incomplete.\n");
		return PIGLIT_SKIP;
	}

	piglit_draw_rect(-1, -1, 2, 2);

	memcpy(expected_color, color, sizeof(color));
	switch (info->base_format) {
	case GL_RGBA_INTEGER:
	case GL_BGRA_INTEGER:
		break;
	case GL_RGB_INTEGER:
		expected_color[3] = 1;
		break;
	case GL_LUMINANCE_INTEGER_EXT:
		expected_color[1] = expected_color[2] = 0;
		expected_color[3] = 1;
		break;
	case GL_LUMINANCE_ALPHA_INTEGER_EXT:
		expected_color[1] = expected_color[2] = 0;
		break;
	case GL_RED_INTEGER:
		expected_color[1] = expected_color[2] = 0;
		expected_color[3] = 1;
		break;
	case GL_RG_INTEGER:
		expected_color[2] = 0;
		expected_color[3] = 1;
		break;
	case GL_ALPHA_INTEGER_EXT:
		expected_color[0] = 0;
		expected_color[1] = 0;
		expected_color[2] = 0;
		break;
	default:
		abort();
	}

	if (piglit_probe_rect_rgba_uint(0, 0, 1, 1, expected_color))
		return PIGLIT_PASS;
	else {
		printf("  Input color: %d %d %d %d\n",
		       color[0], color[1], color[2], color[3]);
		return PIGLIT_FAIL;
	}
}


enum piglit_result
piglit_display(void)
{
	/* unreached */
	return PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	GLuint fbo;
	static const char *vs_source =
		"#version 130\n"
		"void main()\n"
		"{\n"
		"   gl_Position = gl_Vertex;\n"
		"}\n";
	static const char *fs_source =
		"#version 130\n"
		"uniform uvec4 color;\n"
		"out uvec4 result;\n"
		"void main()\n"
		"{\n"
		"   result = color;\n"
		"}\n";
	GLuint prog;
	int f, i;
	enum piglit_result result = PIGLIT_SKIP;
	const struct format_info *test_formats = formats;
	int num_test_formats = ARRAY_SIZE(formats);

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "GL_ARB_texture_rg") == 0) {
			piglit_require_extension(argv[i]);
			test_formats = rg_formats;
			num_test_formats = ARRAY_SIZE(rg_formats);
		} else if (strcmp(argv[i], "GL_ARB_texture_rgb10_a2ui") == 0) {
			piglit_require_extension(argv[i]);
			test_formats = rgb10_formats;
			num_test_formats = ARRAY_SIZE(rgb10_formats);
		} else {
			usage();
			exit(1);
		}
	}

	piglit_require_extension("GL_EXT_texture_integer");
	piglit_require_GLSL_version(130);

	prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);
	color_loc = glGetUniformLocation(prog, "color");
	glUniform4uiv(color_loc, 1, color);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

	/* Turn on all the knobs (except multisample alpha, which
	 * we'll leave to an EXT_framebuffer_multisample test) that
	 * are supposed to be ignored.
	 */
	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO, GL_ZERO);

	glEnable(GL_DITHER);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_NEVER, 1);

	for (f = 0; f < num_test_formats; f++)
		piglit_merge_result(&result, test_format(&test_formats[f]));

	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	glDeleteFramebuffers(1, &fbo);

	piglit_report_result(result);
}
