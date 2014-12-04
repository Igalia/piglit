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

/** @file host-mem-barrier.c
 *
 * Unlike other GL API objects, images are not implicitly synchronized
 * with subsequent GL operations.  The glMemoryBarrier() API is provided
 * to serialize shader memory transactions issued by previous drawing
 * commands with respect to a given set of subsequent GL commands
 * specified as a bit set.
 *
 * This test should cause several kinds of data hazard situations
 * deliberately (RaW, WaR, WaW) between image loads and stores and other
 * parts of the pipeline including vertex, element and indirect command
 * fetch, shader uniform buffer, image and atomic counter access, texture
 * sampling, pixel transfer operations, texture and buffer update
 * commands, framebuffer writes and reads and transform feedback output.
 *
 * The test is repeated for different execution sizes to account for
 * implementations with varying levels of parallelism and with caches
 * of different sizes.  Unless running in "quick" mode a series of
 * control tests is executed which inhibits all glMemoryBarrier()
 * calls in order to make sure that the test is leading to data
 * hazards, since otherwise the main test is not meaningful.  The
 * control test always passes as it is expected to misrender.
 */

#include "common.h"

/** Maximum image width. */
#define L 64

/** Maximum number of pixels. */
#define N (L * L)

PIGLIT_GL_TEST_CONFIG_BEGIN

config.supports_gl_core_version = 32;

config.window_width = L;
config.window_height = L;
config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static char *
common_hunk(const struct image_info img)
{
        return concat(hunk("#extension GL_ARB_shader_atomic_counters : enable\n"),
                      image_hunk(img, ""),
                      hunk("#define RED DATA_T(1, 0, 0, 1)\n"
                           "#define GREEN DATA_T(0, 1, 0, 1)\n"
                           "\n"
                           "uniform IMAGE_T src_img;\n"
                           "uniform IMAGE_T dst_img;\n"
                           "uniform int pass;\n"), NULL);
}

struct image_barrier_info {
        /** Test name. */
        const char *name;

        /** Invoke a memory barrier affecting the specified units. */
        bool (*run_barrier)(GLbitfield barriers);

        /** Informative "control" test with no barriers whose result
         * is ignored. */
        bool control_test;
};

static bool
run_barrier_none(GLbitfield barriers)
{
        return true;
}

