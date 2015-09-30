/*
 * Copyright 2011 VMware, Inc.
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
 * Asst. gl[Get]Uniformdv tests.
 * based on getunifom02.c from Brian Paul.
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static char *TestName = "getuniformdv";

static const char vs_text[] =
   "#version 150\n"
   "#extension GL_ARB_gpu_shader_fp64 : require\n"
   "struct s1 { \n"
   "   double a, b, c, d; \n"
   "}; \n"
   "uniform double d1; \n"
   "uniform dvec4 v[3]; \n"
   "uniform s1 s;\n"
   "uniform double d2; \n"
   "uniform float f1; \n"
   "uniform uint ui1; \n"
   "uniform int i1; \n"
   "uniform bool b1; \n"
   "out vec4 color; \n"
   "\n"
   "void main()\n"
   "{\n"
   "  gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
   "  dvec4 t = dvec4(s.a, s.b, s.c, s.d) * d1 + d2 + double(f1) + double(ui1) + double(i1) + double(b1);\n"
   "  t += v[0] + v[1] + v[2]; \n"
   "  color = vec4(t); \n"
   "}\n";

static const char fs_text[] =
   "#version 150\n"
   "in vec4 color;\n"
   "void main() { gl_FragColor = color; }";

enum piglit_result
piglit_display(void)
{
   /* never called */
   return PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
   GLuint vs, fs, prog;
   GLint numUniforms, i;
   GLint expectedNum = 11;
   GLint loc_d1, loc_d2, loc_sa, loc_sd, loc_v1, loc_f1, loc_ui1, loc_i1, loc_b1;
   GLdouble v[4];
   GLfloat f[4];
   GLuint ui[4];
   GLint i_val[4];

   static const GLdouble vVals[4] = {30.0, 31.0, 32.0, 33.0};
   
   piglit_require_extension("GL_ARB_gpu_shader_fp64");

   vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
   fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);
   prog = piglit_link_simple_program(vs, fs);

   glUseProgram(prog);

   glGetProgramiv(prog, GL_ACTIVE_UNIFORMS, &numUniforms);
   if (numUniforms != expectedNum) {
      printf("%s: incorrect number of uniforms (found %d, expected %d)\n",
             TestName, numUniforms, expectedNum);
      piglit_report_result(PIGLIT_FAIL);
   }

   /* check types, sizes */
   for (i = 0; i < numUniforms; i++) {
      GLcharARB name[100];
      GLsizei len;
      GLint size, expectedSize;
      GLenum type, expectedType;
      GLint loc;

      glGetActiveUniform(prog, i, sizeof(name), &len, &size, &type, name);
      loc = glGetUniformLocation(prog, name);

      if (loc < 0) {
         printf("%s: bad uniform location for %s: %d\n", TestName, name, loc);
         piglit_report_result(PIGLIT_FAIL);
      }

      if (!piglit_automatic) {
         printf("%d: %s loc=%d size=%d type=0x%x\n", i, name, loc, size, type);
      }

      /* OpenGL ES 3.0 and OpenGL 4.2 require that the "[0]" be appended to
       * the name.  Earlier versions of the spec are ambiguous.  Accept either
       * name.
       */
      if (strcmp(name, "v") == 0 || strcmp(name, "v[0]") == 0) {
         expectedType = GL_DOUBLE_VEC4;
         expectedSize = 3;
      } else if (name[0] == 'd' || (name[0] == 's')) {
         expectedType = GL_DOUBLE;
         expectedSize = 1;
      } else if (name[0] == 'f') {
         expectedType = GL_FLOAT;
         expectedSize = 1;
      } else if (name[0] == 'i') {
         expectedType = GL_INT;
         expectedSize = 1;
      } else if (name[0] == 'u') {
         expectedType = GL_UNSIGNED_INT;
         expectedSize = 1;
      } else if (name[0] == 'b') {
	 expectedType = GL_BOOL;
         expectedSize = 1;
      }

      if (type != expectedType) {
         printf("%s: wrong type for 'v' (found 0x%x, expected 0x%x)\n",
                TestName, type, expectedType);
         piglit_report_result(PIGLIT_FAIL);
      }

      if (size != expectedSize) {
         printf("%s: wrong size for 'v' (found %d, expected %d)\n",
                TestName, size, expectedSize);
         piglit_report_result(PIGLIT_FAIL);
      }
   }

   /* Check setting/getting values */

   loc_d1 = glGetUniformLocation(prog, "d1");
   loc_d2 = glGetUniformLocation(prog, "d2");
   loc_sa = glGetUniformLocation(prog, "s.a");
   loc_sd = glGetUniformLocation(prog, "s.d");
   loc_v1 = glGetUniformLocation(prog, "v[1]");
   loc_f1 = glGetUniformLocation(prog, "f1");
   loc_ui1 = glGetUniformLocation(prog, "ui1");
   loc_i1 = glGetUniformLocation(prog, "i1");
   loc_b1 = glGetUniformLocation(prog, "b1");
   glUniform1d(loc_d1, 5.0);
   glUniform1d(loc_d2, 10.0);
   glUniform1d(loc_sa, 15.0);
   glUniform1d(loc_sd, 20.0);
   glUniform4dv(loc_v1, 1, vVals);
   glUniform1f(loc_f1, 40.0);
   glUniform1ui(loc_ui1, 100);
   glUniform1i(loc_i1, -100);

   glUniform1d(loc_b1, 1);
   if (!piglit_check_gl_error(GL_INVALID_OPERATION))
      piglit_report_result(PIGLIT_FAIL);

   glUniform1i(loc_b1, 1);

   /* double getters on doubles */
   glGetUniformdv(prog, loc_d1, v);
   if (v[0] != 5.0) {
      printf("%s: wrong value for d1 (found %f, expected %f)\n",
             TestName, v[0], 5.0);
      piglit_report_result(PIGLIT_FAIL);
   }

   glGetUniformdv(prog, loc_d2, v);
   if (v[0] != 10.0) {
      printf("%s: wrong value for d2 (found %f, expected %f)\n",
             TestName, v[0], 10.0);
      piglit_report_result(PIGLIT_FAIL);
   }

   glGetUniformdv(prog, loc_sa, v);
   if (v[0] != 15.0) {
      printf("%s: wrong value for s.a (found %f, expected %f)\n",
             TestName, v[0], 15.0);
      piglit_report_result(PIGLIT_FAIL);
   }

   glGetUniformdv(prog, loc_sd, v);
   if (v[0] != 20.0) {
      printf("%s: wrong value for s.d (found %f, expected %f)\n",
             TestName, v[0], 20.0);
      piglit_report_result(PIGLIT_FAIL);
   }

   glGetUniformdv(prog, loc_v1, v);
   if (v[0] != 30.0 ||
       v[1] != 31.0 ||
       v[2] != 32.0 ||
       v[3] != 33.0) {
      printf("%s: wrong value for v[1] (found %g,%g,%g,%g, expected %g,%g,%g,%g)\n",
             TestName, v[0], v[1], v[2], v[3], 30.0, 31.0, 32.0, 33.0);
      piglit_report_result(PIGLIT_FAIL);
   }

   /* use non-double getters on doubles */
   glGetUniformfv(prog, loc_d1, f);
   if (f[0] != 5.0) {
      printf("%s: wrong value for d1 (found %f, expected %f)\n",
             TestName, v[0], 5.0);
      piglit_report_result(PIGLIT_FAIL);
   }

   glGetUniformuiv(prog, loc_d1, ui);
   if (ui[0] != 5) {
      printf("%s: wrong value for d1 (found %f, expected %f)\n",
             TestName, v[0], 5.0);
      piglit_report_result(PIGLIT_FAIL);
   }

   glGetUniformiv(prog, loc_d1, i_val);
   if (i_val[0] != 5) {
      printf("%s: wrong value for d1 (found %f, expected %f)\n",
             TestName, v[0], 5.0);
      piglit_report_result(PIGLIT_FAIL);
   }

   /* use double getters on non-doubles */
   glGetUniformdv(prog, loc_f1, v);
   if (v[0] != 40.0) {
      printf("%s: wrong value for f1 (found %f, expected %f)\n",
             TestName, v[0], 40.0);
      piglit_report_result(PIGLIT_FAIL);
   }
   glGetUniformdv(prog, loc_ui1, v);
   if (v[0] != 100.0) {
      printf("%s: wrong value for ui1 (found %f, expected %f)\n",
             TestName, v[0], 100.0);
      piglit_report_result(PIGLIT_FAIL);
   }
   glGetUniformdv(prog, loc_i1, v);
   if (v[0] != -100.0) {
      printf("%s: wrong value for i1 (found %f, expected %f)\n",
             TestName, v[0], -100.0);
      piglit_report_result(PIGLIT_FAIL);
   }
   glGetUniformdv(prog, loc_b1, v);
   if (v[0] != 1.0) {
      printf("%s: wrong value for b1 (found %f, expected %f)\n",
             TestName, v[0], 1.0);
      piglit_report_result(PIGLIT_FAIL);
   }

   piglit_report_result(PIGLIT_PASS);
}
