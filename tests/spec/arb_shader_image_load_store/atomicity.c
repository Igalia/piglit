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

/** @file atomicity.c
 *
 * Test the atomicity of the read-modify-write image operations
 * defined by the spec.  The subtests can be classified in two groups:
 *
 * The ones that test bitwise operations (imageAtomicAnd(),
 * imageAtomicOr(), imageAtomicXor()) and imageAtomicExchange() work
 * by using an image as bitmap which is written to by a large number
 * of shader invocations in parallel, each of them will use a bitwise
 * built-in to flip an individual bit on the image.  If the
 * read-modify-write operation is implemented atomically no write will
 * overwrite any concurrent write supposed to flip a different bit in
 * the same dword, so the whole bitmap will be inverted when the
 * rendering completes.
 *
 * The remaining subtests (imageAtomicAdd(), imageAtomicMin(),
 * imageAtomicMax(), imageAtomicCompSwap()) operate on a single 32-bit
 * location of the image which is accessed concurrently from all
 * shader invocations.  In each case a function written in terms of
 * one of the built-ins is guaranteed to return a unique 32-bit value
 * for each concurrent invocation as long as the read-modify-write
 * operation is implemented atomically.  The way in which this is
 * achieved differs for each built-in and is described in more detail
 * below.
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
init_image(const struct image_info img, uint32_t v)
{
        uint32_t pixels[N];

        return init_pixels(img, pixels, v, 0, 0, 0) &&
                upload_image(img, 0, pixels);
}

static bool
check_fb_unique(const struct grid_info grid)
{
        uint32_t pixels[H][W];
        int frequency[N] = { 0 };
        int i, j;

        if (!download_result(grid, pixels[0]))
                return false;

        for (i = 0; i < W; ++i) {
                for (j = 0; j < H; ++j) {
                        if (frequency[pixels[j][i] % N]++) {
                                printf("Probe value at (%d, %d)\n", i, j);
                                printf("  Observed: 0x%08x\n", pixels[j][i]);
                                printf("  Value not unique.\n");
                                return false;
                        }
                }
        }

        return true;
}

static bool
check_image_const(const struct image_info img, unsigned n, uint32_t v)
{
        uint32_t pixels[N];

        return download_image(img, 0, pixels) &&
                check_pixels(set_image_size(img, n, 1, 1, 1),
                             pixels, v, 0, 0, 0);
}

/**
 * Test skeleton: Init image to \a init_value, run the provided shader
 * \a op, check that the first \a check_sz pixels of the image equal
 * \a check_value and optionally check that the resulting fragment
 * values on the framebuffer are unique.
 */
static bool
run_test(uint32_t init_value, unsigned check_sz, uint32_t check_value,
         bool check_unique, const char *op)
{
        const struct grid_info grid =
                grid_info(GL_FRAGMENT_SHADER, GL_R32UI, W, H);
        const struct image_info img =
                image_info(GL_TEXTURE_1D, GL_R32UI, W, H);
        GLuint prog = generate_program(
                grid, GL_FRAGMENT_SHADER,
                concat(image_hunk(img, ""),
                       hunk("volatile uniform IMAGE_T img;\n"),
                       hunk(op), NULL));
        bool ret = prog &&
                init_fb(grid) &&
                init_image(img, init_value) &&
                set_uniform_int(prog, "img", 0) &&
                draw_grid(grid, prog) &&
                check_image_const(img, check_sz, check_value) &&
                (!check_unique || check_fb_unique(grid));

        glDeleteProgram(prog);
        return ret;
}