static bool
run_barrier_one(GLbitfield barriers)
{
        glMemoryBarrier(barriers);
        return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
run_barrier_full(GLbitfield barriers)
{
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        return piglit_check_gl_error(GL_NO_ERROR);
}

const struct image_barrier_info image_barriers[] = {
        { "control", run_barrier_none, true },
        { "one bit", run_barrier_one, false },
        { "full", run_barrier_full, false },
        { 0 }
};

static bool
check_fb_green(const struct grid_info grid)
{
        uint32_t pixels[N][4];

        return download_result(grid, pixels[0]) &&
                check_pixels(image_info_for_grid(grid),
                             pixels[0], 0, 1, 0, 1);
}

static bool
check_img_green(const struct image_info img)
{
        uint32_t pixels[N][4];

        return download_image(img, 1, pixels[0]) &&
                check_pixels(img, pixels[0], 0, 1, 0, 1);
}

static bool
init_common(const struct grid_info grid, const struct image_info img,
            GLuint prog)
{
        uint32_t pixels[N][4];

        if (!init_pixels(img, pixels[0], 0, 1, 0, 1) ||
            !upload_image(img, 0, pixels[0]) ||
            !init_pixels(img, pixels[0], 66, 66, 66, 66) ||
            !upload_image(img, 1, pixels[0]))
                return false;

        set_uniform_int(prog, "src_img", 0);
        set_uniform_int(prog, "dst_img", 1);
        set_uniform_int(prog, "pass", 0);

        return init_fb(grid);
}

static bool
run_test_vertex_array_raw(const struct image_barrier_info *bar, unsigned l)
{
        const struct grid_info grid =
                grid_info(GL_VERTEX_SHADER, GL_RGBA32F, l, l);
        const struct image_info img =
                image_info(GL_TEXTURE_BUFFER, GL_RGBA32F, l, l);
        static GLuint vao, vbo;
        GLuint prog = generate_program(
                grid, GL_VERTEX_SHADER,
                concat(common_hunk(img),
                       hunk("in vec4 piglit_texcoord;\n"
                            "\n"
                            "GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        if (pass == 1) {\n"
                            "                return piglit_texcoord;\n"
                            "        } else {\n"
                            "                imageStore(dst_img, IMAGE_ADDR(idx), GREEN);\n"
                            "                return x;\n"
                            "        }\n"
                            "}\n"), NULL));
        bool ret = prog && init_common(grid, img, prog) &&
                generate_grid_arrays(&vao, &vbo,
                                     1.0 / l - 1.0, 1.0 / l - 1.0,
                                     2.0 / l, 2.0 / l, l, l);

        /* Bind the image as texcoord vbo simultaneously. */
        glBindBuffer(GL_ARRAY_BUFFER, get_buffer(1));
        glVertexAttribPointer(PIGLIT_ATTRIB_TEX, 4, GL_FLOAT, GL_FALSE,
                               0, 0);
        glEnableVertexAttribArray(PIGLIT_ATTRIB_TEX);

        /* First pass: render green to the vbo. */
        glDrawArrays(GL_POINTS, 0, l * l);

        /* Barrier. */
        bar->run_barrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

        /* Second pass: check that the texcoords are all green
         * (read-after-write). */
        set_uniform_int(prog, "pass", 1);
        glDrawArrays(GL_POINTS, 0, l * l);

        ret &= piglit_check_gl_error(GL_NO_ERROR) &&
                check_fb_green(grid);

        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_element_array_raw(const struct image_barrier_info *bar, unsigned l)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_RGBA32F, l, l);
        const struct image_info img =
                image_info(GL_TEXTURE_BUFFER, GL_R32UI, l, l);
        static GLuint vao, vbo;
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(common_hunk(img),
                       hunk("GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        if (pass == 1) {\n"
                            "                return GRID_T(GREEN);\n"
                            "        } else {\n"
                            "                imageStore(dst_img, IMAGE_ADDR(idx),"
                            "                           DATA_T(IMAGE_ADDR(idx)));\n"
                            "                return x;\n"
                            "        }\n"
                            "}\n"), NULL));
        bool ret = prog && init_common(grid, img, prog) &&
                generate_grid_arrays(&vao, &vbo,
                                     1.0 / l - 1.0, 1.0 / l - 1.0,
                                     2.0 / l, 2.0 / l, l, l);

        /* Bind the image as element buffer simultaneously. */
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, get_buffer(1));

        /* First pass: write sequential indices to the element buffer. */
        glDrawArrays(GL_POINTS, 0, l * l);

        /* Barrier. */
        bar->run_barrier(GL_ELEMENT_ARRAY_BARRIER_BIT);

        /* Second pass: render the generated element buffer
         * (read-after-write). */
        set_uniform_int(prog, "pass", 1);
        glDrawElements(GL_POINTS, l * l, GL_UNSIGNED_INT, 0);

        ret &= piglit_check_gl_error(GL_NO_ERROR) &&
                check_fb_green(grid);

        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_ubo_raw(const struct image_barrier_info *bar, unsigned l)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_RGBA32F, l, l);
        const struct image_info img =
                image_info(GL_TEXTURE_BUFFER, GL_RGBA32F, l, l);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(common_hunk(img),
                       hunk("layout(std140) uniform u {\n"
                            "        vec4 xs[N];\n"
                            "};\n"
                            "\n"
                            "GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        if (pass == 1) {\n"
                            "                return xs[IMAGE_ADDR(idx)];\n"
                            "        } else {\n"
                            "                imageStore(dst_img, IMAGE_ADDR(idx), GREEN);\n"
                            "                return x;\n"
                            "        }\n"
                            "}\n"), NULL));
        bool ret = prog && init_common(grid, img, prog);

        /* Bind the image as uniform buffer simultaneously. */
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, get_buffer(1));

        /* First pass: render green to the uniform buffer. */
        ret &= draw_grid(grid, prog) &&

                /* Barrier. */
                bar->run_barrier(GL_UNIFORM_BARRIER_BIT) &&

                /* Second pass: check that the uniforms are all green
                 * (read-after-write). */
                set_uniform_int(prog, "pass", 1) &&
                draw_grid(grid, prog) &&
                check_fb_green(grid);

        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_tex_fetch_raw(const struct image_barrier_info *bar, unsigned l)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_RGBA32F, l, l);
        const struct image_info img =
                image_info(GL_TEXTURE_2D, GL_RGBA32F, l, l);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(common_hunk(img),
                       hunk("uniform sampler2D tex;\n"
                            "\n"
                            "GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        if (pass == 1) {\n"
                            "                return texelFetch(tex, idx, 0);\n"
                            "        } else {\n"
                            "                imageStore(dst_img, IMAGE_ADDR(idx), GREEN);\n"
                            "                return x;\n"
                            "        }\n"
                            "}\n"), NULL));
        bool ret = prog && init_common(grid, img, prog);

        /* Bind the image as texture simultaneously. */
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, get_texture(1));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        /* First pass: render green to the texture. */
        ret &= draw_grid(grid, prog) &&

                /* Barrier. */
                bar->run_barrier(GL_TEXTURE_FETCH_BARRIER_BIT) &&
                set_uniform_int(prog, "pass", 1) &&

                /* Second pass: check that the texture is green
                 * (read-after-write). */
                draw_grid(grid, prog) &&
                check_fb_green(grid);

        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_image_raw(const struct image_barrier_info *bar, unsigned l)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_RGBA32F, l, l);
        const struct image_info img =
                image_info(GL_TEXTURE_2D, GL_RGBA32F, l, l);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(common_hunk(img),
                       hunk("GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        if (pass == 1) {\n"
                            "                return imageLoad(dst_img, IMAGE_ADDR(idx));\n"
                            "        } else {\n"
                            "                imageStore(dst_img, IMAGE_ADDR(idx), GREEN);\n"
                            "                return x;\n"
                            "        }\n"
                            "}\n"), NULL));
        bool ret = prog && init_common(grid, img, prog) &&

                /* First pass: render green to the image. */
                draw_grid(grid, prog) &&

                /* Barrier. */
                bar->run_barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT) &&

                /* Second pass: check that the image is green
                 * (read-after-write). */
                set_uniform_int(prog, "pass", 1) &&
                draw_grid(grid, prog) &&
                check_fb_green(grid);

        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_image_war(const struct image_barrier_info *bar, unsigned l)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_RGBA32F, l, l);
        const struct image_info img =
                image_info(GL_TEXTURE_2D, GL_RGBA32F, l, l);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(common_hunk(img),
                       hunk("GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        if (pass == 1) {\n"
                            "                imageStore(src_img, IMAGE_ADDR(idx), RED);\n"
                            "        } else {\n"
                            "                imageStore(dst_img, IMAGE_ADDR(idx),"
                            "                           imageLoad(src_img, IMAGE_ADDR(idx)));\n"
                            "        }\n"
                            "        return x;\n"
                            "}\n"), NULL));
        bool ret = prog && init_common(grid, img, prog) &&

                /* First pass: read back the source image. */
                draw_grid(grid, prog) &&

                /* Barrier. */
                bar->run_barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT) &&

                /* Second pass: render red to the source image
                 * (write-after-read). */
                set_uniform_int(prog, "pass", 1) &&
                draw_grid(grid, prog) &&

                /* Check that the read-back results from the first
                 * pass are green. */
                check_img_green(img);

        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_indirect_raw(const struct image_barrier_info *bar, unsigned l)
{
        const struct grid_info grid =
                grid_info(GL_VERTEX_SHADER, GL_RGBA32F, l, l);
        const struct image_info img =
                image_info(GL_TEXTURE_BUFFER, GL_RGBA32UI, l, l);
        static GLuint vao, vbo;
        GLuint prog = generate_program(
                grid, GL_VERTEX_SHADER,
                concat(common_hunk(img),
                       hunk("GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        if (pass == 1) {\n"
                            "                return GRID_T(gl_InstanceID == 2 ? GREEN : RED);\n"
                            "        } else {\n"
                            "                imageStore(dst_img, IMAGE_ADDR(idx),"
                            "                           DATA_T(1, 3, IMAGE_ADDR(idx), 0));\n"
                            "                return x;\n"
                            "        }\n"
                            "}\n"), NULL));
        bool ret = prog && init_common(grid, img, prog) &&
                generate_grid_arrays(&vao, &vbo,
                                     1.0 / l - 1.0, 1.0 / l - 1.0,
                                     2.0 / l, 2.0 / l, l, l);

        /* Bind the image as indirect command buffer
         * simultaneously. */
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, get_buffer(1));

        /* First pass: Render instanced draw commands to the indirect
         * buffer. */
        glDrawArrays(GL_POINTS, 0, l * l);

        /* Barrier. */
        bar->run_barrier(GL_COMMAND_BARRIER_BIT);

        /* Second pass: render the generated indirect buffer
         * (read-after-write). */
        set_uniform_int(prog, "pass", 1);
        glMultiDrawArraysIndirect(GL_POINTS, 0, l * l, 0);

        ret &= piglit_check_gl_error(GL_NO_ERROR) &&
                check_fb_green(grid);

        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_pixel_raw(const struct image_barrier_info *bar, unsigned l)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_RGBA32F, l, l);
        const struct image_info img =
                image_info(GL_TEXTURE_BUFFER, GL_RGBA32F, l, l);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(common_hunk(img),
                       hunk("GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        imageStore(dst_img, IMAGE_ADDR(idx), GREEN);\n"
                            "        return x;\n"
                            "}\n"), NULL));
        bool ret = prog && init_common(grid, img, prog);
        uint32_t pixels[N][4];
        GLuint tex;

        /* Bind the image as pixel unpack buffer simultaneously. */
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, get_buffer(1));

        /* First pass: render green to the image. */
        draw_grid(grid, prog);

        /* Barrier. */
        bar->run_barrier(GL_PIXEL_BUFFER_BARRIER_BIT);

        /* Second pass: use the result as pixel source
         * (read-after-write). */
        glTexImage2D(GL_TEXTURE_2D, 0, img.format->format,
                     l, l, 0, img.format->pixel_format,
                     img.format->pixel_type, 0);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        /* Check that the resulting texture is green. */
        glGetTexImage(GL_TEXTURE_2D, 0, img.format->pixel_format,
                      image_base_type(img.format), pixels);

        ret &= piglit_check_gl_error(GL_NO_ERROR) &&
                check_pixels(img, pixels[0], 0, 1, 0, 1);

        glDeleteTextures(1, &tex);
        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_pixel_waw(const struct image_barrier_info *bar, unsigned l)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_RGBA32F, l, l);
        const struct image_info img =
                image_info(GL_TEXTURE_BUFFER, GL_RGBA32F, l, l);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(common_hunk(img),
                       hunk("GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        imageStore(dst_img, IMAGE_ADDR(idx), RED);\n"
                            "        return x;\n"
                            "}\n"), NULL));
        bool ret = prog && init_common(grid, img, prog);
        uint32_t pixels[N][4];
        GLuint tex;

        init_pixels(img, pixels[0], 0, 1, 0, 1);

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, img.format->format,
                     l, l, 0, img.format->pixel_format,
                     image_base_type(img.format), pixels);

        /* Bind the image as pixel pack buffer simultaneously. */
        glBindBuffer(GL_PIXEL_PACK_BUFFER, get_buffer(1));

        /* First pass: render red to the image. */
        draw_grid(grid, prog);

        /* Barrier. */
        bar->run_barrier(GL_PIXEL_BUFFER_BARRIER_BIT);

        /* Second pass: use the image as pixel destination filling it
         * with green (write-after-write). */
        glGetTexImage(GL_TEXTURE_2D, 0, img.format->pixel_format,
                      img.format->pixel_type, 0);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

        /* Check that the resulting image is green. */
        ret &= piglit_check_gl_error(GL_NO_ERROR) &&
                check_img_green(img);

        glDeleteTextures(1, &tex);
        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_tex_update_raw(const struct image_barrier_info *bar, unsigned l)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_RGBA32F, l, l);
        const struct image_info img =
                image_info(GL_TEXTURE_2D, GL_RGBA32F, l, l);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(common_hunk(img),
                       hunk("GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        imageStore(dst_img, IMAGE_ADDR(idx), GREEN);\n"
                            "        return x;\n"
                            "}\n"), NULL));
        bool ret = prog && init_common(grid, img, prog);
        uint32_t pixels[N][4];

        glBindTexture(GL_TEXTURE_2D, get_texture(1));

        /* First pass: render green to the image. */
        draw_grid(grid, prog);

        /* Barrier. */
        bar->run_barrier(GL_TEXTURE_UPDATE_BARRIER_BIT);

        /* Read back the result (read-after-write). */
        glGetTexImage(GL_TEXTURE_2D, 0, img.format->pixel_format,
                      image_base_type(img.format), pixels);

        ret &= piglit_check_gl_error(GL_NO_ERROR) &&
                check_pixels(img, pixels[0], 0, 1, 0, 1);

        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_tex_update_waw(const struct image_barrier_info *bar, unsigned l)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_RGBA32F, l, l);
        const struct image_info img =
                image_info(GL_TEXTURE_2D, GL_RGBA32F, l, l);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(common_hunk(img),
                       hunk("GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        imageStore(dst_img, IMAGE_ADDR(idx),"
                            "                   (idx.y >= H / 2 ? GREEN : RED));\n"
                            "        return x;\n"
                            "}\n"), NULL));
        bool ret = prog && init_common(grid, img, prog);
        uint32_t pixels[N][4];

        init_pixels(set_image_size(img, l, l / 2, 1, 1),
                    pixels[0], 0, 1, 0, 1);

        glBindTexture(GL_TEXTURE_2D, get_texture(1));

        /* First pass: render red to the first half of the image,
         * green to the second half. */
        draw_grid(grid, prog);

        /* Barrier. */
        bar->run_barrier(GL_TEXTURE_UPDATE_BARRIER_BIT);

        /* Fill the first half with green (write-after-write). */
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, l, l / 2,
                        img.format->pixel_format, image_base_type(img.format),
                        pixels);

        /* Check that the resulting image is green. */
        ret &= piglit_check_gl_error(GL_NO_ERROR) &&
                check_img_green(img);

        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_buf_update_raw(const struct image_barrier_info *bar, unsigned l)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_RGBA32F, l, l);
        const struct image_info img =
                image_info(GL_TEXTURE_BUFFER, GL_RGBA32F, l, l);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(common_hunk(img),
                       hunk("GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        imageStore(dst_img, IMAGE_ADDR(idx), GREEN);\n"
                            "        return x;\n"
                            "}\n"), NULL));
        bool ret = prog && init_common(grid, img, prog);
        uint32_t pixels[N][4];

        glBindBuffer(GL_TEXTURE_BUFFER, get_buffer(1));

        /* First pass: render green to the image. */
        draw_grid(grid, prog);

        /* Barrier. */
        bar->run_barrier(GL_BUFFER_UPDATE_BARRIER_BIT);

        /* Read back the result (read-after-write). */
        glGetBufferSubData(GL_TEXTURE_BUFFER, 0,
                           4 * l * l * sizeof(uint32_t), pixels[0]);

        ret &= piglit_check_gl_error(GL_NO_ERROR) &&
                check_pixels(img, pixels[0], 0, 1, 0, 1);

        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_buf_update_waw(const struct image_barrier_info *bar, unsigned l)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_RGBA32F, l, l);
        const struct image_info img =
                image_info(GL_TEXTURE_BUFFER, GL_RGBA32F, l, l);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(common_hunk(img),
                       hunk("GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        imageStore(dst_img, IMAGE_ADDR(idx),"
                            "                   (idx.y >= H / 2 ? GREEN : RED));\n"
                            "        return x;\n"
                            "}\n"), NULL));
        bool ret = prog && init_common(grid, img, prog);
        uint32_t pixels[N][4];

        init_pixels(set_image_size(img, l * l / 2, 1, 1, 1),
                    pixels[0], 0, 1, 0, 1);
        glBindBuffer(GL_TEXTURE_BUFFER, get_buffer(1));

        /* First pass: render red to the first half of the image,
         * green to the second half. */
        draw_grid(grid, prog);

        /* Barrier. */
        bar->run_barrier(GL_BUFFER_UPDATE_BARRIER_BIT);

        /* Fill the first half with green (write-after-write). */
        glBufferSubData(GL_TEXTURE_BUFFER, 0, 4 * l * l / 2 * sizeof(uint32_t),
                        pixels);

        /* Check that the resulting image is green. */
        ret &= piglit_check_gl_error(GL_NO_ERROR) &&
                check_img_green(img);

        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_fb_raw(const struct image_barrier_info *bar, unsigned l)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_RGBA32F, l, l);
        const struct image_info img =
                image_info(GL_TEXTURE_2D, GL_RGBA32F, l, l);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(common_hunk(img),
                       hunk("GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        imageStore(dst_img, IMAGE_ADDR(idx), GREEN);\n"
                            "        return x;\n"
                            "}\n"), NULL));
        bool ret = prog && init_common(grid, img, prog);
        uint32_t pixels[N][4];
        GLuint fb;

        glGenFramebuffers(1, &fb);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);

        /* Bind the image as color attachment of the read framebuffer
         * simultaneously. */
        glFramebufferTexture(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             get_texture(1), 0);

        /* First pass: render green to the image. */
        draw_grid(grid, prog);

        /* Barrier. */
        bar->run_barrier(GL_FRAMEBUFFER_BARRIER_BIT);

        /* Read back and check the result from the read
         * framebuffer (read-after-write). */
        glReadPixels(0, 0, l, l, img.format->pixel_format,
                     image_base_type(img.format), pixels);

        ret &= piglit_check_gl_error(GL_NO_ERROR) &&
                check_pixels(img, pixels[0], 0, 1, 0, 1);

        glDeleteFramebuffers(1, &fb);
        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_fb_waw(const struct image_barrier_info *bar, unsigned l)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_RGBA32F, l, l);
        const struct image_info img =
                image_info(GL_TEXTURE_2D, GL_RGBA32F, l, l);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(common_hunk(img),
                       hunk("GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        if (pass == 1) {\n"
                            "                return GREEN;"
                            "        } else {\n"
                            "                imageStore(dst_img, IMAGE_ADDR(idx), RED);\n"
                            "                return RED;\n"
                            "        }\n"
                            "}\n"), NULL));
        bool ret = prog && init_common(grid, img, prog);
        GLuint fb;

        glGenFramebuffers(1, &fb);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);

        /* Bind the image as color attachment of the framebuffer
         * simultaneously. */
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             get_texture(1), 0);

        /* First pass: render red to the image. */
        ret &= draw_grid(grid, prog) &&

                /* Barrier. */
                bar->run_barrier(GL_FRAMEBUFFER_BARRIER_BIT) &&

                /* Second pass: render green to the framebuffer
                 * (write-after-write). */
                set_uniform_int(prog, "pass", 1) &&
                draw_grid(grid, prog) &&

                /* Check that the resulting image is green. */
                check_img_green(img);

        glDeleteFramebuffers(1, &fb);
        glDeleteProgram(prog);
        return ret;
}

