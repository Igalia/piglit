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

/** @file qualifiers.c
 *
 * Test several combinations of image access qualifiers and binding
 * access modes and check that omitting optional qualifiers doesn't
 * have any effect on the rendering.
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

static bool
init_image_pixels(const struct image_info img, unsigned unit,
                  uint32_t *r_pixels)
{
        const unsigned m = image_num_components(img.format);
        const struct image_datum s = image_format_scale(img.format);
        unsigned i;

        for (i = 0; i < m * N; ++i)
                r_pixels[i] =
                        (unit == 1 ? 0 :
                         encode(img.format, get_idx(s, i % m) * i / (m * N)));

        return true;
}

static bool
init_image(const struct image_info img, unsigned unit,
           bool strict_binding)
{
        uint32_t pixels[4 * N];
        bool ret = init_image_pixels(img, unit, pixels) &&
                upload_image(img, unit, pixels);

        if (strict_binding)
                glBindImageTexture(unit, get_texture(unit), 0, GL_TRUE, 0,
                                   (unit == 1 ? GL_WRITE_ONLY : GL_READ_ONLY),
                                   img.format->format);

        return ret && piglit_check_gl_error(GL_NO_ERROR);
}

static bool
check(const struct grid_info grid, const struct image_info img)
{
        uint32_t pixels[4 * N], expect[4 * N];

        return init_image_pixels(img, 0, expect) &&
                download_image(img, 1, pixels) &&
                check_pixels_v(img, pixels, expect);
}

static char *
test_hunk(bool strict_layout_qualifiers,
          bool strict_access_qualifiers)
{
        char *s = NULL;

        asprintf(&s, "#define SRC_IMAGE_Q IMAGE_LAYOUT_Q %s\n"
                 "#define DST_IMAGE_Q %s %s\n",
                 (strict_access_qualifiers ? "readonly" : ""),
                 (strict_layout_qualifiers || !strict_access_qualifiers ?
                  "IMAGE_LAYOUT_Q" : ""),
                 (strict_access_qualifiers ? "writeonly" : ""));
        return s;
}

/**
 * Copy from a source image into a destination image of the specified
 * format and check the result.
 *
 * If \a strict_layout_qualifiers is false, uniform layout qualifiers
 * will be omitted where allowed by the spec.  If \a
 * strict_access_qualifiers is false, the "readonly" and "writeonly"
 * qualifiers will be omitted.  If \a strict_binding is false, the
 * image will be bound as READ_WRITE, otherwise only the required
 * access type will be used.
 */
static bool
run_test(const struct image_format_info *format,
         bool strict_layout_qualifiers,
         bool strict_access_qualifiers,
         bool strict_binding)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER,
                          image_base_internal_format(format), W, H);
        const struct image_info img =
                image_info(GL_TEXTURE_2D, format->format, W, H);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(image_hunk(img, ""),
                       test_hunk(strict_layout_qualifiers,
                                 strict_access_qualifiers),
                       hunk("SRC_IMAGE_Q uniform IMAGE_BARE_T src_img;\n"
                            "DST_IMAGE_Q uniform IMAGE_BARE_T dst_img;\n"
                            "\n"
                            "GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        imageStore(dst_img, IMAGE_ADDR(idx),"
                            "                   imageLoad(src_img, IMAGE_ADDR(idx)));\n"
                            "        return x;\n"
                            "}\n"), NULL));
        bool ret = prog && init_fb(grid) &&
                init_image(img, 0, strict_binding) &&
                init_image(img, 1, strict_binding) &&
                set_uniform_int(prog, "src_img", 0) &&
                set_uniform_int(prog, "dst_img", 1) &&
                draw_grid(grid, prog) &&
                check(grid, img);

        glDeleteProgram(prog);
        return ret;
}

void
piglit_init(int argc, char **argv)
{
        enum piglit_result status = PIGLIT_PASS;
        const struct image_format_info *format;
        unsigned i;

        piglit_require_extension("GL_ARB_shader_image_load_store");

        for (format = image_formats_load_store; format->name; ++format) {
                for (i = 0; i < 8; ++i) {
                        const bool strict_layout_qualifiers = i & 1;
                        const bool strict_access_qualifiers = i & 2;
                        const bool strict_binding = i & 4;

                        subtest(&status, true,
                                run_test(format, strict_layout_qualifiers,
                                         strict_access_qualifiers,
                                         strict_binding),
                                "%s/%s layout qualifiers/%s access qualifiers/"
                                "%s binding test", format->name,
                                (strict_layout_qualifiers ? "strict" : "permissive"),
                                (strict_access_qualifiers ? "strict" : "permissive"),
                                (strict_binding ? "strict" : "permissive"));
                }
        }

        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_FAIL;
}
