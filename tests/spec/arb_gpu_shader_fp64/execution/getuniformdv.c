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
   "uniform dvec2 u1[2]; \n"
   "uniform dvec3 u2[4]; \n"
   "uniform dvec4 v[3]; \n"
   "uniform dmat2 m1; \n"
   "uniform dmat3 m2; \n"
   "uniform dmat4 m3[3]; \n"
   "uniform dmat2x3 m4; \n"
   "uniform dmat2x4 m5; \n"
   "uniform s1 s;\n"
   "uniform double d2; \n"
   "out vec4 vscolor; \n"
   "\n"
   "void main()\n"
   "{\n"
   "  gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
   "  dvec4 t = dvec4(s.a, s.b, s.c, s.d) * d1 + d2;\n"
   "  t += v[0]*m3[0] + v[1]*m3[1] + v[2]*m3[2]; \n"
   "  t.rb += u1[0]*m1 + u1[1] + u2[0]*m4 + v[0]*m5; \n"
   "  t.xyw += u2[0]*m2 + u2[1] + u2[2] + u2[3]; \n"
   "  vscolor = vec4(t); \n"
   "}\n";

static const char fs_text[] =
   "#version 150\n"
   "in vec4 vscolor;\n"
   "out vec4 fscolor;\n"
   "void main() { fscolor = vscolor; }";

#define UNIFORM_SIZE 9

static struct {
   char *name;
   char *altName;
   GLint expectedType;
   GLenum expectedSize;
} uniforms[] = {
   {NULL,    NULL, GL_DOUBLE,        1},  //default
   { "v",  "v[0]", GL_DOUBLE_VEC4,   3},
   {"u1", "u1[0]", GL_DOUBLE_VEC2,   2},
   {"u2", "u2[0]", GL_DOUBLE_VEC3,   4},
   {"m1",    NULL, GL_DOUBLE_MAT2,   1},
   {"m2",    NULL, GL_DOUBLE_MAT3,   1},
   {"m3", "m3[0]", GL_DOUBLE_MAT4,   3},
   {"m4",    NULL, GL_DOUBLE_MAT2x3, 1},
   {"m5",    NULL, GL_DOUBLE_MAT2x4, 1}};