static bool
setup_xfb_varying(GLuint prog, const char *varying)
{
        glTransformFeedbackVaryings(prog, 1, &varying, GL_INTERLEAVED_ATTRIBS);
        glLinkProgram(prog);

        return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
run_test_xfb_waw(const struct image_barrier_info *bar, unsigned l)
{
        const struct grid_info grid =
                grid_info(GL_VERTEX_SHADER, GL_RGBA32F, l, l);
        const struct image_info img =
                image_info(GL_TEXTURE_BUFFER, GL_RGBA32F, l, l);
        GLuint prog = generate_program(
                grid, GL_VERTEX_SHADER,
                concat(common_hunk(img),
                       hunk("GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        if (pass == 1) {\n"
                            "                return GREEN;\n"
                            "        } else {\n"
                            "                imageStore(dst_img, IMAGE_ADDR(idx), RED);\n"
                            "                return x;\n"
                            "        }\n"
                            "}\n"), NULL));
        bool ret = prog && setup_xfb_varying(prog, "vcolor") &&
                init_common(grid, img, prog);
        GLuint xfb;

        glGenTransformFeedbacks(1, &xfb);
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, xfb);

        /* Bind the image as transform feedback buffer
         * simultaneously. */
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, get_buffer(1));
        glBeginTransformFeedback(GL_POINTS);
        glPauseTransformFeedback();

        /* First pass: render red to the image. */
        draw_grid(grid, prog);

        /* Barrier. */
        bar->run_barrier(GL_TRANSFORM_FEEDBACK_BARRIER_BIT);

        /* Second pass: Write out the vcolor output to the transform
         * feedback buffer (write-after-write). */
        set_uniform_int(prog, "pass", 1);
        glResumeTransformFeedback();
        draw_grid(grid, prog);
        glEndTransformFeedback();

        /* Check that the resulting image is green. */
        ret &= piglit_check_gl_error(GL_NO_ERROR) &&
                check_img_green(img);

        glDeleteTransformFeedbacks(1, &xfb);
        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_atom_raw(const struct image_barrier_info *bar, unsigned l)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_RGBA32F, l, l);
        const struct image_info img =
                image_info(GL_TEXTURE_BUFFER, GL_RGBA32UI, l, l);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(common_hunk(img),
                       hunk("layout(binding=0, offset=0) uniform atomic_uint c[4];\n"
                            "\n"
                            "GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        if (pass == 1) {\n"
                            "                return GRID_T(atomicCounter(c[0]),"
                            "                              atomicCounter(c[1]),"
                            "                              atomicCounter(c[2]),"
                            "                              atomicCounter(c[3]));\n"
                            "        } else {\n"
                            "                imageStore(dst_img, IMAGE_ADDR(idx), GREEN);\n"
                            "                return x;\n"
                            "        }\n"
                            "}\n"), NULL));
        bool ret = prog && init_common(grid, img, prog);

        /* Bind the image as atomic counter buffer simultaneously. */
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, get_buffer(1));

        /* First pass: render green to the image. */
        ret &= draw_grid(grid, prog) &&

                /* Barrier. */
                bar->run_barrier(GL_ATOMIC_COUNTER_BARRIER_BIT) &&

                /* Second pass: Check that the atomic counters read
                 * back green (read-after-write). */
                set_uniform_int(prog, "pass", 1) &&
                draw_grid(grid, prog) &&
                check_fb_green(grid);

        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_atom_war(const struct image_barrier_info *bar, unsigned l)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_RGBA32F, l, l);
        const struct image_info img =
                image_info(GL_TEXTURE_BUFFER, GL_RGBA32UI, l, l);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(common_hunk(img),
                       hunk("layout(binding=0, offset=0) uniform atomic_uint c[4];\n"
                            "\n"
                            "GRID_T op(ivec2 idx, GRID_T x) {\n"
                            "        if (pass == 1) {\n"
                            "                atomicCounterIncrement(c[0]);"
                            "                atomicCounterIncrement(c[1]);"
                            "                atomicCounterIncrement(c[2]);"
                            "                atomicCounterIncrement(c[3]);\n"
                            "        } else {\n"
                            "                imageStore(dst_img, IMAGE_ADDR(idx),"
                            "                           imageLoad(src_img, IMAGE_ADDR(idx)));\n"
                            "        }\n"
                            "        return x;\n"
                            "}\n"), NULL));
        bool ret = prog && init_common(grid, img, prog);

        /* Bind the image as atomic counter buffer simultaneously. */
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, get_buffer(0));

        /* First pass: read back the source image. */
        ret &= draw_grid(grid, prog) &&

                /* Barrier. */
                bar->run_barrier(GL_ATOMIC_COUNTER_BARRIER_BIT) &&

                /* Second pass: Modify the source image using atomic
                 * counter increments (write-after-read). */
                set_uniform_int(prog, "pass", 1) &&
                draw_grid(grid, prog) &&

                /* Check that the read-back results from the first
                 * pass are green. */
                check_img_green(img);

        glDeleteProgram(prog);
        return ret;
}

