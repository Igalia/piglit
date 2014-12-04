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

/** @file restrict.c
 *
 * Test if the implementation is incorrectly assuming that different
 * image uniforms point to disjoint locations in memory, which could lead
 * to code reordering and access coalescing that could break valid GLSL
 * programs.  This is done by repeatedly reading and writing to an image
 * through two different uniforms that alias the same image in a way that
 * is likely to misrender if the implementation is coalescing loads.
 *
 * The same test is repeated with the "restrict" keyword which
 * explicitly allows the implementation to make such assumptions.  The
 * rendering results from this test are ignored as it's only useful to
 * test the "restrict" keyword and to find out if the implementation
 * is making such transformations since otherwise the main test is not
 * meaningful.
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

struct image_qualifier_info {
        /** Test name. */
        const char *name;

        /** Image qualifier keyword. */
        const char *qualifier;

        /** Informative "control" test whose result is ignored. */
        bool control_test;
};

const struct image_qualifier_info image_qualifiers[] = {
        { "no qualifier", "", false },
        { "restrict qualifier", "restrict", true },
        { 0 }
};

char *
qualifier_hunk(const struct image_qualifier_info *qual)
{
        char *s = NULL;

        asprintf(&s, "#define IMAGE_Q %s\n", qual->qualifier);
        return s;
}

static bool
init_image(const struct image_info img)
{
        uint32_t pixels[4 * N];

        return init_pixels(img, pixels, 1, 0, 0, 0) &&
                upload_image(img, 0, pixels);
}

static bool
check(const struct image_info img)
{
        uint32_t pixels[N];
        uint32_t expect[N];
        int i;

        for (i = 0; i < N; ++i)
                expect[i] = (i > W ? 2 : 1) + (i % 2 ? -1 : 1);

        return download_image(img, 0, pixels) &&
                check_pixels_v(img, pixels, expect);
}

static bool
run_test(const struct image_qualifier_info *qual)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_R32UI, W, H);
        const struct image_info img =
                image_info(GL_TEXTURE_1D, GL_R32UI, W, H);
        GLuint prog = generate_program(
                grid,
                /**
                 * Write to consecutive locations of an image using a
                 * the value read from a fixed location of a different
                 * image uniform which aliases the first image.  If
                 * the implementation incorrectly coalesces repeated
                 * loads from the fixed location the results of the
                 * test will be altered.
                 */
                GL_FRAGMENT_SHADER,
                concat(qualifier_hunk(qual),
                       image_hunk(img, ""),
                       hunk("IMAGE_Q uniform IMAGE_T src_img;\n"
                            "IMAGE_Q uniform IMAGE_T dst_img;\n"
                            "\n"
                            "GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        int i;\n"
                            "\n"
                            "        for (i = 0; i < N / 2; ++i) {\n"
                            "                imageStore(dst_img, 2 * i,"
                            "                           imageLoad(src_img, W) + 1u);\n"
                            "                imageStore(dst_img, 2 * i + 1,"
                            "                           imageLoad(src_img, W) - 1u);\n"
                            "        }\n"
                            "\n"
                            "        return x;\n"
                            "}\n"), NULL));
        bool ret = prog &&
                init_fb(grid) &&
                init_image(img) &&
                set_uniform_int(prog, "src_img", 0) &&
                set_uniform_int(prog, "dst_img", 0) &&
                draw_grid(set_grid_size(grid, 1, 1), prog) &&
                (check(img) || qual->control_test);

        glDeleteProgram(prog);
        return ret;
}

void
piglit_init(int argc, char **argv)
{
        const struct image_qualifier_info *qual;
        enum piglit_result status = PIGLIT_PASS;

        piglit_require_extension("GL_ARB_shader_image_load_store");

        for (qual = image_qualifiers; qual->name; ++qual) {
                subtest(&status, true, run_test(qual),
                        "%s image aliasing test", qual->name);
        }


        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_FAIL;
}
