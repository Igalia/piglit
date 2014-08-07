/*
 * Copyright © 2013 Intel Corporation
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

/** @file unique-id.c
 *
 * Tests that concurrent execution of atomic operations on the same
 * counter yield unique values for each vertex or fragment shader
 * thread.
 */

#include "common.h"

#define L 256
#define N (L * L)

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_core_version = 31;

        config.window_width = 1;
        config.window_height = 1;
        config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static bool
check(int dx, int dy, uint32_t start_value, uint32_t end_value)
{
        const uint32_t base = MIN2(start_value, end_value);
        const uint32_t size = MAX2(start_value, end_value) - base;
        uint32_t pixels[L][L];
        uint32_t *frequency = malloc(size * sizeof(uint32_t));
        int x, y;

        memset(frequency, 0, size * sizeof(uint32_t));
        glReadPixels(0, 0, L, L, GL_RED_INTEGER, GL_UNSIGNED_INT, pixels);

        for (y = 0; y < L; y += dy) {
                for (x = 0; x < L; x += dx) {
                        uint32_t v = pixels[y][x] - base;

                        if (v >= size) {
                                printf("Probe value at (%d, %d)\n", x, y);
                                printf("  Observed: 0x%08x\n", v);
                                printf("  Value outside expected window.\n");
                                free(frequency);
                                return false;
                        }

                        if (size > 1 && frequency[v]++) {
                                printf("Probe value at (%d, %d)\n", x, y);
                                printf("  Observed: 0x%08x\n", v);
                                printf("  Value not unique.\n");
                                free(frequency);
                                return false;
                        }
                }
        }

        free(frequency);
        return true;
}

static bool
run_test_vertex(const char *op, uint32_t start_value, uint32_t end_value)
{
        const char *fs_source = "#version 140\n"
                "smooth in vec4 vcolor;\n"
                "out ivec4 fcolor;\n"
                "void main() {\n"
                "       fcolor.x = int(round(vcolor.x));\n"
                "}\n";
        const char *vs_template = "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "#define OP %s\n"
                "\n"
                "layout(binding = 0, offset = 0) uniform atomic_uint x;\n"
                "in vec4 piglit_vertex;\n"
                "smooth out vec4 vcolor;\n"
                "\n"
                "void main() {\n"
                "       uint y = OP(x);"
                "       vcolor.x = float(y);\n"
                "       gl_Position = piglit_vertex;\n"
                "}\n";
        char *vs_source;
        GLuint prog = glCreateProgram();
        int ret;

        ret = asprintf(&vs_source, vs_template, op);
        assert(ret > 0);

        ret = atomic_counters_compile(prog, GL_FRAGMENT_SHADER, fs_source) &&
                atomic_counters_compile(prog, GL_VERTEX_SHADER, vs_source) &&
                atomic_counters_draw_rect(prog, 1, &start_value) &&
                check(L - 1, L - 1, start_value, end_value);

        free(vs_source);
        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_fragment(const char *op, uint32_t start_value, uint32_t end_value)
{
        const char *fs_template = "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "#define OP %s\n"
                "\n"
                "layout(binding = 0, offset = 0) uniform atomic_uint x;\n"
                "out ivec4 fcolor;\n"
                "\n"
                "void main() {\n"
                "       fcolor.x = int(OP(x));\n"
                "}\n";
        char *fs_source;
        const char *vs_source = "#version 140\n"
                "in vec4 piglit_vertex;\n"
                "\n"
                "void main() {\n"
                "       gl_Position = piglit_vertex;\n"
                "}\n";
        GLuint prog = glCreateProgram();
        int ret;

        ret = asprintf(&fs_source, fs_template, op);
        assert(ret > 0);

        ret = atomic_counters_compile(prog, GL_FRAGMENT_SHADER, fs_source) &&
                atomic_counters_compile(prog, GL_VERTEX_SHADER, vs_source) &&
                atomic_counters_draw_rect(prog, 1, &start_value) &&
                check(1, 1, start_value, end_value);

        free(fs_source);
        glDeleteProgram(prog);
        return ret;
}

void
piglit_init(int argc, char **argv)
{
        GLuint fb, rb, buffer;
        enum piglit_result status = PIGLIT_PASS;

        piglit_require_extension("GL_ARB_shader_atomic_counters");

        glGenFramebuffers(1, &fb);
        glGenRenderbuffers(1, &rb);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
        glBindRenderbuffer(GL_RENDERBUFFER, rb);

        glViewport(0, 0, L, L);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_R32UI, L, L);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER, rb);

        glGenBuffers(1, &buffer);
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, buffer);

        atomic_counters_subtest(&status, GL_VERTEX_SHADER,
                                "Vertex atomic increment atomicity",
                                run_test_vertex,
                                "atomicCounterIncrement", 0, 4);

        atomic_counters_subtest(&status, GL_VERTEX_SHADER,
                                "Vertex atomic decrement atomicity",
                                run_test_vertex,
                                "atomicCounterDecrement", 4, 0);

        atomic_counters_subtest(&status, GL_VERTEX_SHADER,
                                "Vertex atomic read atomicity",
                                run_test_vertex,
                                "atomicCounter", 100, 101);

        atomic_counters_subtest(&status, GL_FRAGMENT_SHADER,
                                "Fragment atomic increment atomicity",
                                run_test_fragment,
                                "atomicCounterIncrement", 0, N);

        atomic_counters_subtest(&status, GL_FRAGMENT_SHADER,
                                "Fragment atomic decrement atomicity",
                                run_test_fragment,
                                "atomicCounterDecrement", N, 0);

        atomic_counters_subtest(&status, GL_FRAGMENT_SHADER,
                                "Fragment atomic read atomicity",
                                run_test_fragment,
                                "atomicCounter", 0, 1);

        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        /* UNREACHED */
        return PIGLIT_FAIL;
}