void
piglit_init(int argc, char **argv)
{
        enum piglit_result status = PIGLIT_PASS;

        piglit_require_extension("GL_ARB_shader_image_load_store");

        /*
         * If imageAtomicAdd() is atomic the return values obtained
         * from each call are guaranteed to be unique.
         */
        subtest(&status, true,
                run_test(0, 1, N, true,
                         "GRID_T op(ivec2 idx, GRID_T x) {\n"
                         "       return GRID_T("
                         "          imageAtomicAdd(img, IMAGE_ADDR(ivec2(0)), 1u),"
                         "          0, 0, 1);\n"
                         "}\n"),
                "imageAtomicAdd");

        /*
         * Call imageAtomicMin() on a fixed location from within a
         * loop passing the most recent guess of the counter value
         * decremented by one.
         *
         * If no race occurs the counter will be decremented by one
         * and we're done, if another thread updates the counter in
         * parallel imageAtomicMin() has no effect since
         * min(x-n, x-1) = x-n for n >= 1, so we update our guess and
         * repeat.  In the end we obtain a unique counter value for
         * each fragment if the read-modify-write operation is atomic.
         */
        subtest(&status, true,
                run_test(0xffffffff, 1, 0xffffffff - N, true,
                        "GRID_T op(ivec2 idx, GRID_T x) {\n"
                        "       uint old, v = 0xffffffffu;"
                        "\n"
                        "       do {\n"
                        "               old = v;\n"
                        "               v = imageAtomicMin(img, IMAGE_ADDR(ivec2(0)),"
                        "                                  v - 1u);\n"
                        "       } while (v != old);\n"
                        "\n"
                        "       return GRID_T(v, 0, 0, 1);\n"
                        "}\n"),
                "imageAtomicMin");

        /*
         * Use imageAtomicMax() on a fixed location to increment a
         * counter as explained above for imageAtomicMin().  The
         * atomicity of the built-in guarantees that the obtained
         * values will be unique for each fragment.
         */
        subtest(&status, true,
                run_test(0, 1, N, true,
                        "GRID_T op(ivec2 idx, GRID_T x) {\n"
                        "       uint old, v = 0u;"
                        "\n"
                        "       do {\n"
                        "               old = v;\n"
                        "               v = imageAtomicMax(img, IMAGE_ADDR(ivec2(0)),"
                        "                                  v + 1u);\n"
                        "       } while (v != old);\n"
                        "\n"
                        "       return GRID_T(v, 0, 0, 1);\n"
                        "}\n"),
                "imageAtomicMax");

        /*
         * Use imageAtomicAnd() to flip individual bits of a bitmap
         * atomically.  The atomicity of the built-in guarantees that
         * all bits will be clear on termination.
         */
        subtest(&status, true,
                run_test(0xffffffff, N / 32, 0, false,
                        "GRID_T op(ivec2 idx, GRID_T x) {\n"
                        "       int i = IMAGE_ADDR(idx);\n"
                        "       uint m = ~(1u << (i % 32));\n"
                        "\n"
                        "       imageAtomicAnd(img, i / 32, m);\n"
                        "\n"
                        "       return GRID_T(0, 0, 0, 1);\n"
                        "}\n"),
                "imageAtomicAnd");

        /*
         * Use imageAtomicOr() to flip individual bits of a bitmap
         * atomically.  The atomicity of the built-in guarantees that
         * all bits will be set on termination.
         */
        subtest(&status, true,
                run_test(0, N / 32, 0xffffffff, false,
                        "GRID_T op(ivec2 idx, GRID_T x) {\n"
                        "       int i = IMAGE_ADDR(idx);\n"
                        "       uint m = (1u << (i % 32));\n"
                        "\n"
                        "       imageAtomicOr(img, i / 32, m);\n"
                        "\n"
                        "       return GRID_T(0, 0, 0, 1);\n"
                        "}\n"),
                "imageAtomicOr");

        /*
         * Use imageAtomicXor() to flip individual bits of a bitmap
         * atomically.  The atomicity of the built-in guarantees that
         * all bits will have been inverted on termination.
         */
        subtest(&status, true,
                run_test(0x55555555, N / 32, 0xaaaaaaaa, false,
                        "GRID_T op(ivec2 idx, GRID_T x) {\n"
                        "       int i = IMAGE_ADDR(idx);\n"
                        "       uint m = (1u << (i % 32));\n"
                        "\n"
                        "       imageAtomicXor(img, i / 32, m);\n"
                        "\n"
                        "       return GRID_T(0, 0, 0, 1);\n"
                        "}\n"),
                "imageAtomicXor");

        /*
         * Use imageAtomicExchange() to flip individual bits of a
         * bitmap atomically.  The atomicity of the built-in
         * guarantees that all bits will be set on termination.
         */
        subtest(&status, true,
                run_test(0, N / 32, 0xffffffff, false,
                        "GRID_T op(ivec2 idx, GRID_T x) {\n"
                        "       int i = IMAGE_ADDR(idx);\n"
                        "       uint m = (1u << (i % 32));\n"
                        "       uint old = 0u;\n"
                        "\n"
                        "       do {\n"
                        "               m |= old;\n"
                        "               old = imageAtomicExchange("
                        "                       img, i / 32, m);\n"
                        "       } while ((old & ~m) != 0u);\n"
                        "\n"
                        "       return GRID_T(0, 0, 0, 1);\n"
                        "}\n"),
                "imageAtomicExchange");

#if 0
        /*
         * Use imageAtomicExchange() on a fixed location to increment
         * a counter, implementing a sort of spin-lock.
         *
         * The counter has two states: locked (0xffffffff) and
         * unlocked (any other value).  While locked a single thread
         * owns the value of the counter, increments its value and
         * puts it back to the same location, atomically releasing the
         * counter.  The atomicity of the built-in guarantees that the
         * obtained values will be unique for each fragment.
         *
         * Unlike the classic spin-lock implementation, this uses the
         * same atomic call to perform either a lock or an unlock
         * operation depending on the current thread state.  This is
         * critical to avoid a dead-lock situation on machines where
         * neighboring threads have limited parallelism (e.g. share
         * the same instruction pointer).
         *
         * This could lead to a different kind of dead-lock on devices
         * that simulate concurrency by context-switching threads
         * based on some sort of priority queue: If there is a
         * possibility for a low-priority thread to acquire the lock
         * and be preempted before the end of the critical section, it
         * will prevent higher priority threads from making progress
         * while the higher priority threads may prevent the
         * lock-owning thread from being scheduled again and releasing
         * the lock.
         *
         * Disabled for now because the latter dead-lock can easily be
         * reproduced on current Intel hardware where it causes a GPU
         * hang.  It seems to work fine on nVidia though, it would be
         * interesting to see if it works on other platforms.
         */
        subtest(&status, true,
                run_test(0, 1, N, true,
                        "GRID_T op(ivec2 idx, GRID_T x) {\n"
                        "       uint p = 0xffffffffu, v = 0xffffffffu;\n"
                        "\n"
                        "       do {\n"
                        "               if (p != 0xffffffffu)\n"
                        "                       v = p++;\n"
                        "               p = imageAtomicExchange("
                        "                  img, IMAGE_ADDR(ivec2(0)), p);\n"
                        "       } while (v == 0xffffffffu);\n"
                        "\n"
                        "       return GRID_T(v, 0, 0, 1);\n"
                        "}\n"),
                "imageAtomicExchange (locking)");
#endif

        /*
         * Use imageAtomicCompSwap() on a fixed location from within a
         * loop passing the most recent guess of the counter value as
         * comparison value and the same value incremented by one as
         * argument.  The atomicity of the built-in guarantees that
         * the obtained values will be unique for each fragment.
         */
        subtest(&status, true,
                run_test(0, 1, N, true,
                        "GRID_T op(ivec2 idx, GRID_T x) {\n"
                        "       uint old, v = 0u;"
                        "\n"
                        "       do {\n"
                        "               old = v;\n"
                        "               v = imageAtomicCompSwap("
                        "                  img, IMAGE_ADDR(ivec2(0)), v, v + 1u);\n"
                        "       } while (v != old);\n"
                        "\n"
                        "       return GRID_T(v, 0, 0, 1);\n"
                        "}\n"),
                "imageAtomicCompSwap");

        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_FAIL;
}
