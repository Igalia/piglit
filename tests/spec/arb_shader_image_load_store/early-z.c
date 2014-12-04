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

/** @file early-z.c
 *
 * Test the interaction between early per-fragment tests, image access
 * and occlusion queries.  According to the spec:
 *
 * "When early per-fragment operations are enabled, the depth bounds
 *  test, stencil test, depth buffer test, and occlusion query sample
 *  counting operations are performed prior to fragment shader
 *  execution, and the stencil buffer, depth buffer, and occlusion
 *  query sample counts will be updated accordingly."
 *
 * "If a fragment is discarded during any of these operations, it will
 *  not be processed by any subsequent stage, including fragment
 *  shader execution."
 *
 * This checks several consequences of the quoted text, including that
 * the fragment shader is guaranteed not to be executed if the depth
 * test fails, that the depth value computed by the fragment shader is
 * ignored, and that fragments discarded during fragment shader
 * execution are counted by occlusion queries.  We also check that
 * when using normal (late) fragment tests image stores have an effect
 * regardless of the depth test results.
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
init_image(const struct image_info img)
{
        uint32_t pixels[4 * N];

        return init_pixels(img, pixels, 1, 0, 0, 1) &&
                upload_image(img, 0, pixels);
}

static bool
check_zb(double z)
{
        const struct image_info img = image_info(GL_TEXTURE_2D, GL_R32F, W, H);
        uint32_t pixels[N];

        glReadPixels(0, 0, W, H, GL_DEPTH_COMPONENT, GL_FLOAT, pixels);
        if (!piglit_check_gl_error(GL_NO_ERROR))
                return false;

        if (!check_pixels(img, pixels, z, 0, 0, 0)) {
                printf("  Source: depth buffer\n");
                return false;
        }

        return true;
}

static bool
check_img(const struct image_info img, double r, double g, double b, double a)
{
        uint32_t pixels[4 * N];

        if (!download_image(img, 0, pixels))
                return false;

        if (!check_pixels(img, pixels, r, g, b, a)) {
                printf("  Source: image\n");
                return false;
        }

        return true;
}

static bool
check_query(GLuint q, int expect)
{
        int value;

        glGetQueryObjectiv(q, GL_QUERY_RESULT, &value);
        if (value != expect) {
                printf("Query result\n  Expected: %d\n  Observed: %d\n",
                       expect, value);
                return false;
        }

        return piglit_check_gl_error(GL_NO_ERROR);
}

/**
 * Write to an image from the fragment shader using early or late
 * depth tests according to \a input_layout and check the results.
 */
static bool
run_test_image(const char *input_layout, GLenum depth_func,
               double expect_r, double expect_g, double expect_b,
               double expect_a, double expect_z)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_RGBA32F, W, H);
        const struct image_info img = image_info_for_grid(grid);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(hunk(input_layout),
                       image_hunk(img, ""),
                       hunk("uniform IMAGE_T img;\n"
                            "\n"
                            "GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        imageStore(img, IMAGE_ADDR(idx),"
                            "                   GRID_T(0, 1, 0, 1));\n"
                            "        gl_FragDepth = 1.0;\n"
                            "        return GRID_T(0, 1, 0, 1);\n"
                            "}\n"), NULL));
        bool ret = prog && init_fb(grid) &&
                init_image(img) &&
                set_uniform_int(prog, "img", 0);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(depth_func);

        ret &= draw_grid(grid, prog) &&
                check_img(img, expect_r, expect_g, expect_b, expect_a) &&
                check_zb(expect_z);

        glDeleteProgram(prog);
        return ret;
}

/**
 * Draw a grid of conditionally discarded fragments with early or late
 * depth tests according to \a input_layout and check the resulting
 * occlusion query sample count.
 */
static bool
run_test_query(const char *input_layout, GLenum depth_func,
               unsigned expect_samples_passed)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_RGBA32F, W, H);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(hunk(input_layout),
                       hunk("GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        if ((idx.x & 1) == 0)\n"
                            "                discard;\n"
                            "        gl_FragDepth = 1.0;\n"
                            "        return GRID_T(0, 1, 0, 1);\n"
                            "}\n"), NULL));
        bool ret = prog && init_fb(grid);
        GLenum q;

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(depth_func);
        glGenQueries(1, &q);

        glBeginQuery(GL_SAMPLES_PASSED, q);
        ret &= draw_grid(grid, prog);
        glEndQuery(GL_SAMPLES_PASSED);

        ret &= check_query(q, expect_samples_passed);

        glDeleteQueries(1, &q);
        glDeleteProgram(prog);
        return ret;
}

void
piglit_init(int argc, char **argv)
{
        enum piglit_result status = PIGLIT_PASS;

        piglit_require_extension("GL_ARB_shader_image_load_store");

        /*
         * Image stores should be executed, but the computed depth
         * value should have no effect if the early depth test
         * passes.
         */
        subtest(&status, true,
                run_test_image("layout(early_fragment_tests) in;\n",
                               GL_LEQUAL,
                               0.0, 1.0, 0.0, 1.0, 0.5),
                "image access test/early-z pass");

        /*
         * Image stores should have no effect if the early depth test
         * fails.
         */
        subtest(&status, true,
                run_test_image("layout(early_fragment_tests) in;\n",
                               GL_GREATER,
                               1.0, 0.0, 0.0, 1.0, 0.5),
                "image access test/early-z fail");

        /*
         * Image stores should be executed and the computed depth
         * value should be recorded if the late depth test passes.
         */
        subtest(&status, true,
                run_test_image("",
                               GL_GREATER,
                               0.0, 1.0, 0.0, 1.0, 1.0),
                "image access test/late-z pass");

        /*
         * Image stores should be executed, but the computed depth
         * value should have no effect if the late depth test
         * fails.
         */
        subtest(&status, true,
                run_test_image("",
                               GL_LEQUAL,
                               0.0, 1.0, 0.0, 1.0, 0.5),
                "image access test/late-z fail");

        /*
         * All fragments should be recorded in the occlusion query
         * with a passing early depth test even if some are discarded
         * further down the pipeline.
         */
        subtest(&status, true,
                run_test_query("layout(early_fragment_tests) in;\n",
                               GL_LEQUAL, N),
                "occlusion query test/early-z pass");

        /*
         * No fragments should be recorded in the occlusion query with
         * a failing early depth test.
         */
        subtest(&status, true,
                run_test_query("layout(early_fragment_tests) in;\n",
                               GL_GREATER, 0),
                "occlusion query test/early-z fail");

        /*
         * Only the fragments that don't call discard should be
         * recorded in the sample count with a passing late depth
         * test.
         */
        subtest(&status, true,
                run_test_query("",
                               GL_GREATER, N / 2),
                "occlusion query test/late-z pass");

        /*
         * No fragments should be recorded in the sample count with a
         * failing late depth test.
         */
        subtest(&status, true,
                run_test_query("",
                               GL_LEQUAL, 0),
                "occlusion query test/late-z fail");

        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_FAIL;
}
