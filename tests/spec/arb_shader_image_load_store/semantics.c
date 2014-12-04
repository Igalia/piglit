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

/** @file semantics.c
 *
 * Test all allowed combinations of image targets, formats, built-in
 * functions and shader stages.  The test initializes an image to some
 * arbitrary pattern and runs N invocations of a shader that calls the
 * built-in function once on the corresponding location of the image.
 * Then the same operation is simulated on the CPU and the results are
 * compared with each other.
 */

#include "common.h"

/** Window width.  The actual width of the image varies with the image
 * dimensionality, but the total number of pixels \a N remains
 * invariant. */
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

        /** Execute this image built-in on the CPU. */
        void (*exec)(const struct image_format_info *format,
                     const uint32_t *arg, uint32_t *img, uint32_t *ret);

        /** GLSL statement that invokes this image built-in. */
        const char *hunk;
};

static void
image_exec_load(const struct image_format_info *format,
                const uint32_t *arg, uint32_t *img, uint32_t *ret)
{
        int i;

        for (i = 0; i < image_num_components(format); ++i)
                ret[i] = img[i];
}

static void
image_exec_store(const struct image_format_info *format,
                 const uint32_t *arg, uint32_t *img, uint32_t *ret)
{
        int i;

        for (i = 0; i < image_num_components(format); ++i)
                img[i] = arg[i];

        for (i = 0; i < 4; ++i)
                ret[i] = 0;
}

static void
image_exec_add(const struct image_format_info *format,
               const uint32_t *arg, uint32_t *img, uint32_t *ret)
{
        ret[0] = img[0];
        img[0] = encode(format, (decode(format, img[0]) +
                                 decode(format, arg[0])));
}

static void
image_exec_min(const struct image_format_info *format,
               const uint32_t *arg, uint32_t *img, uint32_t *ret)
{
        ret[0] = img[0];
        img[0] = encode(format, MIN2(decode(format, img[0]),
                                     decode(format, arg[0])));
}

static void
image_exec_max(const struct image_format_info *format,
               const uint32_t *arg, uint32_t *img, uint32_t *ret)
{
        ret[0] = img[0];
        img[0] = encode(format, MAX2(decode(format, img[0]),
                                     decode(format, arg[0])));
}

static void
image_exec_and(const struct image_format_info *format,
               const uint32_t *arg, uint32_t *img, uint32_t *ret)
{
        ret[0] = img[0];
        img[0] = img[0] & arg[0];
}

static void
image_exec_or(const struct image_format_info *format,
              const uint32_t *arg, uint32_t *img, uint32_t *ret)
{
        ret[0] = img[0];
        img[0] = img[0] | arg[0];
}

static void
image_exec_xor(const struct image_format_info *format,
               const uint32_t *arg, uint32_t *img, uint32_t *ret)
{
        ret[0] = img[0];
        img[0] = img[0] ^ arg[0];
}

static void
image_exec_exchange(const struct image_format_info *format,
                    const uint32_t *arg, uint32_t *img, uint32_t *ret)
{
        ret[0] = img[0];
        img[0] = arg[0];
}

static void
image_exec_comp_swap(const struct image_format_info *format,
                     const uint32_t *arg, uint32_t *img, uint32_t *ret)
{
        ret[0] = img[0];
        img[0] = (img[0] == encode(format, image_format_scale(format).x / N) ?
                  arg[0] : img[0]);
}

