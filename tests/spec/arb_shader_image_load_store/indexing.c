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

/** @file indexing.c
 *
 * Test that image array indexing gives the expected results.  The
 * original ARB_shader_image_load_store is rather vague in this
 * regard, but the GLSL 4.2 specification states that:
 *
 * "When aggregated into arrays within a shader, images can only be
 *  indexed with a dynamically uniform integral expression, otherwise
 *  results are undefined."
 *
 * Which means that we can only check indexing with dynamically
 * uniform expressions, i.e. expressions that are invariant for all
 * shader invocations in which they are evaluated.
 */

#include "common.h"

/** Window width. */
#define W 16

/** Window height. */
#define H 96

/** Total number of pixels in the window and images. */
#define N (W * H)

PIGLIT_GL_TEST_CONFIG_BEGIN

config.supports_gl_core_version = 32;

config.window_width = W;
config.window_height = H;
config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static bool
init_images(const struct image_info img, GLuint prog)
{
        uint32_t pixels[H][W];
        int i, j, unit;

        for (unit = 0; unit < 8; ++unit) {
                char *name = NULL;

                for (i = 0; i < W; ++i)
                        for (j = 0; j < H; ++j)
                                pixels[j][i] = (i + j + unit) % 16;

                if (!upload_image(img, unit, pixels[0]))
                        return false;

                asprintf(&name, "imgs[%d]", unit);
                set_uniform_int(prog, name, unit);
                free(name);
        }

        return true;
}

static bool
check(const struct grid_info grid, unsigned u)
{
        uint32_t pixels[H][W];
        uint32_t expect[H][W];
        int i, j, unit;

        for (i = 0; i < W; ++i) {
                for (j = 0; j < H; ++j) {
                        if (i % 2 == j % 3) {
                                /* Skipped invocation. */
                                expect[j][i] = 0xdeadcafe;
                        } else {
                                /* Active invocation. */
                                unsigned x = 0;

                                for (unit = 0; unit < 8; ++unit)
                                        x = (x << 4 |
                                             ((i + j + (unit + u) % 8) % 16));

                                expect[j][i] = x;
                        }
                }
        }

        return download_result(grid, pixels[0]) &&
                check_pixels_v(image_info_for_grid(grid),
                               pixels[0], expect[0]);
}

/**
 * Discard a number of fragments and then load elements from an array
 * of images using dynamically uniform indices.
 */
static bool
run_test(const struct image_stage_info *stage)
{
        const struct grid_info grid =
                grid_info(stage->stage, GL_R32UI, W, H);
        const struct image_info img = image_info_for_grid(grid);
        GLuint prog = generate_program(
                grid, stage->stage ,
                concat(image_hunk(img, ""),
                       hunk("uniform int u;\n"
                            "uniform IMAGE_T imgs[8];\n"
                            "\n"
                            "GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        int i;\n"
                            "\n"
                            "        if (idx.x % 2 == idx.y % 3)\n"
                            "                return GRID_T(0xdeadcafeu);\n"
                            "\n"
                            "        for (i = 0; i < 8; ++i) {\n"
                            "                x.x = (x.x << 4 |"
                            "                       imageLoad(imgs[(i + u) % 8],"
                            "                                 IMAGE_ADDR(idx)).x);\n"
                            "        }\n"
                            "\n"
                            "        return x;\n"
                            "}\n"), NULL));
        bool ret = prog &&
                init_fb(grid) &&
                init_images(img, prog) &&
                set_uniform_int(prog, "u", 5) &&
                draw_grid(grid, prog) &&
                check(grid, 5);

        glDeleteProgram(prog);
        return ret;
}

void
piglit_init(int argc, char **argv)
{
        enum piglit_result status = PIGLIT_PASS;
        const struct image_stage_info *stage;

        piglit_require_extension("GL_ARB_shader_image_load_store");

        for (stage = image_stages(); stage->name; ++stage)
                subtest(&status, true, run_test(stage),
                        "%s shader/dynamically uniform indexing test",
                        stage->name);

        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_FAIL;
}
