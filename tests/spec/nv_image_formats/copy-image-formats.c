/*
 * Copyright (C) 2016 Intel Corporation
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

/** @file copy-image-formats.c
 *
 * A simple test verifying we can make use of the additional OpenGL ES 3.1
 * image formats provided by the GL_NV_image_formats extension. The
 * normalized 16 bits format provided by this extension are subject to the
 * condition that GL_EXT_texture_norm16 or equivalent is available.
 */

#include "piglit-util-gl.h"

const struct image_format {
	/** Format name as specified by GLSL. */
	const char *name;

	/** Format enum. */
	GLenum format;

	/** Pixel transfer format (e.g. as specified for glGetTexImage()). */
	GLenum pixel_format;

	/** Pixel transfer type (e.g. as specified for glGetTexImage()). */
	GLenum pixel_type;
} image_formats[] = {
	{ "rg32f", GL_RG32F, GL_RG, GL_FLOAT },
	{ "rg16f", GL_RG16F, GL_RG, GL_HALF_FLOAT },
	{ "r11f_g11f_b10f", GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV },
	{ "r16f", GL_R16F, GL_RED, GL_HALF_FLOAT },
	{ "rgb10_a2ui", GL_RGB10_A2UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV },
	{ "rg32ui", GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT },
	{ "rg16ui", GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_SHORT },
	{ "rg8ui", GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_BYTE },
	{ "r16ui", GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT },
	{ "r8ui", GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE },
	{ "rg32i", GL_RG32I, GL_RG_INTEGER, GL_INT },
	{ "rg16i", GL_RG16I, GL_RG_INTEGER, GL_SHORT },
	{ "rg8i", GL_RG8I, GL_RG_INTEGER, GL_BYTE },
	{ "r16i", GL_R16I, GL_RED_INTEGER, GL_SHORT },
	{ "r8i", GL_R8I, GL_RED_INTEGER, GL_BYTE },
	{ "rgba16", GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT },
	{ "rgb10_a2", GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV },
	{ "rg16", GL_RG16, GL_RG, GL_UNSIGNED_SHORT },
	{ "rg8", GL_RG8, GL_RG, GL_UNSIGNED_BYTE },
	{ "r16", GL_R16, GL_RED, GL_UNSIGNED_SHORT },
	{ "r8", GL_R8, GL_RED, GL_UNSIGNED_BYTE },
	{ "rgba16_snorm", GL_RGBA16_SNORM, GL_RGBA, GL_SHORT },
	{ "rg16_snorm", GL_RG16_SNORM, GL_RG, GL_SHORT },
	{ "rg8_snorm", GL_RG8_SNORM, GL_RG, GL_BYTE },
	{ "r16_snorm", GL_R16_SNORM, GL_RED, GL_SHORT },
	{ "r8_snorm", GL_R8_SNORM, GL_RED, GL_BYTE },
};

static struct piglit_subtest tests[ARRAY_SIZE(image_formats) + 1];
static struct piglit_gl_test_config * piglit_config;
static enum piglit_result run_test(void *);

PIGLIT_GL_TEST_CONFIG_BEGIN

piglit_config = &config;

for (unsigned i = 0; i < ARRAY_SIZE(image_formats); ++i) {

	char * name[64];
	asprintf(name, "copy-%s", image_formats[i].name);
	tests[i].name = *name;
	tests[i].option = image_formats[i].name;
	tests[i].subtest_func = run_test;
	tests[i].data = (void *)&image_formats[i];
}
config.subtests = tests;

config.supports_gl_es_version = 31;

PIGLIT_GL_TEST_CONFIG_END

#define WIDTH 16
#define HEIGHT 16

static const char *
glsl_image_type_name(GLenum format)
{
	switch (format) {
	case GL_RG32F:
	case GL_RG16F:
	case GL_R11F_G11F_B10F:
	case GL_R16F:
		/* Fallthrough */

	case GL_RGBA16_EXT:
	case GL_RGB10_A2:
	case GL_RG16_EXT:
	case GL_RG8:
	case GL_R16_EXT:
	case GL_R8:
		/* Fallthrough */

	case GL_RGBA16_SNORM_EXT:
	case GL_RG16_SNORM_EXT:
	case GL_RG8_SNORM:
	case GL_R16_SNORM_EXT:
	case GL_R8_SNORM:
		return "image";

	case GL_RGB10_A2UI:
	case GL_RG32UI:
	case GL_RG16UI:
	case GL_RG8UI:
	case GL_R16UI:
	case GL_R8UI:
		return "uimage";

	case GL_RG32I:
	case GL_RG16I:
	case GL_RG8I:
	case GL_R16I:
	case GL_R8I:
		return "iimage";

	default:
		assert("Unsupported format");
		return "";
	}
}

