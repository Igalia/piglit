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

/** @file function-argument.c
 *
 * Check that atomic operations work as expected on counters passed as
 * function arguments.
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
run_test(void)
{
        const char *fs_source = "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "layout(binding = 0, offset = 0) uniform atomic_uint x;\n"
                "layout(binding = 0, offset = 4) uniform atomic_uint y;\n"
                "out ivec4 fcolor;\n"
                "\n"
                "uint f(atomic_uint z) {\n"
                "       return atomicCounterIncrement(z);"
                "}\n"
                "\n"
                "void main() {\n"
                "       fcolor.x = int(f(y));\n"
                "}\n";
        const char *vs_source = "#version 140\n"
                "in vec4 piglit_vertex;\n"
                "\n"
                "void main() {\n"
                "       gl_Position = piglit_vertex;\n"
                "}\n";
        const uint32_t start_value[] = { 0, 0 };
        const uint32_t expected_value[] = { 0, N };
        GLuint prog = glCreateProgram();
        bool ret = atomic_counters_compile(prog, GL_FRAGMENT_SHADER, fs_source) &&
                atomic_counters_compile(prog, GL_VERTEX_SHADER, vs_source) &&
                atomic_counters_draw_rect(prog, 2, start_value) &&
                atomic_counters_probe_buffer(0, 2, expected_value);

        glDeleteProgram(prog);
        return ret;
}

void
piglit_init(int argc, char **argv)
{
        GLuint fb, rb, buffer;

        piglit_require_gl_version(31);
        piglit_require_extension("GL_ARB_shader_atomic_counters");

        glGenFramebuffers(1, &fb);
        glGenRenderbuffers(1, &rb);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
        glBindRenderbuffer(GL_RENDERBUFFER, rb);

        glRenderbufferStorage(GL_RENDERBUFFER, GL_R32UI, L, L);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER, rb);

        glGenBuffers(1, &buffer);
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, buffer);

	glViewport(0, 0, L, L);

        piglit_report_result(run_test() ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_FAIL; /* UNREACHED */
}
