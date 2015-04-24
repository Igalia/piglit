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

/** @file builtin.c
 *
 * Tests GLSL's imageSize builtin. The code is highly-based on
 * image_load_store's max-size test from Francisco Jerez. It also uses the
 * grid framework he introduced.
 *
 * From GL_ARB_shader_image_size's spec:
 *
 * "Including the following line in a shader can be used to control the
 * language features described in this extension:
 *
 *      #extension GL_ARB_shader_image_size
 *
 * A new preprocessor #define is added to the OpenGL Shading Language:
 *
 *      #define GL_ARB_shader_image_size 1
 *
 * Add to section 8.11 "Image Functions"
 *
 * Syntax:
 *      int imageSize(gimage1D image)
 *      ivec2 imageSize(gimage2D image)
 *      ivec3 imageSize(gimage3D image)
 *      ivec2 imageSize(gimageCube image)
 *      ivec3 imageSize(gimageCubeArray image)
 *      ivec2 imageSize(gimageRect image)
 *      ivec2 imageSize(gimage1DArray image)
 *      ivec3 imageSize(gimage2DArray image)
 *      int imageSize(gimageBuffer image)
 *      ivec2 imageSize(gimage2DMS image)
 *      ivec3 imageSize(gimage2DMSArray image)
 *
 * Description:
 *
 *      Returns the dimensions of the image or images bound to <image>.  For
 *      arrayed images, the last component of the return value will hold the
 *      size of the array.  Cube images return the dimensions of one face, and
 *      number of cubes in the cube map array, if arrayed."
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
	const unsigned n = image_num_components(img.format) * product(img.size);
	uint32_t *pixels = malloc(sizeof(uint32_t) * n);
	bool ret;

	ret = download_result(grid, pixels);

	if (img_src.target->target == GL_TEXTURE_CUBE_MAP_ARRAY) {
		/* Unlike image-load-store's size (exported by the framework
		 * used for this test), image-size reports the third
		 * coordinate of cubemap arrays as the number of cubes and not
		 * the number of faces. Correct for this by dividing by 6.
		 */
		ret &= check_pixels(img, pixels, img_src.size.x, img_src.size.y,
			     img_src.size.z / 6.0, img_src.size.w);
	} else if (image_target_samples(img_src.target) > 1) {
		/* Unlike image-load-store's size (exported by the framework
		 * used for this test), image-size does not report the sample
		 * count as the first component. Let's fix this by shiftling
		 * left the components.
		 */
		ret &= check_pixels(img, pixels, img_src.size.y, img_src.size.z,
					     img_src.size.w, 1.0);
	} else  {
		ret &= check_pixels(img, pixels, img_src.size.x, img_src.size.y,
		                    img_src.size.z, img_src.size.w);
	}

	free(pixels);
	return ret;
}

static bool
run_test(const struct image_target_info *target,
	 const struct image_extent size)
{
	const struct grid_info grid = grid_info(GL_FRAGMENT_SHADER, GL_RGBA32I,
						16, 16);
	const struct image_info img = {
		target, grid.format, size,
		image_format_epsilon(grid.format)
	};
	GLuint prog = generate_program(
		grid, GL_FRAGMENT_SHADER,
		concat(hunk("#extension GL_ARB_shader_image_size : enable\n"),
		       image_hunk(img, ""),
		       hunk("readonly uniform IMAGE_T src_img;\n"
			    "\n"
			    "GRID_T op(ivec2 idx, GRID_T x) {\n"
			    "        return ivec4(imageSize(src_img), ivec3(1));\n"
			    "}\n"), NULL));
	bool ret = prog && init_fb(grid) &&
		randomize_image(img, 0) &&
		set_uniform_int(prog, "src_img", 0) &&
		draw_grid(grid, prog) &&
		check(grid, img);

	glDeleteProgram(prog);
	return ret;
}

static struct image_extent
get_test_extent(const struct image_target_info *target, unsigned d)
{
	const struct image_extent ls = image_target_limits(target);
	const unsigned high = ~0, low = 8;
	struct image_extent ext;
	int i;

	for (i = 0; i < 4; ++i)
		set_idx(ext, i, MIN2(get_idx(ls, i), (i == d ? high : low)));

	if (target->target == GL_TEXTURE_CUBE_MAP ||
	    target->target == GL_TEXTURE_CUBE_MAP_ARRAY) {
		/* Cube maps have to be square and the number of faces
		 * should be a multiple of six. */
		ext.y = ext.x;
		ext.z = 6 * MAX2(ext.z / 6, 1);

	} else if (image_target_samples(target) > 1) {
		/* Use the maximum number of samples to keep things
		 * interesting. */
		ext.x = image_target_samples(target);
	}

	return ext;
}

static bool
should_test_dimension(const struct image_target_info *target, int d)
{
	const struct image_extent ls = image_target_limits(target);

	return get_idx(ls, d) > 1 &&
		/* Skip second cube map dimension as faces have to be
		 * square. */
		!(target->target == GL_TEXTURE_CUBE_MAP && d >= 1) &&
		!(target->target == GL_TEXTURE_CUBE_MAP_ARRAY && d == 1) &&
		/* Skip sample dimension. */
		!(image_target_samples(target) > 1 && d == 0);
}

static bool
is_test_reasonable(bool quick, const struct image_extent size)
{
	/* Set an arbitrary limit on the number of texels so the test
	 * doesn't take forever. */
	return product(size) < (quick ? 4 : 64) * 1024 * 1024;
}

void
piglit_init(int argc, char **argv)
{
	const bool quick = (argc >= 2 && !strcmp(argv[1], "--quick"));
	enum piglit_result status = PIGLIT_PASS;
	const struct image_target_info *target;
	const struct image_stage_info *stage;
	int d;

	/* The spec of the extension says we should require GL 4.2 but let's
	 * just request GL_ARB_shader_image_size which will in turn require
	 * GL_ARB_shader_image_load_store which should be sufficient.
	 */
	piglit_require_extension("GL_ARB_shader_image_size");


	for (stage = image_stages(); stage->stage; ++stage) {
		for (target = image_targets(); target->name; ++target) {
			for (d = 0; d < 4; ++d) {
				if (should_test_dimension(target, d)) {
					const struct image_extent size =
						get_test_extent(target, d);

					subtest(&status,
						is_test_reasonable(quick, size),
						run_test(target, size),
						"%s/image%s max size test/"
						"%dx%dx%dx%d", stage->name,
						target->name,
						size.x, size.y, size.z, size.w);
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