enum piglit_result
piglit_display(void)
{
   /* never called */
   return PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
   bool piglit_pass = true;
   GLuint vs, fs, prog;
   GLint numUniforms, i;
   GLint expectedNum = 14;
   GLint loc_d1, loc_d2, loc_sa, loc_sd, loc_u1, loc_u2,
      loc_v1, loc_m1, loc_m2, loc_m3, loc_m4, loc_m5;
   GLdouble v[16];
   static const GLdouble saVals[1] = {15.0};
   static const GLdouble u1Vals[2] = {5.0, 8.0};
   static const GLdouble u2Vals[3] = {1.0, 1.0, 2.0};
   static const GLdouble vVals[4] = {30.0, 31.0, 32.0, 33.0};
   static const GLdouble m1Vals[4] = {1.0, 2.0, 3.0, 4.0};
   static const GLdouble m2Vals[9] = {1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 3.0, 3.0, 3.0};
   static const GLdouble m3Vals[16] = { 1.0, 2.0, 3.0, 4.0,
                                        5.0, 6.0, 7.0, 8.0,
                                        1.5, 2.5, 3.5, 4.5,
                                        5.5, 6.5, 7.5, 8.5};
   static const GLdouble m4Vals[6] = {15.0, 16.0,
                                      17.0, 18.0,
                                      19.0, 20.0};
   static const GLdouble m5Vals[8] = {10.0, 11.0,
                                      12.0, 13.0,
                                      14.0, 15.0,
                                      16.0, 17.0};

   piglit_require_extension("GL_ARB_gpu_shader_fp64");

   vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
   fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);
   prog = piglit_link_simple_program(vs, fs);

   glUseProgram(prog);

   glGetProgramiv(prog, GL_ACTIVE_UNIFORMS, &numUniforms);
   if (numUniforms != expectedNum) {
      printf("%s: incorrect number of uniforms (found %d, expected %d)\n",
             TestName, numUniforms, expectedNum);
      piglit_pass = false;
   }

   /* check types, sizes */
   for (i = 0; i < numUniforms; i++) {
      GLcharARB name[100];
      GLsizei len;
      GLint size, j;
      GLenum type;
      GLint loc;

      glGetActiveUniform(prog, i, sizeof(name), &len, &size, &type, name);
      loc = glGetUniformLocation(prog, name);

      if (loc < 0) {
         printf("%s: bad uniform location for %s: %d\n", TestName, name, loc);
         piglit_pass = false;
      }

      if (!piglit_automatic) {
         printf("%d: %s loc=%d size=%d type=0x%x\n", i, name, loc, size, type);
      }

      /* OpenGL ES 3.0 and OpenGL 4.2 require that the "[0]" be appended to
       * the name.  Earlier versions of the spec are ambiguous.  Accept either
       * name.
       */
      for (j = 1; j < UNIFORM_SIZE; j++) {
         if (strcmp(name, uniforms[j].name) == 0) {
            break;
         }
         if (uniforms[j].altName && strcmp(name, uniforms[j].altName) == 0) {
            break;
         }
      }
      if (j == UNIFORM_SIZE) {
         j = 0;
      }

      if (type != uniforms[j].expectedType) {
         printf("%s: wrong type for '%s' (found 0x%x, expected 0x%x)\n",
                TestName,
                uniforms[j].name ? uniforms[j].name : name,
                type, uniforms[j].expectedType);
         piglit_pass = false;
      }

      if (size != uniforms[j].expectedSize) {
         printf("%s: wrong size for '%s' (found %d, expected %d)\n",
                TestName,
                uniforms[j].name ? uniforms[j].name : name,
                size, uniforms[j].expectedSize);
         piglit_pass = false;
      }
   }

   /* Check setting/getting values */

   loc_d1 = glGetUniformLocation(prog, "d1");
   loc_d2 = glGetUniformLocation(prog, "d2");
   loc_sa = glGetUniformLocation(prog, "s.a");
   loc_sd = glGetUniformLocation(prog, "s.d");
   loc_u1 = glGetUniformLocation(prog, "u1[1]");
   loc_u2 = glGetUniformLocation(prog, "u2[0]");
   loc_v1 = glGetUniformLocation(prog, "v[1]");
   loc_m1 = glGetUniformLocation(prog, "m1");
   loc_m2 = glGetUniformLocation(prog, "m2");
   loc_m3 = glGetUniformLocation(prog, "m3[1]");
   loc_m4 = glGetUniformLocation(prog, "m4");
   loc_m5 = glGetUniformLocation(prog, "m5");

   glUniform1d(loc_d1, 5.0);
   glUniform1d(loc_d2, 10.0);
   glUniform1dv(loc_sa, 1, saVals);
   glUniform1d(loc_sd, 20.0);
   glUniform2dv(loc_u1, 1, u1Vals);
   glUniform3dv(loc_u2, 1, u2Vals);
   glUniform4dv(loc_v1, 1, vVals);
   glUniformMatrix2dv(loc_m1, 1, false, m1Vals);
   glUniformMatrix3dv(loc_m2, 1, true, m2Vals);
   glUniformMatrix4dv(loc_m3, 1, false, m3Vals);
   glUniformMatrix2x3dv(loc_m4, 1, false, m4Vals);
   glUniformMatrix2x4dv(loc_m5, 1, false, m5Vals);

   glGetUniformdv(prog, loc_d1, v);
   if (v[0] != 5.0) {
      printf("%s: wrong value for d1 (found %f, expected %f)\n",
             TestName, v[0], 5.0);
      piglit_pass = false;
   }

   glGetUniformdv(prog, loc_d2, v);
   if (v[0] != 10.0) {
      printf("%s: wrong value for d2 (found %f, expected %f)\n",
             TestName, v[0], 10.0);
      piglit_pass = false;
   }

   glGetUniformdv(prog, loc_sa, v);
   if (v[0] != 15.0) {
      printf("%s: wrong value for s.a (found %f, expected %f)\n",
             TestName, v[0], 15.0);
      piglit_pass = false;
   }

   glGetUniformdv(prog, loc_sd, v);
   if (v[0] != 20.0) {
      printf("%s: wrong value for s.d (found %f, expected %f)\n",
             TestName, v[0], 20.0);
      piglit_pass = false;
   }

   glGetUniformdv(prog, loc_u1, v);
   if (v[0] != 5.0  ||
       v[1] != 8.0) {
      printf("%s: wrong value for u1[0] (found %g,%g, expected %g,%g)\n",
             TestName, v[0], v[1], 5.0, 8.0);
      piglit_pass = false;
   }

   glGetUniformdv(prog, loc_u2, v);
   if (v[0] != 1.0 ||
       v[1] != 1.0 ||
       v[2] != 2.0) {
      printf("%s: wrong value for u2[0] (found %g,%g,%g, expected %g,%g,%g)\n",
             TestName, v[0], v[1], v[2], 1.0, 1.0, 2.0);
      piglit_pass = false;
   }

   glGetUniformdv(prog, loc_v1, v);
   if (v[0] != 30.0 ||
       v[1] != 31.0 ||
       v[2] != 32.0 ||
       v[3] != 33.0) {
      printf("%s: wrong value for v[1] (found %g,%g,%g,%g, expected %g,%g,%g,%g)\n",
             TestName, v[0], v[1], v[2], v[3], 30.0, 31.0, 32.0, 33.0);
      piglit_pass = false;
   }

   glGetUniformdv(prog, loc_m1, v);
   if (v[0] != 1.0 ||
       v[1] != 2.0 ||
       v[2] != 3.0 ||
       v[3] != 4.0) {
      printf("%s: wrong value for m1 (found %g,%g,%g,%g, expected %g,%g,%g,%g)\n",
             TestName, v[0], v[1], v[2], v[3], 1.0, 2.0, 3.0, 4.0);
      piglit_pass = false;
   }

   glGetUniformdv(prog, loc_m2, v);
   if (v[0] != 1.0 ||
       v[1] != 2.0 ||
       v[2] != 3.0 ||
       v[3] != 1.0 ||
       v[4] != 2.0 ||
       v[5] != 3.0 ||
       v[6] != 1.0 ||
       v[7] != 2.0 ||
       v[8] != 3.0) {
      printf("%s: wrong value for m2 (found %g,%g,%g,%g,%g,%g,%g,%g,%g, expected %g,%g,%g,%g,%g,%g,%g,%g,%g)\n",
             TestName,
             v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8],
             1.0, 2.0, 3.0, 1.0, 2.0, 3.0, 1.0, 2.0, 3.0);
      piglit_pass = false;
   }

   glGetUniformdv(prog, loc_m3, v);
   if (v[0] != 1.0 || v[1] != 2.0 || v[2] != 3.0 || v[3] != 4.0 ||
       v[4] != 5.0 || v[5] != 6.0 || v[6] != 7.0 || v[7] != 8.0 ||
       v[8] != 1.5 || v[9] != 2.5 || v[10] != 3.5 || v[11] != 4.5 ||
       v[12] != 5.5 || v[13] != 6.5 || v[14] != 7.5 || v[15] != 8.5) {
      printf("%s: wrong value for m3 (found %g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g, "
             "expected %g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g)\n",
             TestName,
             v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8],
             v[9], v[10], v[11], v[12], v[13], v[14], v[15],
             1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0,
             1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5);
      piglit_pass = false;
   }

   glGetUniformdv(prog, loc_m4, v);
   if (v[0] != 15.0 || v[1] != 16.0 ||
       v[2] != 17.0 || v[3] != 18.0 ||
       v[4] != 19.0 || v[5] != 20.0) {
      printf("%s: wrong value for m4 (found %g,%g,%g,%g,%g,%g, "
             "expected %g,%g,%g,%g,%g,%g)\n",
             TestName,
             v[0], v[1], v[2], v[3], v[4], v[5], v[6],
             15.0, 16.0, 17.0, 18.0, 19.0, 20.0);
      piglit_pass = false;
   }

   glGetUniformdv(prog, loc_m5, v);
   if (v[0] != 10.0 || v[1] != 11.0 ||
       v[2] != 12.0 || v[3] != 13.0 ||
       v[4] != 14.0 || v[5] != 15.0 ||
       v[6] != 16.0 || v[7] != 17.0) {
      printf("%s: wrong value for m5 (found %g,%g,%g,%g,%g,%g,%g,%g, "
             "expected %g,%g,%g,%g,%g,%g,%g,%g)\n",
             TestName,
             v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
             10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0);
      piglit_pass = false;
   }

   loc_u1 = glGetUniformLocation(prog, "u1[0]");
   loc_u2 = glGetUniformLocation(prog, "u2[2]");
   loc_v1 = glGetUniformLocation(prog, "v[0]");

   glUniform2d(loc_u1, 12.0, 14.0);
   glUniform3d(loc_u2, 20.0, 20.0, 15.0);
   glUniform4d(loc_v1, 2.0, 3.0, 4.0, 5.0);
   glGetUniformdv(prog, loc_u1, v);
   if (v[0] != 12.0 ||
       v[1] != 14.0) {
      printf("%s: wrong value for u1[0] (found %g,%g, expected %g,%g)\n",
             TestName, v[0], v[1], 12.0, 14.0);
      piglit_pass = false;
   }

   glGetUniformdv(prog, loc_u2, v);
   if (v[0] != 20.0 ||
       v[1] != 20.0 ||
       v[2] != 15.0) {
      printf("%s: wrong value for u2[2] (found %g,%g,%g, expected %g,%g,%g)\n",
             TestName, v[0], v[1], v[2], 20.0, 20.0, 15.0);
      piglit_pass = false;
   }

   glGetUniformdv(prog, loc_v1, v);
   if (v[0] != 2.0 ||
       v[1] != 3.0 ||
       v[2] != 4.0 ||
       v[3] != 5.0) {
     printf("%s: wrong value for v[0] (found %g,%g,%g,%g, expected %g,%g,%g,%g)\n",
            TestName, v[0], v[1], v[2], v[3], 2.0, 3.0, 4.0, 5.0);
     piglit_pass = false;
   }

   piglit_report_result(piglit_pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