static const char *
glsl_type_name(GLenum format)
{
	switch (format) {
	case GL_RG32F:
	case GL_RG16F:
	case GL_R11F_G11F_B10F:
	case GL_R16F:
		/* Fallthrough */

	case GL_RGBA16_EXT:
	case GL_RGB10_A2:
	case GL_RG16_EXT:
	case GL_RG8:
	case GL_R16_EXT:
	case GL_R8:
		/* Fallthrough */

	case GL_RGBA16_SNORM_EXT:
	case GL_RG16_SNORM_EXT:
	case GL_RG8_SNORM:
	case GL_R16_SNORM_EXT:
	case GL_R8_SNORM:
		return "highp vec4";

	case GL_RGB10_A2UI:
	case GL_RG32UI:
	case GL_RG16UI:
	case GL_RG8UI:
	case GL_R16UI:
	case GL_R8UI:
		return "highp uvec4";

	case GL_RG32I:
	case GL_RG16I:
	case GL_RG8I:
	case GL_R16I:
	case GL_R8I:
		return "highp ivec4";

	default:
		assert("Unsupported format");
		return "";
	}
}

static bool
format_is_norm16(GLenum format)
{
	switch (format) {
	case GL_RGBA16_EXT:
	case GL_RG16_EXT:
	case GL_R16_EXT:
	case GL_RGBA16_SNORM_EXT:
	case GL_RG16_SNORM_EXT:
	case GL_R16_SNORM_EXT:
		return true;

	default:
		return false;
	}
}

static enum piglit_result
run_test(void * data)
{
	struct image_format * image_format = (struct image_format *)data;
	GLuint src, dst, prog;
	char *fs_source;

	glGenTextures(1, &src);
	glBindTexture(GL_TEXTURE_2D, src);
	glTexStorage2D(GL_TEXTURE_2D, 1, image_format->format, WIDTH, HEIGHT);
	glBindImageTexture(0, src, 0, GL_FALSE, 0,
			   GL_READ_ONLY, image_format->format);

	if (format_is_norm16(image_format->format)) {
		if (!piglit_is_extension_supported("GL_EXT_texture_norm16")) {
			piglit_check_gl_error(GL_INVALID_VALUE);
			return PIGLIT_PASS;
		}
	}
	piglit_check_gl_error(GL_NO_ERROR);

	glGenTextures(1, &dst);
	glBindTexture(GL_TEXTURE_2D, dst);
	glTexStorage2D(GL_TEXTURE_2D, 1, image_format->format, WIDTH, HEIGHT);
	glBindImageTexture(0, dst, 0, GL_FALSE, 0,
			   GL_WRITE_ONLY, image_format->format);
	piglit_check_gl_error(GL_NO_ERROR);

	fs_source = NULL;
	if (asprintf(&fs_source,
		     "#version 310 es\n"
		     "#extension GL_NV_image_formats : require\n"
		     "\n"
		     "layout(%s) readonly uniform highp %s2D img_src;\n"
		     "layout(%s) writeonly uniform highp %s2D img_dst;\n"
		     "\n"
		     "void main() {\n"
		     "  %s v = imageLoad(img_src, ivec2(gl_FragCoord));\n"
		     "  imageStore(img_dst, ivec2(gl_FragCoord), v);\n"
		     "}",
		     image_format->name,
		     glsl_image_type_name(image_format->format),
		     image_format->name,
		     glsl_image_type_name(image_format->format),
		     glsl_type_name(image_format->format)) < 0)
		return PIGLIT_FAIL;

	prog = piglit_build_simple_program(
		"#version 310 es\n"
		"\n"
		"in vec4 piglit_vertex;\n"
		"void main() {\n"
		"  gl_Position = piglit_vertex;\n"
		"}",
		fs_source);
	free(fs_source);

	glUseProgram(prog);

	piglit_draw_rect(-1, -1, 1, 1);

	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	enum piglit_result status = PIGLIT_PASS;

	piglit_require_extension("GL_NV_image_formats");

	status = piglit_run_selected_subtests(
		tests,
		piglit_config->selected_subtests,
		piglit_config->num_selected_subtests,
		status);

	piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
