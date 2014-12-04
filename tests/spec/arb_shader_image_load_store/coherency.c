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

/** @file coherency.c
 *
 * Images declared as "coherent" or "volatile" give certain guarantees
 * regarding the visibility of memory writes to other shader invocations
 * in the pipeline.  This test checks the following assertion of the
 * spec:
 * "When reading a variable declared as 'coherent', the values returned
 *  will reflect the results of previously completed writes performed by
 *  other shader invocations.  When writing a variable declared as
 *  'coherent', the values written will be reflected in subsequent
 *  coherent reads performed by other shader invocations."
 *
 * Together with:
 * "When executing a shader whose inputs are generated from a previous
 *  programmable stage, the shader invocations from the previous stage
 *  are guaranteed to have executed far enough to generate final values
 *  for all next-stage inputs."
 *
 * In order to test this we bind two shader programs at different
 * execution stages of the pipeline and check that the values written to
 * an image from the first stage are visible when the same primitive is
 * dispatched at the second stage.  This is repeated for all combinations
 * of dependent shader stages (what necessarily excludes the compute
 * shader), for the "coherent" and "volatile" qualifiers (the latter is
 * supposed to give stricter ordering guarantees), and for different
 * execution sizes between 4x4 and 1024x1024 to account for
 * implementations with varying levels of parallelism and with caches of
 * different sizes.
 *
 * Unless running in "quick" mode a series of control tests is
 * executed repeating the same process without any memory qualifiers.
 * This is useful to see if the cache is being hit since otherwise the
 * main test is not meaningful.  The control test always passes as it
 * is expected to misrender.
 */

#include "common.h"

/** Maximum image width. */
#define L 1024

/** Maximum number of pixels. */
#define N (L * L)

PIGLIT_GL_TEST_CONFIG_BEGIN

config.supports_gl_core_version = 32;

config.window_width = L;
config.window_height = L;
config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

struct image_qualifier_info {
        /** Test name. */
        const char *name;

        /** Image qualifier keyword. */
        const char *qualifier;

        /** Informative "control" test with no memory qualifiers whose
         * result is ignored. */
        bool control_test;
};

const struct image_qualifier_info image_qualifiers[] = {
        { "control", "", true },
        { "'coherent' qualifier", "coherent", false },
        { "'volatile' qualifier", "volatile", false },
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
        static uint32_t pixels[N][4];

        return init_pixels(img, pixels[0], 99, 99, 99, 99) &&
                upload_image(img, 0, pixels[0]);
}

static bool
check(const struct grid_info grid, const struct image_info img)
{
        static uint32_t pixels[N][4];

        return download_result(grid, pixels[0]) &&
                check_pixels(img, pixels[0], 33, 33, 33, 33);
}

static bool
run_test(const struct image_qualifier_info *qual,
         const struct image_stage_info *stage_w,
         const struct image_stage_info *stage_r,
         unsigned l)
{
        const struct grid_info grid = {
                stage_w->bit | stage_r->bit,
                get_image_format(GL_RGBA32UI),
                { l, l, 1, 1 }
        };
        const struct image_info img = image_info_for_grid(grid);
        GLuint prog = generate_program(
                grid,
                /*
                 * Write (11, 22, 33, 44) to some location on the
                 * image from the write stage.
                 */
                stage_w->stage,
                concat(qualifier_hunk(qual),
                       image_hunk(img, ""),
                       hunk("IMAGE_Q uniform IMAGE_T img;\n"
                            "\n"
                            "GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "       imageStore(img, idx, DATA_T(11, 22, 33, 44));"
                            "       return x;"
                            "}\n"), NULL),
                /*
                 * The same location will read back the expected value
                 * if image access is coherent, as the shader inputs
                 * of the read stage are dependent on the outputs of
                 * the write stage and consequently they are
                 * guaranteed to be executed sequentially.
                 */
                stage_r->stage,
                concat(qualifier_hunk(qual),
                       image_hunk(img, ""),
                       hunk("IMAGE_Q uniform IMAGE_T img;\n"
                            "\n"
                            "GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "       DATA_T v = imageLoad(img, idx);"
                            "       if (v == DATA_T(11, 22, 33, 44))"
                            "             return GRID_T(33, 33, 33, 33);"
                            "       else"
                            "             return GRID_T(77, 77, 77, 77);"
                            "}\n"), NULL));
        bool ret = prog &&
                init_fb(grid) &&
                init_image(img) &&
                set_uniform_int(prog, "img", 0) &&
                draw_grid(grid, prog) &&
                (check(grid, img) || qual->control_test);

        glDeleteProgram(prog);
        return ret;
}

void
piglit_init(int argc, char **argv)
{
        const bool quick = (argc >= 2 && !strcmp(argv[1], "--quick"));
        const struct image_qualifier_info *qual;
        const struct image_stage_info *stage_w;
        const struct image_stage_info *stage_r;
        enum piglit_result status = PIGLIT_PASS;
        unsigned l;

        piglit_require_extension("GL_ARB_shader_image_load_store");

        for (l = 4; l <= L; l *= (quick ? 8 : 2)) {
                for (qual = image_qualifiers; qual->name; ++qual) {
                        if (quick && qual->control_test)
                                continue;

                        /*
                         * Loop for each pair of shader stages
                         * skipping the compute shader: "coherent"
                         * gives no useful guarantees in that case
                         * since its execution ordering is undefined
                         * with respect to the other shader stages.
                         */
                        for (stage_w = image_stages();
                             stage_w->stage; ++stage_w) {
                                for (stage_r = stage_w + 1;
                                     stage_r->stage; ++stage_r) {
                                        if (stage_w->stage == GL_COMPUTE_SHADER ||
                                            stage_r->stage == GL_COMPUTE_SHADER)
                                                continue;

                                      subtest(&status, true,
                                                run_test(qual, stage_w, stage_r, l),
                                                "%s-%s shader/%s coherency test/%dx%d",
                                                stage_w->name, stage_r->name,
                                                qual->name, l, l);
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
