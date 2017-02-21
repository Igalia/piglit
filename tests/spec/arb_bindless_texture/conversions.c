/*
 * Copyright (c) 2017 Valve Corporation
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

/** \file
 *
 * Test cases for conversions and explicit constructors.
 */

#include "common.h"

static struct piglit_gl_test_config *piglit_config;

PIGLIT_GL_TEST_CONFIG_BEGIN

	piglit_config = &config;
	config.supports_gl_core_version = 33;
	config.supports_gl_compat_version = 33;

PIGLIT_GL_TEST_CONFIG_END

static const char *passthrough_vs_src =
	"#version 330\n"
	"\n"
	"in vec4 piglit_vertex;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_Position = piglit_vertex;\n"
	"}\n";

static const char *sampler_types[] = {
	"sampler1D",
	"sampler2D",
	"sampler3D",
	"samplerCube",
	"sampler1DArray",
	"sampler2DArray",
	"samplerCubeArray",
	"sampler2DRect",
	"samplerBuffer",
	"sampler2DMS",
	"sampler2DMSArray",
	"isampler1D",
	"isampler2D",
	"isampler3D",
	"isamplerCube",
	"isampler1DArray",
	"isampler2DArray",
	"isamplerCubeArray",
	"isampler2DRect",
	"isamplerBuffer",
	"isampler2DMS",
	"isampler2DMSArray",
	"usampler1D",
	"usampler2D",
	"usampler3D",
	"usamplerCube",
	"usampler1DArray",
	"usampler2DArray",
	"usamplerCubeArray",
	"usampler2DRect",
	"usamplerBuffer",
	"usampler2DMS",
	"usampler2DMSArray",
	"sampler1DShadow",
	"sampler2DShadow",
	"samplerCubeShadow",
	"sampler1DArrayShadow",
	"sampler2DArrayShadow",
	"samplerCubeArrayShadow",
	"sampler2DRectShadow",
};

static const char *image_types[] = {
	"image1D",
	"image2D",
	"image3D",
	"image2DRect",
	"imageCube",
	"imageBuffer",
	"image1DArray",
	"image2DArray",
	"imageCubeArray",
	"image2DMS",
	"image2DMSArray",
	"iimage1D",
	"iimage2D",
	"iimage3D",
	"iimage2DRect",
	"iimageCube",
	"iimageBuffer",
	"iimage1DArray",
	"iimage2DArray",
	"iimageCubeArray",
	"iimage2DMS",
	"iimage2DMSArray",
	"uimage1D",
	"uimage2D",
	"uimage3D",
	"uimage2DRect",
	"uimageCube",
	"uimageBuffer",
	"uimage1DArray",
	"uimage2DArray",
	"uimageCubeArray",
	"uimage2DMS",
	"uimage2DMSArray",
};

GLuint vs;

static enum piglit_result
convert_sampler_to_uvec2(void *data)
{
	static const char *fs_src =
		"#version 330\n"
		"#extension GL_ARB_bindless_texture: require\n"
		"\n"
		"layout (bindless_sampler) uniform;\n"
		"\n"
		"uniform sampler2D given_tex;\n"
		"uniform uvec2 expected_uval;\n"
		"out vec4 color;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"	uvec2 packval = uvec2(given_tex);\n"
		"	if (packval != expected_uval)\n"
		"		color.r = 1.0;\n"
		"}\n";
	float expected[4] = { 0.0, 1.0, 0.0, 1.0 };
	GLuint expected_uval[2] = { 0x00040020, 0x1 };
	GLuint64 handle = 0x100040020;
	GLuint prog, fs;
	GLint loc;
	bool pass;

	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_src);
	prog = piglit_link_simple_program(vs, fs);
	glUseProgram(prog);

	loc = glGetUniformLocation(prog, "expected_uval");
	glUniform2uiv(loc, 1, expected_uval);

	loc = glGetUniformLocation(prog, "given_tex");
	glUniformHandleui64vARB(loc, 1, &handle);

	piglit_draw_rect(-1, -1, 1, 1);
	pass = piglit_probe_pixel_rgba(0, 0, expected);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
