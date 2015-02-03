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

/** @file max-size.c
 *
 * Check that images work as expected up to the maximum texture size
 * reported for each target.
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

config.supports_gl_core_version = 32;

config.window_width = 1;
config.window_height = 1;
config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static bool
init_image(const struct image_info img, unsigned unit)
{
        const unsigned n = image_num_components(img.format) * product(img.size);
        uint32_t *pixels = malloc(sizeof(uint32_t) * n);
        bool ret;
        int i;

        for (i = 0; i < n; ++i)
                pixels[i] = (unit == 0 ? encode(img.format, i) : 0);

        ret = upload_image(img, unit, pixels);

        free(pixels);
        return ret;
}

static bool
check(const struct image_info img)
{
        const unsigned n = image_num_components(img.format) * product(img.size);
        uint32_t *pixels = malloc(sizeof(uint32_t) * n);
        uint32_t *expect = malloc(sizeof(uint32_t) * n);
        bool ret;
        int i;

        for (i = 0; i < n; ++i)
                expect[i] = encode(img.format, i);

        ret = download_image(img, 1, pixels) &&
                check_pixels_v(img, pixels, expect);

        free(expect);
        free(pixels);
        return ret;
}

static bool
run_test(const struct image_target_info *target,
         const struct image_extent size)
{
        struct grid_info grid;
        struct image_info img;
        GLuint prog;

        grid.stages = GL_FRAGMENT_SHADER_BIT;
        grid.format = get_image_format(GL_RGBA32F);
        grid.size = image_optimal_extent(size);

        img.target = target;
        img.format = grid.format;
        img.size = size;
        img.epsilon = image_format_epsilon(grid.format);

        prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(image_hunk(img, ""),
                       hunk("readonly uniform IMAGE_T src_img;\n"
                            "writeonly uniform IMAGE_T dst_img;\n"
                            "\n"
                            "GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        imageStore(dst_img, IMAGE_ADDR(idx),"
                            "                imageLoad(src_img, IMAGE_ADDR(idx)));\n"
                            "        return x;\n"
                            "}\n"), NULL));
        bool ret = prog && init_fb(grid) &&
                init_image(img, 0) &&
                init_image(img, 1) &&
                set_uniform_int(prog, "src_img", 0) &&
                set_uniform_int(prog, "dst_img", 1)  &&
                draw_grid(grid, prog) &&
                check(img);

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
        int d;

        piglit_require_extension("GL_ARB_shader_image_load_store");

        for (target = image_targets(); target->name; ++target) {
                for (d = 0; d < 4; ++d) {
                        if (should_test_dimension(target, d)) {
                                const struct image_extent size =
                                        get_test_extent(target, d);

                                subtest(&status,
                                        is_test_reasonable(quick, size),
                                        run_test(target, size),
                                        "image%s max size test/%dx%dx%dx%d",
                                        target->name,
                                        size.x, size.y, size.z, size.w);
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
