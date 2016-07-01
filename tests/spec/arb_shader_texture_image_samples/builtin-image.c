/*
 * Copyright 2015 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** @file builtin-image.c
 *
 * Tests GLSL's imageSamples builtin. The code is highly-based on
 * the imageSize() tests.
 *
 * From GL_ARB_shader_texture_image_samples's spec:
 *
 * "Including the following line in a shader can be used to control the
 * language features described in this extension:
 *
 *     #extension GL_ARB_shader_texture_image_samples
 *
 * A new preprocessor #define is added to the OpenGL Shading Language:
 *
 *     #define GL_ARB_shader_texture_image_samples 1
 *
 * Add to table in section 8.9.1 "Texture Query Functions"
 *
 * Syntax:
 *
 *     int textureSamples(gsampler2DMS sampler)
 *     int textureSamples(gsampler2DMSArray sampler)
 *
 * Description:
 *
 *     Returns the number of samples of the texture or textures bound to
 *     <sampler>."
 */

#include "../arb_shader_image_load_store/common.h"
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
		PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static bool
randomize_image(const struct image_info img, unsigned unit)
{
	const unsigned n = image_num_components(img.format) * product(img.size);
	uint32_t *pixels = malloc(sizeof(uint32_t) * n);
	bool ret;

	ret = upload_image(img, unit, pixels);

	free(pixels);
	return ret;
}

static bool
check(const struct grid_info grid, struct image_info img_src)
{
	struct image_info img = image_info_for_grid(grid);
	const unsigned samples = img_src.size.x;
	const unsigned n = image_num_components(img.format) * product(img.size);
	uint32_t *pixels = malloc(sizeof(uint32_t) * n);
	bool ret;

	ret = download_result(grid, pixels);

	ret &= check_pixels(img, pixels, samples, samples, samples, samples);

	free(pixels);
	return ret;
}

static enum piglit_result
run_test(const struct image_format_info *format,
	 const struct image_target_info *target,
	 const struct image_stage_info *stage,
	 const struct image_extent size)
{
	const struct grid_info grid = grid_info(stage->stage, GL_RGBA32I,
						16, 16);
	const struct image_info img = {
		target, format, size,
		image_format_epsilon(grid.format)
	};
	GLuint prog = generate_program(
		grid, stage->stage,
		concat(hunk("#extension GL_ARB_shader_texture_image_samples : enable\n"),
		       image_hunk(img, ""),
		       hunk("readonly IMAGE_UNIFORM_T src_img;\n"
			    "\n"
			    "GRID_T op(ivec2 idx, GRID_T x) {\n"
			    "        return ivec4(imageSamples(src_img));\n"
			    "}\n"), NULL));
	bool ret = prog && init_fb(grid) &&
		randomize_image(img, 0);
	if (ret) {
		/* Verify that the generated image has the right
		 * number of samples, otherwise skip.
		 */
		GLint tex, samples;
		glGetIntegeri_v(GL_IMAGE_BINDING_NAME, 0, &tex);
		glBindTexture(img.target->target, tex);
		glGetTexLevelParameteriv(img.target->target, 0,
					 GL_TEXTURE_SAMPLES, &samples);
		if (samples != size.x) {
			glDeleteProgram(prog);
			return PIGLIT_SKIP;
		}
	}
	ret = ret &&
		set_uniform_int(prog, "src_img", 0) &&
		draw_grid(grid, prog) &&
		check(grid, img);

	glDeleteProgram(prog);
	return ret ? PIGLIT_PASS : PIGLIT_FAIL;
}

static bool
is_format_interesting(const struct image_format_info *format, bool override)
{
	switch(format->format)
	{
	case GL_R8:
	case GL_RGBA8:
	case GL_RGBA32F:
	case GL_RGBA16F:
	case GL_RGBA32I:
	case GL_RGBA16I:
	case GL_RGBA8I:
	case GL_RGBA32UI:
	case GL_RGBA16UI:
	case GL_RGBA8UI:
		return true;
	default:
		return override;
	}
}

static bool
is_stage_interesting(const struct image_stage_info *stage, bool override)
{
	switch(stage->stage)
	{
	case GL_FRAGMENT_SHADER:
	case GL_COMPUTE_SHADER:
		return true;
	default:
		return override;
	}
}

static void
test(const struct image_format_info *format,
     const struct image_target_info *target,
     const struct image_stage_info *stage,
     unsigned samples,
     enum piglit_result *status,
     bool slow)
{
	struct image_extent size = image_extent_for_target(
			target, 16, 96);
	size.x = samples;

	if (!is_format_interesting(format, slow) ||
	    !is_stage_interesting(stage, slow))
		return;

	enum piglit_result res = run_test(format, target, stage, size);

	piglit_report_subtest_result(res,
		"%s/%s/image%s samples test/%dx%dx%dx%d",
		format->name, stage->name, target->name,
		size.x, size.y, size.z,	size.w);
	if (res == PIGLIT_FAIL)
		*status = PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	const bool slow = (argc >= 2 && !strcmp(argv[1], "--slow"));
	enum piglit_result status = PIGLIT_PASS;
	const struct image_format_info *format;
	const struct image_target_info *target;
	const struct image_stage_info *stage;
	unsigned samples;

	piglit_require_extension("GL_ARB_shader_texture_image_samples");

	for (format = image_formats_load_store; format->format; ++format) {
		for (stage = image_stages(); stage->stage; ++stage) {
			for (target = image_targets(); target->name; ++target) {
				for (samples = 2; samples <= image_target_samples(target); samples += 2) {
					test(format, target,
					     stage, samples,
					     &status,
					     slow);
				}
			}
		}
	}

	piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