convert_uvec2_to_sampler(void *data)
{
	static const char *fs_src =
		"#version 400\n"
		"#extension GL_ARB_bindless_texture: require\n"
		"\n"
		"#define SAMPLER_TYPE %s\n"
		"uniform uvec2 given_uval;\n"
		"out vec4 color;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"	SAMPLER_TYPE tex = SAMPLER_TYPE(given_uval);\n"
		"	uvec2 pair = uvec2(tex);\n"
		"	if (pair != given_uval)\n"
		"		color.r = 1.0;\n"
		"}\n";
	float expected[4] = { 0.0, 1.0, 0.0, 1.0 };
	GLuint given_uval[2] = { 0x00040020, 0x1 };
	bool pass = true;
	GLuint prog, fs;
	char str[2048];
	GLint loc;
	int i;

	for (i = 0; i < ARRAY_SIZE(sampler_types); i++) {
		sprintf(str, fs_src, sampler_types[i]);
		fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, str);
		prog = piglit_link_simple_program(vs, fs);
		glUseProgram(prog);

		loc = glGetUniformLocation(prog, "given_uval");
		glUniform2uiv(loc, 1, given_uval);

		piglit_draw_rect(-1, -1, 1, 1);
		pass &= piglit_probe_pixel_rgba(0, 0, expected);
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
convert_image_to_uvec2(void *data)
{
	static const char *fs_src =
		"#version 330\n"
		"#extension GL_ARB_bindless_texture: require\n"
		"#extension GL_ARB_shader_image_load_store: enable\n"
		"\n"
		"layout (bindless_image) uniform;\n"
		"\n"
		"uniform writeonly image2D given_img;\n"
		"uniform uvec2 expected_uval;\n"
		"out vec4 color;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"	uvec2 packval = uvec2(given_img);\n"
		"	if (packval != expected_uval)\n"
		"		color.r = 1.0;\n"
		"}\n";
	float expected[4] = { 0.0, 1.0, 0.0, 1.0 };
	GLuint expected_val[2] = { 0x00040020, 0x1 };
	GLuint64 handle = 0x100040020;
	GLuint prog, fs;
	GLint loc;
	bool pass;

	if (!piglit_is_extension_supported("GL_ARB_shader_image_load_store"))
		return PIGLIT_SKIP;

	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_src);
	prog = piglit_link_simple_program(vs, fs);
	glUseProgram(prog);

	loc = glGetUniformLocation(prog, "expected_uval");
	glUniform2uiv(loc, 1, expected_val);

	loc = glGetUniformLocation(prog, "given_img");
	glUniformHandleui64vARB(loc, 1, &handle);

	piglit_draw_rect(-1, -1, 1, 1);
	pass = piglit_probe_pixel_rgba(0, 0, expected);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
convert_uvec2_to_image(void *data)
{
	static const char *fs_src =
		"#version 330\n"
		"#extension GL_ARB_bindless_texture: require\n"
		"#extension GL_ARB_shader_image_load_store: enable\n"
		"\n"
		"#define IMAGE_TYPE %s\n"
		"uniform uvec2 given_uval;\n"
		"out vec4 color;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"	writeonly IMAGE_TYPE img = IMAGE_TYPE(given_uval);\n"
		"	uvec2 pair = uvec2(img);\n"
		"	if (pair != given_uval)\n"
		"		color.r = 1.0;\n"
		"}\n";
	float expected[4] = { 0.0, 1.0, 0.0, 1.0 };
	GLuint given_uval[2] = { 0x00040020, 0x1 };
	bool pass = true;
	GLuint prog, fs;
	char str[2048];
	GLint loc;
	int i;

	if (!piglit_is_extension_supported("GL_ARB_shader_image_load_store"))
		return PIGLIT_SKIP;

	for (i = 0; i < ARRAY_SIZE(image_types); i++) {
		sprintf(str, fs_src, image_types[i]);
		fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, str);
		prog = piglit_link_simple_program(vs, fs);
		glUseProgram(prog);

		loc = glGetUniformLocation(prog, "given_uval");
		glUniform2uiv(loc, 1, given_uval);

		piglit_draw_rect(-1, -1, 1, 1);
		pass &= piglit_probe_pixel_rgba(0, 0, expected);
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static const struct piglit_subtest subtests[] = {
	{
		"Convert sampler to uvec2",
		"convert_sampler_to_uvec2",
		convert_sampler_to_uvec2,
		NULL
	},
	{
		"Convert uvec2 to sampler",
		"convert_uvec2_to_sampler",
		convert_uvec2_to_sampler,
	},
	{
		"Convert image to uvec2",
		"convert_image_to_uvec2",
		convert_image_to_uvec2,
		NULL
	},
	{
		"Convert uvec2 to image",
		"convert_uvec2_to_image",
		convert_uvec2_to_image,
	},
	{
		NULL,
		NULL,
		NULL,
		NULL
	}
};

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	enum piglit_result result;

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, passthrough_vs_src);

	piglit_require_extension("GL_ARB_bindless_texture");
	result = piglit_run_selected_subtests(subtests,
					      piglit_config->selected_subtests,
					      piglit_config->num_selected_subtests,
					      PIGLIT_SKIP);
	piglit_report_result(result);
}