static void
barrier_subtest(enum piglit_result *status, bool guard,
                bool (*run)(const struct image_barrier_info *, unsigned),
                const char *name, const struct image_barrier_info *bar,
                unsigned l)
{
        subtest(status, guard, run(bar, l) || bar->control_test,
                "%s/%s barrier test/%dx%d", name, bar->name, l, l);
}

void
piglit_init(int argc, char **argv)
{
        const bool quick = (argc >= 2 && !strcmp(argv[1], "--quick"));
        const struct image_barrier_info *bar;
        enum piglit_result status = PIGLIT_PASS;
        unsigned l;

        piglit_require_extension("GL_ARB_shader_image_load_store");
        piglit_require_extension("GL_ARB_shader_atomic_counters");

        for (l = 4; l <= L; l *= 4) {
                for (bar = image_barriers; bar->name; ++bar) {
                        if (quick && bar->control_test)
                                continue;

                        barrier_subtest(&status,
                                        get_image_stage(GL_VERTEX_SHADER),
                                        run_test_vertex_array_raw,
                                        "Vertex array/RaW", bar, l);

                        barrier_subtest(&status, true, run_test_element_array_raw,
                                        "Element array/RaW", bar, l);

                        barrier_subtest(&status, true, run_test_ubo_raw,
                                        "Uniform buffer/RaW", bar, l);

                        barrier_subtest(&status, true, run_test_tex_fetch_raw,
                                        "Texture fetch/RaW", bar, l);

                        barrier_subtest(&status, true, run_test_image_raw,
                                        "Image/RaW", bar, l);

                        barrier_subtest(&status, true, run_test_image_war,
                                        "Image/WaR", bar, l);

                        barrier_subtest(&status,
                                        (get_image_stage(GL_VERTEX_SHADER) &&
                                         piglit_is_extension_supported(
                                                 "GL_ARB_draw_indirect")),
                                        run_test_indirect_raw,
                                        "Indirect/RaW", bar, l);

                        barrier_subtest(&status, true, run_test_pixel_raw,
                                        "Pixel/RaW", bar, l);

                        barrier_subtest(&status, true, run_test_pixel_waw,
                                        "Pixel/WaW", bar, l);

                        barrier_subtest(&status, true, run_test_tex_update_raw,
                                        "Texture update/RaW", bar, l);

                        barrier_subtest(&status, true, run_test_tex_update_waw,
                                        "Texture update/WaW", bar, l);

                        barrier_subtest(&status, true, run_test_buf_update_raw,
                                        "Buffer update/RaW", bar, l);

                        barrier_subtest(&status, true, run_test_buf_update_waw,
                                        "Buffer update/WaW", bar, l);

                        barrier_subtest(&status, true, run_test_fb_raw,
                                        "Framebuffer/RaW", bar, l);

                        barrier_subtest(&status, true, run_test_fb_waw,
                                        "Framebuffer/WaW", bar, l);

                        barrier_subtest(&status,
                                        (get_image_stage(GL_VERTEX_SHADER) &&
                                         piglit_is_extension_supported(
                                                 "GL_ARB_transform_feedback2")),
                                        run_test_xfb_waw,
                                        "Transform feedback/WaW", bar, l);

                        barrier_subtest(&status, true, run_test_atom_raw,
                                        "Atomic counter/RaW", bar, l);

                        barrier_subtest(&status, true, run_test_atom_war,
                                        "Atomic counter/WaR", bar, l);
                }
        }

        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_FAIL;
}
