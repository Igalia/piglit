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

/** @file layer.c
 *
 * The spec defines two different image unit binding modes for
 * textures with multiple layers, depending on the "layered" parameter
 * of glBindImageTexture():
 *
 * "If <layered> is TRUE, the entire level is bound.  If <layered> is
 *  FALSE, only the single layer identified by <layer> will be bound.
 *  When <layered> is FALSE, the single bound layer is treated as a
 *  different texture target for image accesses:
 *
 *   * one-dimensional array texture layers are treated as one-dimensional
 *     textures;
 *
 *   * two-dimensional array, three-dimensional, cube map, cube map array
 *     texture layers are treated as two-dimensional textures; and
 *
 *   * two-dimensional multisample array textures are treated as
 *     two-dimensional multisample textures."
 *
 * We check that this is the case by binding a texture of each target
 * in layered or unlayered mode and then dumping all the accessible
 * contents of the texture to the framebuffer from a fragment shader.
 * After each individual texel is read its old contents are
 * overwritten by the shader.
 *
 * For textures that don't have multiple layers we check that the
 * described process reads and subsequently overwrites the full
 * texture contents regardless of the values of the layered and layer
 * parameters, since according to the spec:
 *
 * "If the texture identified by <texture> does not have multiple
 *  layers or faces, the entire texture level is bound, regardless of
 *  the values specified by <layered> and <layer>."
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
init_image(const struct image_info img, bool layered, unsigned l)
{
        uint32_t pixels[4 * N];
        unsigned i;
        bool ret;

        for (i = 0; i < 4 * N; ++i)
                pixels[i] = encode(img.format, i);

        ret = upload_image(img, 0, pixels);

        glBindImageTexture(0, get_texture(0), 0, layered, l,
                           GL_READ_WRITE, img.format->format);

        return ret && piglit_check_gl_error(GL_NO_ERROR);
}

static bool
check(const struct grid_info grid, const struct image_info img, unsigned l)
{
        const unsigned layer_sz = 4 * product(grid.size);
        uint32_t pixels_fb[4 * N], expect_fb[4 * N];
        uint32_t pixels_img[4 * N], expect_img[4 * N];
        unsigned i;

        if (!download_result(grid, pixels_fb) ||
            !download_image(img, 0, pixels_img))
                return false;

        for (i = 0; i < layer_sz; ++i) {
                /*
                 * The framebuffer contents should reflect layer l of
                 * the image which is bound to the image unit.
                 */
                expect_fb[i] = encode(grid.format, layer_sz * l + i);
        }

        for (i = 0; i < 4 * N; ++i) {
                if (i / layer_sz == l) {
                        /*
                         * Layer l should have been modified by the
                         * shader.
                         */
                        expect_img[i] = encode(img.format, 33);
                } else {
                        /*
                         * Other layers should have remained
                         * unchanged.
                         */
                        expect_img[i] = encode(img.format, i);
                }
        }

        if (!check_pixels_v(image_info_for_grid(grid), pixels_fb, expect_fb)) {
                printf("  Source: framebuffer\n");
                return false;
        }

        if (!check_pixels_v(img, pixels_img, expect_img)) {
                printf("  Source: image\n");
                return false;
        }

        return true;
}

/**
 * If \a layered is false, bind an individual layer of a texture to an
 * image unit, read its contents and write back a different value to
 * the same location.  If \a layered is true or the texture has a
 * single layer, the whole texture will be read and written back.
 *
 * For textures with a single layer, the arguments \a layered and \a
 * layer which are passed to the same arguments of
 * glBindImageTexture() should have no effect as required by the spec.
 */
static bool
run_test(const struct image_target_info *target,
         bool layered, unsigned layer)
{
        const struct image_info real_img = image_info(
                target->target, GL_RGBA32F, W, H);
        const unsigned slices = (layered ? 1 : image_num_layers(real_img));
        /*
         * "Slice" of the image that will be bound to the pipeline.
         */
        const struct image_info slice_img = image_info(
                (layered ? target->target : image_layer_target(target)),
                GL_RGBA32F, W, H / slices);
        /*
         * Grid with as many elements as the slice.
         */
        const struct grid_info grid = grid_info(
                GL_FRAGMENT_SHADER, GL_RGBA32F, W, H / slices);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(image_hunk(slice_img, ""),
                       hunk("uniform IMAGE_T img;\n"
                            "\n"
                            "GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        GRID_T v = imageLoad(img, IMAGE_ADDR(idx));\n"
                            "        imageStore(img, IMAGE_ADDR(idx), DATA_T(33));\n"
                            "        return v;\n"
                            "}\n"), NULL));
        bool ret = prog && init_fb(grid) &&
                init_image(real_img, layered, layer) &&
                set_uniform_int(prog, "img", 0) &&
                draw_grid(grid, prog) &&
                check(grid, real_img, (slices == 1 ? 0 : layer));

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
                subtest(&status, true,
                        run_test(target, false, 5),
                        "image%s/non-layered binding test", target->name);

                subtest(&status, true,
                        run_test(target, true, 5),
                        "image%s/layered binding test", target->name);
        }

        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_FAIL;
}
