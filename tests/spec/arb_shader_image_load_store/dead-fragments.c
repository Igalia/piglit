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

/** @file dead-fragments.c
 *
 * Quoting the ARB_shader_image_load_store extension:
 * "If a fragment shader is invoked to process fragments or samples
 *  not covered by a primitive being rasterized to facilitate the
 *  approximation of derivatives for texture lookups, stores and
 *  atomics have no effect."
 *
 * The purpose of this test is to check this assertion, as well as
 * that image stores and atomics have no effect after a fragment is
 * discarded.  Both tests are repeated for a few different built-in
 * functions.
 */

#include "common.h"

/** Window width. */
#define W 16

/** Window height. */
#define H 96

/** Total number of pixels in the window and image. */
#define N (W * H)

PIGLIT_GL_TEST_CONFIG_BEGIN

config.supports_gl_core_version = 32;

config.window_width = W;
config.window_height = H;
config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

struct image_op_info {
        /** Image built-in name. */
        const char *name;

        /** GLSL statement that invokes this image built-in. */
        const char *hunk;
};

static const struct image_op_info image_ops[] = {
        {
                "imageStore",
                "void inc(ivec2 idx) {\n"
                "        imageStore(img, IMAGE_ADDR(idx),"
                "                imageLoad(img, IMAGE_ADDR(idx)) + BASE_T(1));\n"
                "}\n"
        },
        {
                "imageAtomicAdd",
                "void inc(ivec2 idx) {\n"
                "        imageAtomicAdd(img, IMAGE_ADDR(idx), BASE_T(1));\n"
                "}\n"
        },
        {
                "imageAtomicMax",
                "void inc(ivec2 idx) {\n"
                "        imageAtomicMax(img, IMAGE_ADDR(idx),"
                "                imageLoad(img, IMAGE_ADDR(idx)).x + BASE_T(1));\n"
                "}\n"
        },
        {
                "imageAtomicExchange",
                "void inc(ivec2 idx) {\n"
                "        imageAtomicExchange(img, IMAGE_ADDR(idx),"
                "                imageLoad(img, IMAGE_ADDR(idx)).x + BASE_T(1));\n"
                "}\n"
        },
        {
                "imageAtomicCompSwap",
                "void inc(ivec2 idx) {\n"
                "        imageAtomicCompSwap(img, IMAGE_ADDR(idx),"
                "                imageLoad(img, IMAGE_ADDR(idx)).x,"
                "                imageLoad(img, IMAGE_ADDR(idx)).x + BASE_T(1));\n"
                "}\n"
        },
        { 0 }
};

static bool
init_image(const struct image_info img)
{
        uint32_t pixels[N];
        int i;

        for (i = 0; i < N; ++i)
                pixels[i] = i / W;

        return upload_image(img, 0, pixels);
}

static bool
check_discard(const struct grid_info grid, const struct image_info img,
              unsigned w, unsigned h)
{
        uint32_t pixels[H][W], expect[H][W];
        int i, j;

        for (i = 0; i < W; ++i)
                for (j = 0; j < H; ++j)
                        expect[j][i] = (i % 5 == 0 ? 0 : 1) + j;

        return download_image(img, 0, pixels[0]) &&
                check_pixels_v(img, pixels[0], expect[0]);
}

static bool
check_derivative(const struct grid_info grid, const struct image_info img,
                 unsigned w, unsigned h)
{
        uint32_t pixels_fb[H][W], expect_fb[H][W];
        uint32_t pixels_img[H][W], expect_img[H][W];
        int i, j;

        for (i = 0; i < W; ++i) {
                for (j = 0; j < H; ++j) {
                        expect_fb[j][i] = (j < h && i < w ? 1000 :
                                           encode(get_image_format(GL_R32F), 0.5));
                        expect_img[j][i] = (j < h && i < w ? 1 : 0) + j;
                }
        }

        if (!download_result(grid, pixels_fb[0]) ||
            !download_image(img, 0, pixels_img[0]))
                return false;

        if (!check_pixels_v(img, pixels_fb[0], expect_fb[0])) {
                printf("  Source: framebuffer\n");
                /*
                 * Purely informational check, we don't care what the
                 * result is as long as derivatives are being
                 * calculated, don't fail if the result doesn't equal
                 * the expected value as it's most likely an accuracy
                 * issue.
                 */
        }

        if (!check_pixels_v(img, pixels_img[0], expect_img[0])) {
                printf("  Source: image\n");
                return false;
        }

        return true;
}

static bool
run_test(const struct image_op_info *op,
         unsigned w, unsigned h,
         bool (*check)(const struct grid_info grid,
                       const struct image_info img,
                       unsigned w, unsigned h),
         const char *body)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_R32UI, W, H);
        const struct image_info img = image_info_for_grid(grid);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(image_hunk(img, ""),
                       hunk("uniform IMAGE_T img;\n"),
                       hunk(op->hunk),
                       hunk(body), NULL));
        bool ret = prog &&
                init_fb(grid) &&
                init_image(img) &&
                set_uniform_int(prog, "img", 0) &&
                draw_grid(set_grid_size(grid, w, h), prog) &&
                check(grid, img, w, h);

        glDeleteProgram(prog);
        return ret;
}

void
piglit_init(int argc, char **argv)
{
        enum piglit_result status = PIGLIT_PASS;
        const struct image_op_info *op;

        piglit_require_extension("GL_ARB_shader_image_load_store");

        for (op = image_ops; op->name; ++op) {
                /*
                 * Draw a rectangle discarding a subset of the
                 * fragments before inc() is run, then make sure that
                 * it had no effect for those fragments.
                 */
                subtest(&status, true,
                        run_test(op, W, H, check_discard,
                                 "GRID_T op(ivec2 idx, GRID_T x) {\n"
                                 "        if (idx.x % 5 == 0)\n"
                                 "                discard;\n"
                                 "        inc(idx);\n"
                                 "        return x;\n"
                                 "}\n"),
                        "%s/discard test", op->name);

                /*
                 * Draw a 1-pixel wide rectangle and make a derivative
                 * computation in the orthogonal direction to get the
                 * GPU to run fragment shader invocations located
                 * outside the primitive, then make sure that inc()
                 * had no effect for those fragments.
                 */
                subtest(&status, true,
                        run_test(op, W - 3, 1, check_derivative,
                                 "GRID_T op(ivec2 idx, GRID_T x) {\n"
                                 "        x = uvec4(1000 * dFdy(vec4("
                                 "                imageLoad(img, IMAGE_ADDR(idx)))));"
                                 "        inc(idx);\n"
                                 "        return x;\n"
                                 "}\n"),
                        "%s/derivative test", op->name);
        }

        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_FAIL;
}
