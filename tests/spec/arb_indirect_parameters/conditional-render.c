/*
 * Copyright (C) 2019 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 * This test checks that  GL_ARB_indirect_parameters works correctly
 * with GL_NV_conditional_render.
 *
 * Both extensions conditionally execute commands and on hardware
 * level they could use the same flag/mechanism to do this so
 * driver may fail to account their simultaneous usage.
 * This bug was found on i965.
 *
 * Bugzilla: https://bugs.freedesktop.org/show_bug.cgi?id=108759
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

   config.supports_gl_core_version = 32;
   config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
   config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static unsigned const point_count = 10;
static unsigned const pass_count = 4;
static GLuint prog_vs_atomic, prog_indr, draw_vao, q, atomic_bo;

static const char vs_indr[] =
   "#version 140\n"
   "in vec4 piglit_vertex;\n"
   "void main()\n"
   "{\n"
   "	gl_Position = piglit_vertex;\n"
   "}\n";

static const char fs_indr[] =
   "#version 140\n"
   "void main()\n"
   "{\n"
   "	gl_FragColor = vec4(1);\n"
   "}\n";

static const char vs_atom[] =
   "#version 140\n"
   "#extension GL_ARB_shader_atomic_counters: require\n"
   "\n"
   "layout(binding = 0, offset = 0) uniform atomic_uint counter;\n"
   "\n"
   "void main() {\n"
   "	atomicCounterIncrement(counter);\n"
   "	gl_Position = vec4(0);\n"
   "}\n";

void
piglit_init(int argc, char **argv)
{
   GLuint vbo, dbo, xfb_buf;
   const int in_param[] = { 4 };
   float point[] = { 0.5, 0.5 };
   const unsigned cmds[] = { 1, 1, 0, 0 };
   int vertex_counters;

   piglit_require_extension("GL_ARB_indirect_parameters");
   piglit_require_extension("GL_NV_conditional_render");
   piglit_require_extension("GL_ARB_shader_atomic_counters");

   glGetIntegerv(GL_MAX_VERTEX_ATOMIC_COUNTERS,
                 &vertex_counters);
   if(vertex_counters < 1) {
      fprintf(stderr, "Insufficient number of supported atomic "
              "counter buffers.\n");
      piglit_report_result(PIGLIT_SKIP);
   }

   prog_indr = piglit_build_simple_program(vs_indr, fs_indr);
   prog_vs_atomic = piglit_build_simple_program(vs_atom, NULL);

   glGenVertexArrays(1, &draw_vao);
   glBindVertexArray(draw_vao);

   glGenBuffers(1, &vbo);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(point), point, GL_STATIC_DRAW);

   char *indirect_draw_count_data = malloc(point_count * sizeof(cmds));
   char *dst = indirect_draw_count_data;
   for (unsigned i = 0; i < point_count; ++i) {
      memcpy(dst, cmds, sizeof(cmds));
      dst += sizeof(cmds);
   }

   glGenBuffers(1, &atomic_bo);
   glBindBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, atomic_bo, 0, 4);

   glGenBuffers(1, &dbo);
   glBindBuffer(GL_DRAW_INDIRECT_BUFFER, dbo);
   glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(cmds) * point_count,
                indirect_draw_count_data, GL_STATIC_DRAW);
   free(indirect_draw_count_data);

   glGenBuffers(1, &xfb_buf);
   glBindBuffer(GL_PARAMETER_BUFFER_ARB, xfb_buf);
   glBufferData(GL_PARAMETER_BUFFER_ARB, sizeof(in_param), in_param,
                GL_STATIC_DRAW);

   glGenQueries(1, &q);
}

enum piglit_result
run_subtest(bool inverted, bool query_result, unsigned expected_points_count)
{
   unsigned result;
   enum piglit_result subtest_result;
   char *inverted_s = inverted ? "Yes" : "No";
   char *query_result_s = query_result ? "Pass" : "Fail";
   const unsigned zero = 0;

   glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(zero), &zero,
                GL_STATIC_DRAW);

   glUseProgram(prog_indr);

   glBeginQuery(GL_ANY_SAMPLES_PASSED, q);

   if (query_result)
      piglit_draw_rect(-1, -1, 0.5, 0.5);

   glEndQuery(GL_ANY_SAMPLES_PASSED);

   glUseProgram(prog_vs_atomic);
   glEnable(GL_RASTERIZER_DISCARD);

   if (inverted)
      glBeginConditionalRenderNV(q, GL_QUERY_WAIT_INVERTED);
   else
      glBeginConditionalRenderNV(q, GL_QUERY_WAIT_NV);

   glMultiDrawArraysIndirectCountARB(GL_POINTS, 0, 0, 10, 0);

   glEndConditionalRenderNV();
   glDisable(GL_RASTERIZER_DISCARD);

   glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(result), &result);
   glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(zero), &zero,
                GL_STATIC_DRAW);

   subtest_result = result == expected_points_count ?
      PIGLIT_PASS : PIGLIT_FAIL;

   piglit_report_subtest_result(subtest_result,
                                "Query result: %s. Inverted mode: %s.  Points drawn: %d",
                                query_result_s, inverted_s, result);

   return subtest_result;
}

enum piglit_result
piglit_display(void)
{
   enum piglit_result piglit_test_state = PIGLIT_PASS;
   enum piglit_result piglit_subtest_state = PIGLIT_PASS;

   piglit_subtest_state = run_subtest(false, true, pass_count);
   piglit_merge_result(&piglit_test_state, piglit_subtest_state);

   piglit_subtest_state = run_subtest(true, false, pass_count);
   piglit_merge_result(&piglit_test_state, piglit_subtest_state);

   piglit_subtest_state = run_subtest(true, true, 0);
   piglit_merge_result(&piglit_test_state, piglit_subtest_state);

   piglit_subtest_state = run_subtest(false, false, 0);
   piglit_merge_result(&piglit_test_state, piglit_subtest_state);

   return piglit_test_state;
}
