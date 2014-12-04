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

/** @file unused.c
 *
 * Test that atomic ops with unused return value still have the
 * expected effect (which implies that they aren't being optimized out
 * accidentally by the compiler).
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

static bool
init_image(const struct image_info img, uint32_t v)
{
        uint32_t pixels[N];

        return init_pixels(img, pixels, v, 0, 0, 0) &&
                upload_image(img, 0, pixels);
}

static bool
check(const struct image_info img, uint32_t v)
{
        uint32_t pixels[N];

        return download_image(img, 0, pixels) &&
                check_pixels(img, pixels, v, 0, 0, 0);
}

/**
 * Test skeleton: Init image to \a init_value, run the provided shader
 * \a op and check that the resulting image pixels equal \a
 * check_value.
 */
static bool
run_test(uint32_t init_value, uint32_t check_value,
         const char *op)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_R32UI, W, H);
        const struct image_info img = image_info_for_grid(grid);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(image_hunk(img, ""),
                       hunk("uniform IMAGE_T img;\n"),
                       hunk(op), NULL));
        bool ret = prog &&
                init_fb(grid) &&
                init_image(img, init_value) &&
                set_uniform_int(prog, "img", 0) &&
                draw_grid(grid, prog) &&
                check(img, check_value);

        glDeleteProgram(prog);
        return ret;
}

void
piglit_init(int argc, char **argv)
{
        enum piglit_result status = PIGLIT_PASS;

        piglit_require_extension("GL_ARB_shader_image_load_store");

        subtest(&status, true,
                run_test(0, 33,
                         "GRID_T op(ivec2 idx, GRID_T x) {\n"
                         "        imageAtomicAdd(img, IMAGE_ADDR(idx), BASE_T(33));\n"
                         "        return x;\n"
                         "}\n"),
                "imageAtomicAdd");

        subtest(&status, true,
                run_test(0xffffffff, 33,
                         "GRID_T op(ivec2 idx, GRID_T x) {\n"
                         "        imageAtomicMin(img, IMAGE_ADDR(idx), BASE_T(33));\n"
                         "        return x;\n"
                         "}\n"),
                "imageAtomicMin");

        subtest(&status, true,
                run_test(0, 33,
                         "GRID_T op(ivec2 idx, GRID_T x) {\n"
                         "        imageAtomicMax(img, IMAGE_ADDR(idx), BASE_T(33));\n"
                         "        return x;\n"
                         "}\n"),
                "imageAtomicMax");

        subtest(&status, true,
                run_test(0xffffffff, 33,
                         "GRID_T op(ivec2 idx, GRID_T x) {\n"
                         "        imageAtomicAnd(img, IMAGE_ADDR(idx), BASE_T(33));\n"
                         "        return x;\n"
                         "}\n"),
                "imageAtomicAnd");

        subtest(&status, true,
                run_test(0, 33,
                         "GRID_T op(ivec2 idx, GRID_T x) {\n"
                         "        imageAtomicOr(img, IMAGE_ADDR(idx), BASE_T(33));\n"
                         "        return x;\n"
                         "}\n"),
                "imageAtomicOr");

        subtest(&status, true,
                run_test(0, 33,
                         "GRID_T op(ivec2 idx, GRID_T x) {\n"
                         "        imageAtomicXor(img, IMAGE_ADDR(idx), BASE_T(33));\n"
                         "        return x;\n"
                         "}\n"),
                "imageAtomicXor");

        subtest(&status, true,
                run_test(0, 33,
                         "GRID_T op(ivec2 idx, GRID_T x) {\n"
                         "        imageAtomicExchange(img, IMAGE_ADDR(idx),"
                         "                            BASE_T(33));\n"
                         "        return x;\n"
                         "}\n"),
                "imageAtomicExchange");

        subtest(&status, true,
                run_test(0, 33,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        imageAtomicCompSwap(img, IMAGE_ADDR(idx),"
                "                            BASE_T(0), BASE_T(33));\n"
                "        return x;\n"
                         "}\n"),
                "imageAtomicCompSwap");

        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_FAIL;
}
