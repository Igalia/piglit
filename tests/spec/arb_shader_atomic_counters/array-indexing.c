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

/** @file array-indexing.c
 *
 * Check that dynamically uniform indexing of an atomic counter array
 * works as expected.
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_core_version = 31;

        config.window_width = 1;
        config.window_height = 1;
        config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static bool
set_uniform_int(GLuint prog, const char *name, int value)
{
        int loc;

        glLinkProgram(prog);
        glUseProgram(prog);

        loc = glGetUniformLocation(prog, name);
        if (loc < 0) {
                fprintf(stderr, "Failed to get location for uniform '%s'.\n",
                        name);
                return false;
        }

        glUniform1i(loc, value);

        return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
run_test_vertex(void)
{
        const char *fs_source = "#version 140\n"
                "flat in ivec4 vcolor;\n"
                "out ivec4 fcolor;\n"
                "void main() {\n"
                "       fcolor = vcolor;\n"
                "}\n";
        const char *vs_source = "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "layout(binding = 0, offset = 4) uniform atomic_uint x[3];\n"
                "in vec4 position;\n"
                "flat out ivec4 vcolor;\n"
                "uniform int index;\n"
                "\n"
                "void main() {\n"
                "       vcolor.x = int(atomicCounterIncrement(x[1 + index]));\n"
                "       vcolor.y = int(atomicCounterIncrement(x[0 + index]));\n"
                "       vcolor.z = int(atomicCounterIncrement(x[1 + index]));\n"
                "       vcolor.w = int(atomicCounterIncrement(x[0 + index]));\n"
                "       gl_Position = position;\n"
                "}\n";
        const unsigned int start[] = { 1, 2, 4, 8 };
        const unsigned int expected[] = { 8, 4, 9, 5 };
        GLuint prog = glCreateProgram();
        bool ret =
                atomic_counters_compile(prog, GL_FRAGMENT_SHADER, fs_source) &&
                atomic_counters_compile(prog, GL_VERTEX_SHADER, vs_source) &&
                set_uniform_int(prog, "index", 1) &&
                atomic_counters_draw_point(prog, sizeof(start), start) &&
                piglit_probe_rect_rgba_uint(0, 0, 1, 1, expected);

        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_fragment(void)
{
        const char *fs_source = "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "out ivec4 fcolor;\n"
                "uniform int index;\n"
                "layout(binding = 0, offset = 4) uniform atomic_uint x[3];\n"
                "\n"
                "void main() {\n"
                "       fcolor.x = int(atomicCounterIncrement(x[1 + index]));\n"
                "       fcolor.y = int(atomicCounterIncrement(x[0 + index]));\n"
                "       fcolor.z = int(atomicCounterIncrement(x[1 + index]));\n"
                "       fcolor.w = int(atomicCounterIncrement(x[0 + index]));\n"
                "}\n";
        const char *vs_source = "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "in vec4 position;\n"
                "\n"
                "void main() {\n"
                "       gl_Position = position;\n"
                "}\n";
        const uint32_t start[] = { 1, 2, 4, 8 };
        const unsigned int expected[] = { 8, 4, 9, 5 };
        GLuint prog = glCreateProgram();
        bool ret =
                atomic_counters_compile(prog, GL_FRAGMENT_SHADER, fs_source) &&
                atomic_counters_compile(prog, GL_VERTEX_SHADER, vs_source) &&
                set_uniform_int(prog, "index", 1) &&
                atomic_counters_draw_point(prog, sizeof(start), start) &&
                piglit_probe_rect_rgba_uint(0, 0, 1, 1, expected);

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

        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32UI, 1, 1);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER, rb);

        glGenBuffers(1, &buffer);
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, buffer);
}

enum piglit_result
piglit_display(void)
{
        enum piglit_result status = PIGLIT_PASS;

        atomic_counters_subtest(&status, GL_FRAGMENT_SHADER,
                                "Fragment atomic counter array access",
                                run_test_fragment);

        atomic_counters_subtest(&status, GL_VERTEX_SHADER,
                                "Vertex atomic counter array access",
                                run_test_vertex);

        piglit_report_result(status);
        return status;
}
