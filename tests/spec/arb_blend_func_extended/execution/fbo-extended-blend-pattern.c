/*
 * Copyright (c) 2015 Intel Corporation
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

/**
 * @file fbo-extended-blend-pattern.c
 * @author Iago Toral Quiroga <itoral@igalia.com>
 *
 * On Intel hardware at least, SIMD16 dual source rendering requires handling
 * pixel data in two sets of 8 pixels each. Incorrect implementations may fail
 * to map correct colors for each pixel group (for example by using the color
 * for the first group as the color for the second group or viceversa). However,
 * tests that render using solid colors across the entire polygon won't catch
 * these cases (since in that case the color is the same for boths groups of
 * pixels).
 *
 * This test blends using a checker board pattern where each cell is
 * 10px wide and 10px tall. This makes it so that the two sets of 8 pixels
 * issued during SIMD16 operation pack different color data for the pixels
 * involved, enabling testing of correct behavior in that case.
 *
 * This only tests with one specific blend mode. There is no need to test
 * others, since the details of SIMD16 operation are independent of the
 * specific blend mode we use and general testing of the multiple blend modes
 * and parameters is already covered by the tests in fbo-extended-blend.c.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

   config.supports_gl_compat_version = 30;
   config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "fbo-extended-blend-pattern";

static GLint uniform_src0, uniform_src1, uniform_src2;

static const char *vs_text =
   "#version 130\n"
   "void main() {\n"
   "  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
   "}\n"
   ;

static const char *fs_text =
   "#version 130\n"
   "#extension GL_ARB_explicit_attrib_location: require\n"
   "uniform vec4 src0;\n"
   "uniform vec4 src1;\n"
   "uniform vec4 src2;\n"
   "layout(location = 0, index = 0) out vec4 col0;\n"
   "layout(location = 0, index = 1) out vec4 col1;\n"
   "void main() {\n"
   "   int a = int(gl_FragCoord.x) / 10;\n"
   "   int b = int(gl_FragCoord.y) / 10;\n"
   "   int c = int(mod(a + b, 2));\n"
   "   col0 = src0;\n"
   "   if (c == 0)\n"
   "      col1 = src1;\n"
   "   else\n"
   "      col1 = src2;\n"
   "}\n"
   ;

static void
check_error(int line)
{
   GLenum err = glGetError();
   if (err) {
      printf("%s: Unexpected error 0x%x at line %d\n", TestName, err, line);
      piglit_report_result(PIGLIT_FAIL);
   }
}

static void
blend(const float *src, const float *src1, const float *src2, const float *dst)
{
   glUniform4fv(uniform_src0, 1, dst);
   piglit_draw_rect(0, 0, piglit_width, piglit_height);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_COLOR, GL_SRC1_COLOR);
   glUniform4fv(uniform_src0, 1, src);
   glUniform4fv(uniform_src1, 1, src1);
   glUniform4fv(uniform_src2, 1, src2);
   piglit_draw_rect(0, 0, piglit_width, piglit_height);
   glDisable(GL_BLEND);
   glFinish();
}

enum piglit_result
piglit_display(void)
{
   static const GLfloat dest_color[4] = { 1.0, 1.0, 1.0, 1.0 };
   static const GLfloat test_color[4] = { 1.0, 0.0, 0.0, 1.0 };
   static const GLfloat test_color1[4] = { 0.0, 1.0, 0.0, 1.0 };
   static const GLfloat test_color2[4] = { 0.0, 0.0, 1.0, 1.0 };
   static const GLfloat expected1[4] = { 1.0, 1.0, 0.0, 1.0 };
   static const GLfloat expected2[4] = { 1.0, 0.0, 1.0, 1.0 };
   GLuint prog;

   glClearColor(0, 0, 0, 1);
   glClear(GL_COLOR_BUFFER_BIT);

   prog = piglit_build_simple_program(vs_text, fs_text);
   glUseProgram(prog);

   uniform_src0 = glGetUniformLocation(prog, "src0");
   uniform_src1 = glGetUniformLocation(prog, "src1");
   uniform_src2 = glGetUniformLocation(prog, "src2");

   blend(test_color, test_color1, test_color2, dest_color);
   check_error(__LINE__);

   if (!piglit_probe_rect_rgba(0, 0, 10, 10, expected1)) /* cell (0,0) */
      return PIGLIT_FAIL;
   if (!piglit_probe_rect_rgba(10, 0, 10, 10, expected2)) /* cell (1,0) */
      return PIGLIT_FAIL;
   if (!piglit_probe_rect_rgba(0, 10, 10, 10, expected2)) /* cell (0,1) */
      return PIGLIT_FAIL;
   if (!piglit_probe_rect_rgba(10, 10, 10, 10, expected1)) /* cell (1,1) */
      return PIGLIT_FAIL;

   return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
   piglit_require_extension("GL_ARB_blend_func_extended");
   piglit_require_extension("GL_ARB_explicit_attrib_location");
   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
