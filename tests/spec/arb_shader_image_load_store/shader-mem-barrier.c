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

/** @file shader-mem-barrier.c
 *
 * Test that the memoryBarrier() GLSL built-in effectively serializes
 * memory transactions initiated after and before the call.  This is
 * done by having a number of threads write to separate locations in a
 * loop serializing each pair of writes with memoryBarrier() while the
 * remaining threads monitor the evolution of the same memory
 * locations until an inconsistency is observed or the test runs to
 * completion.
 *
 * The test is repeated for the "volatile" qualifier with no barriers,
 * for all execution stages and for different relative arrangements of
 * producer and monitor threads to account for implementations with
 * varying levels of parallelism and with caches of different sizes.
 *
 * Unless running in "quick" mode a series of control tests is
 * executed which disables memory barriers in order to make sure that
 * the test is meaningful.  The control test always passes as it is
 * expected to misrender.
 */

#include "common.h"

/** Window width. */
#define W 256

/** Window height. */
#define H 16

/** Total number of pixels in the image. */
#define N (W * H)

/** Maximum modulus. */
#define K 128

PIGLIT_GL_TEST_CONFIG_BEGIN

config.supports_gl_core_version = 32;

config.window_width = W;
config.window_height = H;
config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

struct image_test_info {
        /** Test name. */
        const char *name;

        /** Image qualifier keyword. */
        const char *qualifier;

        /** Memory barrier built-in call. */
        const char *barrier;

        /** Informative "control" test likely to fail whose result is
         * ignored. */
        bool control_test;
};

const struct image_test_info image_tests[] = {
        { "control", "", "", true },
        { "'coherent' qualifier", "coherent", "memoryBarrier()", false },
        { "'volatile' qualifier", "volatile", "", false },
        { 0 }
};

char *
test_hunk(const struct image_test_info *test, unsigned k)
{
        char *s = NULL;

        asprintf(&s, "#define IMAGE_Q %s\n"
                 "#define MEMORY_BARRIER() %s\n"
                 "#define K %d\n",
                 test->qualifier, test->barrier, k);
        return s;
}

static bool
init_image(const struct image_info img)
{
        uint32_t pixels[N] = { 0 };

        return upload_image(img, 0, pixels);
}

static bool
check(const struct grid_info grid)
{
        uint32_t pixels[N];

        return download_result(grid, pixels) &&
                check_pixels(image_info_for_grid(grid), pixels, 33, 0, 0, 0);
}

static bool
run_test(const struct image_test_info *test,
         const struct image_stage_info *stage,
         unsigned k)
{
        const struct grid_info grid =
                grid_info(stage->stage, GL_R32UI, W, H);
        const struct image_info img = image_info_for_grid(grid);
        GLuint prog = generate_program(
                grid, stage->stage,
                concat(test_hunk(test, k),
                       image_hunk(img, ""),
                       hunk("IMAGE_Q uniform IMAGE_T img;\n"
                            "\n"
                            "GRID_T op(ivec2 idx, GRID_T arg) {\n"
                            "       /*\n"
                            "        * Shader invocations are classified into producers\n"
                            "        * (check==false) and consumers (check==true), each\n"
                            "        * producer being paired with a consumer K threads to\n"
                            "        * the right.\n"
                            "        */\n"
                            "       bool check = ((idx.x / K) % 2 == 1);\n"
                            "       int x = (idx.x % K) + (idx.x / (2 * K)) * (2 * K);\n"
                            "       int i, n = 1000;\n"
                            "\n"
                            "       if (check) {\n"
                            "              /*\n"
                            "               * Consumer: Monitor the evolution of a pair of\n"
                            "               * image locations until the test runs to\n"
                            "               * completion or an inconsistency is observed.\n"
                            "               */\n"
                            "              for (i = 0; i < n; ++i) {\n"
                            "                     uint u, v;\n"
                            "\n"
                            "                     v = imageLoad(img, ivec2(x, idx.y)).x;\n"
                            "                     MEMORY_BARRIER();\n"
                            "                     u = imageLoad(img, ivec2(x + K, idx.y)).x;\n"
                            "\n"
                            "                     if (u < v)\n"
                            "                             /* Fail. */\n"
                            "                             return GRID_T(v << 16 | u, 0, 0, 1);\n"
                            "             }\n"
                            "       } else {\n"
                            "              /*\n"
                            "               * Producer: Update the same pair of image locations\n"
                            "               * sequentially with increasing values ordering the\n"
                            "               * stores with a barrier.\n"
                            "               */\n"
                            "              for (i = 0; i < n; ++i) {\n"
                            "                     imageStore(img, ivec2(x + K, idx.y), DATA_T(i));\n"
                            "                     MEMORY_BARRIER();\n"
                            "                     imageStore(img, ivec2(x, idx.y), DATA_T(i));\n"
                            "              }\n"
                            "       }\n"
                            "\n"
                            "       /* Success. */\n"
                            "       return GRID_T(33, 0, 0, 1);\n"
                            "}\n"), NULL));
        bool ret = prog &&
                init_fb(grid) &&
                init_image(img) &&
                set_uniform_int(prog, "img", 0) &&
                draw_grid(grid, prog) &&
                (check(grid) || test->control_test);

        glDeleteProgram(prog);
        return ret;
}

void
piglit_init(int argc, char **argv)
{
        const bool quick = (argc >= 2 && !strcmp(argv[1], "--quick"));
        const struct image_test_info *test;
        const struct image_stage_info *stage;
        enum piglit_result status = PIGLIT_PASS;
        unsigned k;

        piglit_require_extension("GL_ARB_shader_image_load_store");

        for (k = 1; k <= K; k *= (quick ? 4 : 2)) {
                for (test = image_tests; test->name; ++test) {
                        if (quick && test->control_test)
                                continue;

                        for (stage = image_stages(); stage->name; ++stage) {
                                subtest(&status, true,
                                        run_test(test, stage, k),
                                        "%s shader/%s memory barrier test"
                                        "/modulus=%d",
                                        stage->name, test->name, k);
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
