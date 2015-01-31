/*
 * Copyright (C) 2014 Intel Corporation
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

/** @file level.c
 *
 * Test the binding of individual mipmap levels to an image unit by
 * dumping the whole accessible contents of an image to the
 * framebuffer and then checking that the observed values match the
 * bound mipmap level.  The same mipmap level is then overwritten by
 * the shader program after its contents have been read.
 */

#include "common.h"

/** Window width. */
#define W 16

/** Window height. */
#define H 96

/** Total number of pixels in the window and image. */
#define N (W * H)

/** Maximum number of mipmap levels. */
#define M 11

PIGLIT_GL_TEST_CONFIG_BEGIN

config.supports_gl_core_version = 32;

config.window_width = W;
config.window_height = H;
config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static bool
init_image(const struct image_info img, unsigned l)
{
        const unsigned num_levels = image_num_levels(img);
        uint32_t pixels[4 * N * M];
        unsigned i;

        for (i = 0; i < 4 * N * num_levels; ++i)
                pixels[i] = encode(img.format, i);

        return upload_image_levels(img, num_levels, l, 0, pixels);
}

static bool
check_fb(const struct grid_info grid, const struct image_info img, unsigned l)
{
        const unsigned offset = 4 * image_level_offset(img, l);
        const unsigned n = 4 * product(grid.size);
        uint32_t pixels_fb[4 * N], expect_fb[4 * N];
        unsigned i;

        if (!download_result(grid, pixels_fb))
                return false;

        for (i = 0; i < n; ++i) {
                /*
                 * The framebuffer contents should reflect level l of
                 * the image which is read by the shader program.
                 */
                expect_fb[i] = encode(grid.format, offset + i);
        }

        if (!check_pixels_v(image_info_for_grid(grid), pixels_fb, expect_fb)) {
                printf("  Source: framebuffer\n");
                return false;
        }

        return true;
}

static bool
check_img(const struct image_info img, unsigned l)
{
        const unsigned num_levels = image_num_levels(img);
        uint32_t pixels_img[4 * N * M], expect_img[4 * N];
        unsigned i, j;

        if (!download_image_levels(img, num_levels, 0, pixels_img))
                return false;

        for (j = 0; j < num_levels; ++j) {
                const struct image_info level_img = image_info_for_level(img, j);
                const unsigned offset = 4 * image_level_offset(img, j);
                const unsigned n = 4 * product(level_img.size);

                for (i = 0; i < n; ++i) {
                        if (j == l) {
                                /*
                                 * Level l should have been modified
                                 * by the shader.
                                 */
                                expect_img[i] = encode(img.format, 33);
                        } else {
                                /*
                                 * Other levels should have remained
                                 * unchanged.
                                 */
                                expect_img[i] = encode(img.format, offset + i);
                        }
                }

                if (!check_pixels_v(level_img, &pixels_img[offset],
                                    expect_img)) {
                        printf("  Source: image level %d\n", j);
                        return false;
                }
        }

        return true;
}

/**
 * Bind an individual level of a texture mipmap to an image unit, read
 * its contents and write back a different value to the same location.
 */
static bool
run_test(const struct image_target_info *target)
{
        const unsigned level = 3;
        const struct image_info img = image_info(
                target->target, GL_RGBA32F, W, H);
        const struct image_info level_img = image_info_for_level(img, level);
        const struct grid_info grid = {
                GL_FRAGMENT_SHADER_BIT, img.format,
                image_optimal_extent(level_img.size)
        };
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(image_hunk(level_img, ""),
                       hunk("uniform IMAGE_T img;\n"
                            "\n"
                            "GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        GRID_T v = imageLoad(img, IMAGE_ADDR(idx));\n"
                            "        imageStore(img, IMAGE_ADDR(idx), DATA_T(33));\n"
                            "        return v;\n"
                            "}\n"), NULL));
        bool ret = prog && init_fb(grid) &&
                init_image(img, level) &&
                set_uniform_int(prog, "img", 0) &&
                draw_grid(grid, prog) &&
                check_fb(grid, img, level) &&
                check_img(img, level);

        glDeleteProgram(prog);
        return ret;
}

void
piglit_init(int argc, char **argv)
{
        enum piglit_result status = PIGLIT_PASS;
        const struct image_target_info *target;

        piglit_require_extension("GL_ARB_shader_image_load_store");

        for (target = image_targets(); target->name; ++target) {
                if (image_target_mipmapping_dimensions(target))
                        subtest(&status, true, run_test(target),
                                "%s level binding test", target->name);
        }

        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_FAIL;
}
