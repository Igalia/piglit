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

/** @file max-images.c
 *
 * Check that images work as expected up to the reported limit of
 * image units and the per-shader and combined limit of image
 * uniforms.
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

/**
 * Return the total number of image uniforms used by the shader stages
 * specified in the \a stages bit-field.
 */
static unsigned
num_images_for_stages(const struct grid_info grid, unsigned stages)
{
        const struct image_stage_info *stage;
        unsigned n = 0;

        for (stage = image_stages(); stage->name; ++stage) {
                if (grid.stages & stages & stage->bit)
                        n += image_stage_max_images(stage);
        }


        return MIN2(max_combined_images(), n);
}

/**
 * Return the total number of image uniforms used by the specified
 * shader stage.
 */
static unsigned
num_images_for_stage(const struct grid_info grid,
                     const struct image_stage_info *stage)
{
        return num_images_for_stages(grid, (stage->bit << 1) - 1) -
                num_images_for_stages(grid, stage->bit - 1);
}

static bool
init_images(const struct image_info img)
{
        uint32_t pixels[N];
        int unit, i;

        for (unit = 0; unit < max_image_units(); ++unit) {
                for (i = 0; i < N; ++i)
                        pixels[i] = (i == unit ? 1 : 0);

                if (!upload_image(img, unit, pixels))
                        return false;
        }

        return true;
}

/**
 * Bind all image uniforms present in the program to the available
 * image units, re-using the same unit several times if necessary in
 * cyclical order.
 */
static bool
bind_images(const struct grid_info grid, GLuint prog)
{
        const unsigned m = max_image_units();
        const struct image_stage_info *stage;

        for (stage = image_stages(); stage->name; ++stage) {
                if (grid.stages & stage->bit) {
                        const unsigned first =
                                num_images_for_stages(grid, stage->bit - 1);
                        const unsigned n = num_images_for_stage(grid, stage);
                        const unsigned stage_idx = stage - image_stages();
                        int i;

                        for (i = 0; i < n; ++i) {
                                char *name = NULL;

                                asprintf(&name, "imgs_%d[%d]", stage_idx, i);

                                if (!set_uniform_int(prog, name,
                                                     (first + i) % m))
                                        return false;

                                free(name);
                        }
                }
        }

        return true;
}

static char *
stage_hunk(const struct grid_info grid,
           const struct image_stage_info *stage)
{
        char *s = NULL;

        asprintf(&s, "#define IMGS imgs_%d\n"
                 "#define NUM_IMGS %d\n",
                 (int)(stage - image_stages()),
                 num_images_for_stage(grid, stage));
        return s;
}

static char *
generate_source(const struct grid_info grid,
                const struct image_info img, GLuint s)
{
        const struct image_stage_info *stage = get_image_stage(s);

        if (stage && num_images_for_stage(grid, stage)) {
                /*
                 * Sum up the values read from corresponding locations
                 * of all bound image uniforms.
                 */
                return concat(stage_hunk(grid, stage),
                              image_hunk(img, ""),
                              hunk("uniform IMAGE_T IMGS[NUM_IMGS];\n"
                                   "\n"
                                   "GRID_T op(ivec2 idx, GRID_T x) {\n"
                                   "        int i;\n"
                                   "\n"
                                   "        for (i = 0; i < NUM_IMGS; ++i)\n"
                                   "                x += imageLoad(IMGS[i], IMAGE_ADDR(idx));\n"
                                   "\n"
                                   "        return x;\n"
                                   "}\n"), NULL);

        } else {
                return NULL;
        }
}

static bool
check(const struct grid_info grid, const struct image_info img)
{
        const int n = num_images_for_stages(grid, ~0);
        const int m = max_image_units();
        uint32_t pixels[N], expect[N];
        int i;

        for (i = 0; i < N; ++i) {
                if (i < m) {
                        /*
                         * The sum at this location is just the number
                         * of times that the image with index i was
                         * bound to the pipeline.
                         */
                        expect[i] = (n - i + m - 1) / m;
                } else {
                        /*
                         * No image has a non-zero value at this
                         * location, so the sum is zero.
                         */
                        expect[i] = 0;
                }
        }

        return download_result(grid, pixels) &&
                check_pixels_v(img, pixels, expect);
}

static bool
run_test(GLbitfield shaders)
{
        const struct grid_info grid = {
                shaders,
                get_image_format(GL_R32UI),
                { W, H, 1, 1 }
        };
        const struct image_info img = image_info_for_grid(grid);
        GLuint prog = generate_program(
                grid,
                GL_VERTEX_SHADER,
                generate_source(grid, img, GL_VERTEX_SHADER),
                GL_TESS_CONTROL_SHADER,
                generate_source(grid, img, GL_TESS_CONTROL_SHADER),
                GL_TESS_EVALUATION_SHADER,
                generate_source(grid, img, GL_TESS_EVALUATION_SHADER),
                GL_GEOMETRY_SHADER,
                generate_source(grid, img, GL_GEOMETRY_SHADER),
                GL_FRAGMENT_SHADER,
                generate_source(grid, img, GL_FRAGMENT_SHADER),
                GL_COMPUTE_SHADER,
                generate_source(grid, img, GL_COMPUTE_SHADER));
        bool ret = prog && init_fb(grid) &&
                init_images(img) &&
                bind_images(grid, prog) &&
                draw_grid(grid, prog) &&
                check(grid, img);

        glDeleteProgram(prog);
        return ret;
}

void
piglit_init(int argc, char **argv)
{
        enum piglit_result status = PIGLIT_PASS;
        const struct image_stage_info *stage;

        piglit_require_extension("GL_ARB_shader_image_load_store");

        for (stage = image_stages(); stage->name; ++stage) {
                subtest(&status, true,
                        run_test(stage->bit),
                        "%s shader max image uniforms test",
                        stage->name);
        }

        subtest(&status, true,
                run_test((get_image_stage(GL_VERTEX_SHADER) ?
                          GL_VERTEX_SHADER_BIT : 0) |
                         (get_image_stage(GL_TESS_CONTROL_SHADER) ?
                          GL_TESS_CONTROL_SHADER_BIT : 0) |
                         (get_image_stage(GL_TESS_EVALUATION_SHADER) ?
                          GL_TESS_EVALUATION_SHADER_BIT : 0) |
                         (get_image_stage(GL_GEOMETRY_SHADER) ?
                          GL_GEOMETRY_SHADER_BIT : 0) |
                         GL_FRAGMENT_SHADER_BIT),
                "Combined max image uniforms test");

        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_FAIL;
}
