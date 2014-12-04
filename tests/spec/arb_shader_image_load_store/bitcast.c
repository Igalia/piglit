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

/** @file bitcast.c
 *
 * Test that the reinterpretation of the binary contents of an image
 * as a different compatible format yields predictable results as
 * specified by the extension.
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
init_image(const struct image_info img,
           const struct image_format_info *dst_format)
{
        const unsigned m = image_num_components(img.format);
        uint32_t pixels[4 * N];
        int i;

        for (i = 0; i < m * N; ++i)
                pixels[i] = encode(img.format,
                                   get_idx(image_format_scale(img.format), i % m)
                                   * i / (m * N));

        if (!upload_image(img, 0, pixels))
           return false;

        glBindImageTexture(0, get_texture(0), 0, GL_TRUE, 0,
                           GL_READ_ONLY, dst_format->format);

        return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
check(const struct grid_info grid, const struct image_info src_img,
      const struct image_info dst_img)
{
        uint32_t pixels_fb[4 * N];
        uint32_t pixels_img[4 * N];

        if (!download_result(grid, pixels_fb))
                return false;

        /*
         * According to the spec, the reinterpretation of the texture
         * data performed by image loads is equivalent to:
         *
         * "reading the texel from the source format to scratch
         *  memory according to the process described for GetTexImage
         *  (section 6.1.4), using default pixel storage modes and
         *  <format> and <type> parameters corresponding to the source
         *  format in Table X.3; and [...]
         */
        glBindTexture(src_img.target->target, get_texture(0));
        glGetTexImage(src_img.target->target, 0, src_img.format->pixel_format,
                      src_img.format->pixel_type, pixels_img);

        /*
         *  [...] writing the texel from scratch memory to the destination
         *  format according to the process described for
         *  TexSubImage3D (section 3.9.2), using default pixel storage
         *  modes and <format> and <type> parameters corresponding to
         *  the destination format in Table X.3."
         */
        glTexImage2D(dst_img.target->target, 0, dst_img.format->format,
                     W, H, 0, dst_img.format->pixel_format,
                     dst_img.format->pixel_type, pixels_img);
        glGetTexImage(dst_img.target->target, 0, grid.format->pixel_format,
                      image_base_type(grid.format), pixels_img);

        return piglit_check_gl_error(GL_NO_ERROR) &&
                check_pixels_v(dst_img, pixels_fb, pixels_img);
}

static bool
run_test(const struct image_format_info *src_format,
         const struct image_format_info *dst_format)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER,
                          image_base_internal_format(dst_format), W, H);
        const struct image_info src_img =
                image_info(GL_TEXTURE_2D, src_format->format, W, H);
        const struct image_info dst_img =
                image_info(GL_TEXTURE_2D, dst_format->format, W, H);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(image_hunk(dst_img, ""),
                       hunk("uniform IMAGE_T img;\n"
                            "\n"
                            "GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        return imageLoad(img, IMAGE_ADDR(idx));\n"
                            "}\n"), NULL));
        bool ret = prog && init_fb(grid) &&
                init_image(src_img, dst_format) &&
                set_uniform_int(prog, "img", 0) &&
                draw_grid(grid, prog) &&
                check(grid, src_img, dst_img);

        glDeleteProgram(prog);
        return ret;
}

void
piglit_init(int argc, char **argv)
{
        const struct image_format_info *src_format, *dst_format;
        enum piglit_result status = PIGLIT_PASS;

        piglit_require_extension("GL_ARB_shader_image_load_store");

        for (src_format = image_formats_load_store;
             src_format->name; ++src_format) {
                for (dst_format = image_formats_load_store;
                     dst_format->name; ++dst_format) {
                        if (dst_format != src_format &&
                            image_compat_format(dst_format) ==
                            image_compat_format(src_format)) {
                                subtest(&status, true,
                                        run_test(src_format, dst_format),
                                        "%s to %s bitcast test",
                                        src_format->name, dst_format->name);
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
