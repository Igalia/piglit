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

/** @file invalid.c
 *
 * The ARB_shader_image_load_store extension defines an image access
 * to be invalid when certain conditions are met, in which case image
 * stores and atomics are defined to have no effect and image loads
 * and atomics give zero as result.  This test causes such invalid
 * accesses and checks that the result is as expected and that no data
 * is accidentally overwritten.
 *
 * The spec describes other conditions that cause an image access to
 * have undefined results.  In those cases we simply check that the
 * undefined access didn't lead to program termination.
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

        /** Allowed image formats. */
        const struct image_format_info *formats;

        /** GLSL statement that invokes this image built-in. */
        const char *hunk;
};

static const struct image_op_info image_ops[] = {
        {
                "imageLoad", image_formats_load_store,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        return imageLoad(imgs[u], off + IMAGE_ADDR(idx));\n"
                "}\n"
        },
        {
                "imageStore", image_formats_load_store,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        imageStore(imgs[u], off + IMAGE_ADDR(idx), DATA_T(33));\n"
                "        return GRID_T(0, 0, 0, SCALE.w == 0 ? 1 : 0);"
                "}\n"
        },
        {
                "imageAtomicAdd", image_formats_atomic,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        return GRID_T(imageAtomicAdd(imgs[u],"
                "                                    off + IMAGE_ADDR(idx),"
                "                                    BASE_T(33)),"
                "                      0, 0, 1);\n"
                "}\n"
        },
        {
                "imageAtomicMin", image_formats_atomic,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        return GRID_T(imageAtomicMin(imgs[u],"
                "                                     off + IMAGE_ADDR(idx),"
                "                                     BASE_T(33)),"
                "                      0, 0, 1);\n"
                "}\n"
        },
        {
                "imageAtomicMax", image_formats_atomic,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        return GRID_T(imageAtomicMax(imgs[u],"
                "                                     off + IMAGE_ADDR(idx),"
                "                                     BASE_T(33)),"
                "                      0, 0, 1);\n"
                "}\n"
        },
        {
                "imageAtomicAnd", image_formats_atomic,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        return GRID_T(imageAtomicAnd(imgs[u],"
                "                                     off + IMAGE_ADDR(idx),"
                "                                     BASE_T(33)),"
                "                      0, 0, 1);\n"
                "}\n"
        },
        {
                "imageAtomicOr", image_formats_atomic,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        return GRID_T(imageAtomicOr(imgs[u],"
                "                                    off + IMAGE_ADDR(idx),"
                "                                    BASE_T(33)),"
                "                      0, 0, 1);\n"
                "}\n"
        },
        {
                "imageAtomicXor", image_formats_atomic,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        return GRID_T(imageAtomicXor(imgs[u],"
                "                                     off + IMAGE_ADDR(idx),"
                "                                     BASE_T(33)),"
                "                      0, 0, 1);\n"
                "}\n"
        },
        {
                "imageAtomicExchange", image_formats_atomic,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        return GRID_T(imageAtomicExchange(imgs[u],"
                "                                          off + IMAGE_ADDR(idx),"
                "                                          BASE_T(33)),"
                "                      0, 0, 1);\n"
                "}\n"
        },
        {
                "imageAtomicCompSwap", image_formats_atomic,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        return GRID_T(imageAtomicCompSwap(imgs[u],"
                "                                          off + IMAGE_ADDR(idx),"
                "                                          BASE_T(0), BASE_T(33)),"
                "                      0, 0, 1);\n"
                "}\n"
        },
        { 0 }
};

static bool
init_image(const struct image_info img, GLuint prog)
{
        uint32_t pixels[4 * N];

        return init_pixels(img, pixels, 1, 1, 1, 1) &&
                upload_image(img, 0, pixels) &&
                set_uniform_int(prog, "imgs[0]", 0);
}

static bool
init_level(const struct image_info img, unsigned level,
           GLenum format, unsigned w, unsigned h)
{
        uint32_t pixels[4 * N];

        init_pixels(img, pixels, 1, 1, 1, 1);
        glBindTexture(GL_TEXTURE_2D, get_texture(0));
        glTexImage2D(GL_TEXTURE_2D, level, format,
                     w, h, 0, img.format->pixel_format,
                     image_base_type(img.format), pixels);

        return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
check(const struct grid_info grid, const struct image_info img)
{
        uint32_t pixels_fb[4 * N], pixels_img[4 * N];

        if (!download_result(grid, pixels_fb) ||
            !download_image(img, 0, pixels_img))
                return false;

        /* Check that the built-in return value is zero (nonexisting texel). */
        if (!check_pixels(image_info_for_grid(grid), pixels_fb, 0, 0, 0,
                          (image_num_components(img.format) < 4 ? 1 : 0))) {
                printf("  Source: framebuffer\n");
                return false;
        }

        /* Check that the image wasn't modified. */
        if (!check_pixels(img, pixels_img, 1, 1, 1, 1)) {
                printf("  Source: image\n");
                return false;
        }

        return true;
}

static bool
invalidate_unbound(const struct image_info img, GLuint prog)
{
        glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY,
                           img.format->format);

        return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
invalidate_incomplete(const struct image_info img, GLuint prog)
{
        /* Bind a mipmap level with incorrect dimensions so the
         * texture becomes incomplete. */
        bool ret = init_level(img, 1, img.format->format, W, H);

        glBindImageTexture(0, get_texture(0), 1, GL_TRUE, 0, GL_READ_WRITE,
                           img.format->format);

        return ret && piglit_check_gl_error(GL_NO_ERROR);
}

static bool
invalidate_level_bounds(const struct image_info img, GLuint prog)
{
        const int level = 1;
        const struct image_extent size = image_level_size(img, level);
        /* Create a second mipmap level */
        bool ret = init_level(img, level, img.format->format, size.x, size.y);

        /* and set it as base level, */
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, &level);

        /* but keep the the first level bound. */
        glBindImageTexture(0, get_texture(0), 0, GL_TRUE, 0, GL_READ_WRITE,
                           img.format->format);

        return ret && piglit_check_gl_error(GL_NO_ERROR);
}

static bool
invalidate_invalid_format(const struct image_info img, GLuint prog)
{
        const GLenum base_format = image_base_internal_format(img.format);
        /* Pick an invalid texture format with a compatible base
         * type. */
        bool ret = init_level(img, 0, (base_format == GL_RGBA32F ?
                                       GL_RGB5_A1 : GL_RGB8UI), W, H);

        glBindImageTexture(0, get_texture(0), 0, GL_TRUE, 0,
                           GL_READ_WRITE, img.format->format);

        return ret && piglit_check_gl_error(GL_NO_ERROR);
}

static bool
invalidate_incompatible_format(const struct image_info img, GLuint prog)
{
        GLenum base_format = image_base_internal_format(img.format);
        /* Pick an incompatible texture format with a compatible base
         * type. */
        bool ret = init_level(img, 0, (base_format == GL_RGBA32F ?
                                       GL_RGBA8 : GL_RG32UI), W, H);

        glBindImageTexture(0, get_texture(0), 0, GL_TRUE, 0,
                           GL_READ_WRITE, img.format->format);

        return ret && piglit_check_gl_error(GL_NO_ERROR);
}

static bool
invalidate_layer_bounds(const struct image_info img, GLuint prog)
{
        glBindImageTexture(0, get_texture(0), 0, GL_FALSE, N,
                           GL_READ_WRITE, img.format->format);

        return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
invalidate_address_bounds(const struct image_info img, GLuint prog)
{
        return set_uniform_int(prog, "off", N);
}

static bool
invalidate_index_bounds(const struct image_info img, GLuint prog)
{
        return set_uniform_int(prog, "u", 0xdeadcafe);
}

static bool
invalidate_nop(const struct image_info img, GLuint prog)
{
        return true;
}

static bool
run_test(const struct image_op_info *op,
         const struct image_info real_img,
         const struct image_info prog_img,
         bool (*invalidate)(const struct image_info, GLuint),
         bool control_test)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER,
                          image_base_internal_format(real_img.format), W, H);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(image_hunk(prog_img, ""),
                       hunk("uniform IMAGE_T imgs[1];\n"
                            "uniform int u;\n"
                            "uniform int off;\n"),
                       hunk(op->hunk),
                       NULL));
        bool ret = prog &&
                init_fb(grid) &&
                init_image(real_img, prog) &&
                invalidate(real_img, prog) &&
                draw_grid(grid, prog) &&
                (check(grid, real_img) || control_test);

        glDeleteProgram(prog);
        return ret;
}

void
piglit_init(int argc, char **argv)
{
        enum piglit_result status = PIGLIT_PASS;
        const struct image_op_info *op;
        const struct image_format_info *format;
        const struct image_target_info *target;

        piglit_require_extension("GL_ARB_shader_image_load_store");

        for (op = image_ops; op->name; ++op) {
                const struct image_info def_img = image_info(
                        GL_TEXTURE_2D, op->formats[0].format, W, H);

                /*
                 * According to the spec, an access is considered
                 * invalid in the following cases, in which image
                 * stores and atomics should have no effect, and image
                 * loads should return zero:
                 *
                 * " * no texture is bound to the selected image unit;
                 *     [...]"
                 */
                subtest(&status, true,
                        run_test(op, def_img, def_img,
                                 invalidate_unbound, false),
                        "%s/unbound image test", op->name);

                /*
                 * " * the texture bound to the selected image unit is
                 *     incomplete; [...]"
                 */
                subtest(&status, true,
                        run_test(op, def_img, def_img,
                                 invalidate_incomplete, false),
                        "%s/incomplete image test", op->name);

                /*
                 * " * the texture level bound to the image unit is
                 *     less than the base level or greater than the
                 *     maximum level of the texture; [...]"
                 */
                subtest(&status, true,
                        run_test(op, def_img, def_img,
                                 invalidate_level_bounds, false),
                        "%s/level bounds test", op->name);

                /*
                 * " * the internal format of the texture bound to the
                 *     image unit is not found in Table X.2; [...]"
                 */
                subtest(&status, true,
                        run_test(op, def_img, def_img,
                                 invalidate_invalid_format, false),
                        "%s/invalid format test", op->name);

                /*
                 * " * the internal format of the texture bound to the
                 *     image unit is incompatible with the specified
                 *     <format> according to Table X.3; [...]"
                 */
                subtest(&status, true,
                        run_test(op, def_img, def_img,
                                 invalidate_incompatible_format, false),
                        "%s/incompatible format test", op->name);

                /*
                 * " * the texture bound to the image unit has layers,
                 *     and the selected layer or cube map face doesn't
                 *     exist; [...]"
                 */
                subtest(&status, true,
                        run_test(op,
                                 image_info(GL_TEXTURE_2D_ARRAY,
                                            op->formats[0].format, W, H),
                                 def_img, invalidate_layer_bounds, false),
                        "%s/layer bounds test", op->name);

                /*
                 * " * the selected texel tau_i, tau_i_j, or tau_i_j_k
                 *     doesn't exist; [...]"
                 */
                for (target = image_targets(); target->name; ++target) {
                        const struct image_info img = image_info(
                                target->target, op->formats[0].format, W, H);

                        subtest(&status, true,
                                run_test(op, img, img,
                                         invalidate_address_bounds, false),
                                "%s/address bounds test/image%s/%s",
                                op->name, img.target->name, img.format->name);
                }

                for (format = &op->formats[1]; format->name; ++format) {
                        const struct image_info img = image_info(
                                GL_TEXTURE_2D, format->format, W, H);

                        subtest(&status, true,
                                run_test(op, img, img,
                                         invalidate_address_bounds, false),
                                "%s/address bounds test/image%s/%s",
                                op->name, img.target->name, img.format->name);
                }

                /*
                 * The following cases have undefined results, but may
                 * not lead to program termination:
                 *
                 * "If the index used to select an individual [array]
                 *  element is negative or greater than or equal to
                 *  the size of the array [...]"
                 */
                subtest(&status, true,
                        run_test(op, def_img, def_img,
                                 invalidate_index_bounds, true),
                        "%s/index bounds test", op->name);

                /*
                 * "the type of image variable used to access the
                 *  image unit does not match the target of a texture
                 *  bound to the image unit [...]"
                 */
                subtest(&status, true,
                        run_test(op, def_img,
                                 image_info(GL_TEXTURE_3D,
                                            op->formats[0].format, W, H),
                                 invalidate_nop, true),
                        "%s/target mismatch test", op->name);

                /*
                 * "the format layout qualifier for an image variable
                 *  used for an image load or atomic operation does
                 *  not match the format of the image unit [...]"
                 */
                subtest(&status, true,
                        run_test(op,
                                 image_info(GL_TEXTURE_2D,
                                            GL_R11F_G11F_B10F, W, H),
                                 def_img, invalidate_nop, true),
                        "%s/format mismatch test", op->name);

        }

        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_FAIL;
}
