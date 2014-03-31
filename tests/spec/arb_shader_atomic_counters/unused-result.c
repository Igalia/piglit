/*
 * Copyright © 2013-2014 Intel Corporation
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

/** @file unused-result.c
 *
 * Tests that the atomic built-in functions have the expected effects
 * on the buffer, even if the result of the atomic op is unused within
 * the shader.
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_core_version = 31;

        config.window_width = 1;
        config.window_height = 1;
        config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static bool
run_test_vertex(void)
{
        const char *fs_source = "#version 140\n"
                "out ivec4 fcolor;\n"
                "void main() {\n"
                "       fcolor = ivec4(0);\n"
                "}\n";
        const char *vs_source = "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "layout(binding = 0, offset = 0) uniform atomic_uint x;\n"
                "in vec4 piglit_vertex;\n"
                "\n"
                "void main() {\n"
                "       atomicCounterIncrement(x);\n"
                "       gl_Position = piglit_vertex;\n"
                "}\n";
        const uint32_t start_buffer[] = { 0x0 };
        const uint32_t expected_buffer[] = { 0x1 };
        const uint32_t expected_color[] = { 0x0, 0x0, 0x0, 0x0 };
        GLuint prog = glCreateProgram();
        bool ret =
                atomic_counters_compile(prog, GL_FRAGMENT_SHADER, fs_source) &&
                atomic_counters_compile(prog, GL_VERTEX_SHADER, vs_source) &&
                atomic_counters_draw_point(prog, 1, start_buffer) &&
                piglit_probe_rect_rgba_uint(0, 0, 1, 1, expected_color) &&
                atomic_counters_probe_buffer(0, 1, expected_buffer);

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
                "layout(binding = 0, offset = 0) uniform atomic_uint x;\n"
                "\n"
                "void main() {\n"
                "       atomicCounterIncrement(x);\n"

                "       fcolor = ivec4(0);\n"
                "}\n";
        const char *vs_source = "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "in vec4 piglit_vertex;\n"
                "\n"
                "void main() {\n"
                "       gl_Position = piglit_vertex;\n"
                "}\n";
        const uint32_t start_buffer[] = { 0x0 };
        const uint32_t expected_buffer[] = { 0x1 };
        const uint32_t expected_color[] = { 0x0, 0x0, 0x0, 0x0 };
        GLuint prog = glCreateProgram();
        bool ret =
                atomic_counters_compile(prog, GL_FRAGMENT_SHADER, fs_source) &&
                atomic_counters_compile(prog, GL_VERTEX_SHADER, vs_source) &&
                atomic_counters_draw_point(prog, 1, start_buffer) &&
                piglit_probe_rect_rgba_uint(0, 0, 1, 1, expected_color) &&
                atomic_counters_probe_buffer(0, 1, expected_buffer);

        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_geometry(void)
{
        const char *fs_source = "#version 140\n"
                "out ivec4 fcolor;\n"
                "void main() {\n"
                "       fcolor = ivec4(0);\n"
                "}\n";
        const char *gs_source = "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "layout(points) in;\n"
                "layout(points, max_vertices=1) out;\n"
                "\n"
                "layout(binding = 0, offset = 0) uniform atomic_uint x;\n"
                "\n"
                "void main() {\n"
                "       gl_Position = gl_in[0].gl_Position;\n"
                "       atomicCounterIncrement(x);\n"
                "       EmitVertex();\n"
                "}\n";
        const char *vs_source = "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "in vec4 piglit_vertex;\n"
                "\n"
                "void main() {\n"
                "       gl_Position = piglit_vertex;\n"
                "}\n";
        const uint32_t start_buffer[] = { 0x0 };
        const uint32_t expected_buffer[] = { 0x1 };
        const uint32_t expected_color[] = { 0x0, 0x0, 0x0, 0x0 };
        GLuint prog = glCreateProgram();
        bool ret =
                atomic_counters_compile(prog, GL_FRAGMENT_SHADER, fs_source) &&
                atomic_counters_compile(prog, GL_GEOMETRY_SHADER, gs_source) &&
                atomic_counters_compile(prog, GL_VERTEX_SHADER, vs_source) &&
                atomic_counters_draw_point(prog, 1, start_buffer) &&
                piglit_probe_rect_rgba_uint(0, 0, 1, 1, expected_color) &&
                atomic_counters_probe_buffer(0, 1, expected_buffer);

        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_tess_control(void)
{
        const char *fs_source = "#version 140\n"
                "out ivec4 fg;\n"
                "void main() {\n"
                "       fg = ivec4(0);\n"
                "}\n";
        const char *tes_source = "#version 140\n"
                "#extension GL_ARB_tessellation_shader : enable\n"
                "\n"
                "layout(triangles, point_mode) in;\n"
                "\n"
                "\n"
                "void main() {\n"
                "       gl_Position = gl_in[0].gl_Position * gl_TessCoord.x +\n"
                "               gl_in[1].gl_Position * gl_TessCoord.y +\n"
                "               gl_in[2].gl_Position * gl_TessCoord.z;\n"
                "       \n"
                "}\n";
        const char *tcs_source = "#version 140\n"
                "#extension GL_ARB_tessellation_shader : enable\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "layout(vertices=3) out;\n"
                "\n"
                "layout(binding = 0, offset = 0) uniform atomic_uint x;\n"
                "\n"
                "void main() {\n"
                "       if (gl_InvocationID == 0) {\n"
                "               gl_TessLevelInner[0] = 1;\n"
                "               \n"
                "               gl_TessLevelOuter[0] = 1;\n"
                "               gl_TessLevelOuter[1] = 1;\n"
                "               gl_TessLevelOuter[2] = 1;\n"
                "               \n"
                "               atomicCounterIncrement(x);\n"
                "       }\n"
                "       \n"
                "       gl_out[gl_InvocationID].gl_Position =\n"
                "               gl_in[gl_InvocationID].gl_Position;\n"
                "}\n";
        const char *vs_source = "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "in vec4 piglit_vertex;\n"
                "\n"
                "void main() {\n"
                "       gl_Position = piglit_vertex;\n"
                "}\n";
        const uint32_t start_buffer[] = { 0x0 };
        const uint32_t expected_buffer[] = { 0x1 };
        const uint32_t expected_color[] = { 0x0, 0x0, 0x0, 0x0 };
        GLuint prog = glCreateProgram();
        bool ret =
                atomic_counters_compile(prog, GL_FRAGMENT_SHADER, fs_source) &&
                atomic_counters_compile(prog, GL_TESS_EVALUATION_SHADER,
                                        tes_source) &&
                atomic_counters_compile(prog, GL_TESS_CONTROL_SHADER,
                                        tcs_source) &&
                atomic_counters_compile(prog, GL_VERTEX_SHADER, vs_source) &&
                atomic_counters_draw_patch(prog, 1, start_buffer) &&
                piglit_probe_rect_rgba_uint(0, 0, 1, 1, expected_color) &&
                atomic_counters_probe_buffer(0, 1, expected_buffer);

        glDeleteProgram(prog);
        return ret;
}

static bool
run_test_tess_evaluation(void)
{
        const char *fs_source = "#version 140\n"
                "out ivec4 fg;\n"
                "void main() {\n"
                "       fg = ivec4(0);\n"
                "}\n";
        const char *tes_source = "#version 140\n"
                "#extension GL_ARB_tessellation_shader : enable\n"
                "\n"
                "layout(triangles, point_mode) in;\n"
                "\n"
                "layout(binding = 0, offset = 0) uniform atomic_uint x;\n"
                "\n"
                "void main() {\n"
                "       gl_Position = gl_in[0].gl_Position * gl_TessCoord.x +\n"
                "               gl_in[1].gl_Position * gl_TessCoord.y +\n"
                "               gl_in[2].gl_Position * gl_TessCoord.z;\n"
                "       \n"
                "       if (gl_TessCoord.z == 1.0) {\n"
                "               atomicCounterIncrement(x);\n"
                "       }\n"
                "}\n";
        const char *tcs_source = "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "#extension GL_ARB_tessellation_shader : enable\n"
                "\n"
                "layout(vertices=3) out;\n"
                "\n"
                "void main() {\n"
                "       if (gl_InvocationID == 0) {\n"
                "               gl_TessLevelInner[0] = 1;\n"
                "               \n"
                "               gl_TessLevelOuter[0] = 1;\n"
                "               gl_TessLevelOuter[1] = 1;\n"
                "               gl_TessLevelOuter[2] = 1;\n"
                "       }\n"
                "       \n"
                "       gl_out[gl_InvocationID].gl_Position =\n"
                "               gl_in[gl_InvocationID].gl_Position;\n"
                "}\n";
        const char *vs_source = "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "in vec4 piglit_vertex;\n"
                "\n"
                "void main() {\n"
                "       gl_Position = piglit_vertex;\n"
                "}\n";
        const uint32_t start_buffer[] = { 0x0 };
        const uint32_t expected_buffer[] = { 0x1 };
        const uint32_t expected_color[] = { 0x0, 0x0, 0x0, 0x0 };
        GLuint prog = glCreateProgram();
        bool ret =
                atomic_counters_compile(prog, GL_FRAGMENT_SHADER, fs_source) &&
                atomic_counters_compile(prog, GL_TESS_EVALUATION_SHADER,
                                        tes_source) &&
                atomic_counters_compile(prog, GL_TESS_CONTROL_SHADER,
                                        tcs_source) &&
                atomic_counters_compile(prog, GL_VERTEX_SHADER, vs_source) &&
                atomic_counters_draw_patch(prog, 1, start_buffer) &&
                piglit_probe_rect_rgba_uint(0, 0, 1, 1, expected_color) &&
                atomic_counters_probe_buffer(0, 1, expected_buffer);

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

        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32UI, 1, 1);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER, rb);

        glGenBuffers(1, &buffer);
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, buffer);

        atomic_counters_subtest(&status, GL_FRAGMENT_SHADER,
                                "Fragment shader atomic built-in semantics",
                                run_test_fragment);

        atomic_counters_subtest(&status, GL_VERTEX_SHADER,
                                "Vertex shader atomic built-in semantics",
                                run_test_vertex);

        atomic_counters_subtest(&status, GL_GEOMETRY_SHADER,
                                "Geometry shader atomic built-in semantics",
                                run_test_geometry);

        atomic_counters_subtest(&status, GL_TESS_CONTROL_SHADER,
                                "Tessellation control shader atomic built-in "
                                "semantics",
                                run_test_tess_control);

        atomic_counters_subtest(&status, GL_TESS_EVALUATION_SHADER,
                                "Tessellation evaluation shader atomic built-in "
                                "semantics",
                                run_test_tess_evaluation);

        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        /* UNREACHED */
        return PIGLIT_FAIL;
}