static const struct image_op_info image_ops[] = {
        {
                "imageLoad", image_formats_load_store, image_exec_load,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        return imageLoad(img, IMAGE_ADDR(idx));\n"
                "}\n"
        },
        {
                "imageStore", image_formats_load_store, image_exec_store,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        imageStore(img, IMAGE_ADDR(idx), arg(idx));\n"
                "        return GRID_T(0);"
                "}\n"
        },
        {
                "imageAtomicAdd", image_formats_atomic, image_exec_add,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        return GRID_T(imageAtomicAdd(img, IMAGE_ADDR(idx),"
                "                                     arg(idx).x),"
                "                      0, 0, 1);\n"
                "}\n"
        },
        {
                "imageAtomicMin", image_formats_atomic, image_exec_min,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        return GRID_T(imageAtomicMin(img, IMAGE_ADDR(idx),"
                "                                     arg(idx).x),"
                "                      0, 0, 1);\n"
                "}\n"
        },
        {
                "imageAtomicMax", image_formats_atomic, image_exec_max,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        return GRID_T(imageAtomicMax(img, IMAGE_ADDR(idx),"
                "                                     arg(idx).x),"
                "                      0, 0, 1);\n"
                "}\n"
        },
        {
                "imageAtomicAnd", image_formats_atomic, image_exec_and,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        return GRID_T(imageAtomicAnd(img, IMAGE_ADDR(idx),"
                "                                     arg(idx).x),"
                "                      0, 0, 1);\n"
                "}\n"
        },
        {
                "imageAtomicOr", image_formats_atomic, image_exec_or,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        return GRID_T(imageAtomicOr(img, IMAGE_ADDR(idx),"
                "                                    arg(idx).x),"
                "                      0, 0, 1);\n"
                "}\n"
        },
        {
                "imageAtomicXor", image_formats_atomic, image_exec_xor,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        return GRID_T(imageAtomicXor(img, IMAGE_ADDR(idx),"
                "                                     arg(idx).x),"
                "                      0, 0, 1);\n"
                "}\n"
        },
        {
                "imageAtomicExchange", image_formats_atomic, image_exec_exchange,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        return GRID_T(imageAtomicExchange(img, IMAGE_ADDR(idx),"
                "                                          arg(idx).x),"
                "                      0, 0, 1);\n"
                "}\n"
        },
        {
                "imageAtomicCompSwap", image_formats_atomic, image_exec_comp_swap,
                "GRID_T op(ivec2 idx, GRID_T x) {\n"
                "        return GRID_T(imageAtomicCompSwap(img, IMAGE_ADDR(idx),"
                "                                          BASE_T(SCALE.x / N),"
                "                                          arg(idx).x),"
                "                      0, 0, 1);\n"
                "}\n"
        },
        { 0 }
};

static bool
init_image_pixels(const struct image_info img, unsigned unit,
                  uint32_t *r_pixels)
{
        const unsigned m = image_num_components(img.format);
        unsigned i;

        for (i = 0; i < m * N; ++i)
                r_pixels[i] = encode(img.format,
                                     get_idx(image_format_scale(img.format), i % m)
                                     * (unit == 0 ? i : m * N - i) / (m * N));

        return true;
}

static bool
init_image(const struct image_info img, unsigned unit)
{
        uint32_t pixels[4 * N];

        return init_image_pixels(img, unit, pixels) &&
                upload_image(img, unit, pixels);
}

static bool
check(const struct image_op_info *op,
      const struct grid_info grid,
      const struct image_info img)
{
        const struct image_info grid_img = {
                get_image_target(GL_TEXTURE_2D),
                grid.format, grid.size, img.epsilon
        };
        const unsigned m = image_num_components(img.format);
        uint32_t pixels_fb[4 * N], expect_fb[4 * N];
        uint32_t pixels_img[4 * N], expect_img[4 * N];
        uint32_t arg[4 * N];
        int i;

        if (!download_result(grid, pixels_fb) ||
            !download_image(img, 0, pixels_img))
                return false;

        /* Initialize the image and argument to the known state. */
        init_image_pixels(img, 0, expect_img);
        init_image_pixels(img, 1, arg);
        init_pixels(grid_img, expect_fb, 0, 0, 0, 1);

        /* Calculate the result of the image built-in. */
        for (i = 0; i < N; ++i)
                op->exec(img.format, &arg[m * i], &expect_img[m * i],
                         &expect_fb[4 * i]);

        /* Check that the shader gave the same result. */
        if (!check_pixels_v(grid_img, pixels_fb, expect_fb)) {
                printf("  Source: framebuffer\n");
                return false;
        }

        if (!check_pixels_v(img, pixels_img, expect_img)) {
                printf("  Source: image\n");
                return false;
        }

        return true;
}

static bool
run_test(const struct image_op_info *op,
         const struct image_stage_info *stage,
         const struct image_format_info *format,
         const struct image_target_info *target)
{
        const struct grid_info grid = grid_info(
                stage->stage, image_base_internal_format(format), W, H);
        const struct image_info img = image_info(
                target->target, format->format, W, H);
        GLuint prog = generate_program(
                grid, stage->stage,
                concat(image_hunk(img, ""),
                       hunk("uniform IMAGE_T img;\n"
                            "uniform IMAGE_T arg_img;\n"
                            "\n"
                            "GRID_T arg(ivec2 idx) {\n"
                            "        return imageLoad(arg_img, IMAGE_ADDR(idx));\n"
                            "}\n"),
                       hunk(op->hunk), NULL));
        bool ret = prog && init_fb(grid) &&
                init_image(img, 0) &&
                init_image(img, 1) &&
                set_uniform_int(prog, "img", 0) &&
                set_uniform_int(prog, "arg_img", 1) &&
                draw_grid(grid, prog) &&
                check(op, grid, img);

        glDeleteProgram(prog);
        return ret;
}

void
piglit_init(int argc, char **argv)
{
        /*
         * If quick is enabled traverse each variable (stage, op,
         * format and target) in sequence leaving the other ones fixed
         * instead of going through the cartesian product of the four
         * variables.
         */
        const bool quick = (argc >= 2 && !strcmp(argv[1], "--quick"));
        enum piglit_result status = PIGLIT_PASS;
        const struct image_stage_info *stage;
        const struct image_op_info *op;
        const struct image_format_info *format;
        const struct image_target_info *target;
        unsigned m = 0;

        piglit_require_extension("GL_ARB_shader_image_load_store");
        piglit_require_extension("GL_ARB_texture_cube_map_array");

        for (op = image_ops; op->name; ++op) {
                for (stage = image_stages(); stage->name; ++stage) {
                        for (format = op->formats; format->name; ++format) {
                                for (target = image_targets(); target->name; ++target) {
                                        subtest(&status, true,
                                                run_test(op, stage, format, target),
                                                "%s/%s shader/%s/image%s test",
                                                op->name, stage->name,
                                                format->name, target->name);

                                        if (quick && (m & 1))
                                                break;
                                }

                                if (quick && ((m |= 1) & 2))
                                        break;
                        }

                        if (quick && ((m |= 2) & 4))
                                break;
                }

                if (quick)
                        m |= 4;
        }

        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_FAIL;
}
