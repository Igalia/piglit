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
 * Asst. gl[Get]Uniformfv tests.
 * Brian Paul, 27 May 2011
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static char *TestName = "getuniform-02";

static const char vs_text[] =
   "struct s1 { \n"
   "   float a, b, c, d; \n"
   "}; \n"
   "uniform float f1; \n"
   "uniform vec4 v[3]; \n"
   "uniform s1 s;\n"
   "uniform float f2; \n"
   "\n"
   "void main()\n"
   "{\n"
   "  gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
   "  vec4 t = vec4(s.a, s.b, s.c, s.d) * f1 + f2;\n"
   "  t += v[0] + v[1] + v[2]; \n"
   "  gl_FrontColor = t; \n"
   "}\n";

enum piglit_result
piglit_display(void)
{
   /* never called */
   return PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
   GLuint vs, prog;
   GLint numUniforms, i;
   GLint expectedNum = 7;
   GLint loc_f1, loc_f2, loc_sa, loc_sd, loc_v1;
   GLfloat v[4];
   static const GLfloat vVals[4] = {30.0, 31.0, 32.0, 33.0};
   
   piglit_require_vertex_shader();
   piglit_require_fragment_shader();

   vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
   prog = piglit_link_simple_program(vs, 0);

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

      glGetActiveUniform(prog,
				 i, sizeof(name), &len, &size, &type, name);
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
         expectedType = GL_FLOAT_VEC4_ARB;
         expectedSize = 3;
      }
      else {
         expectedType = GL_FLOAT;
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

   loc_f1 = glGetUniformLocation(prog, "f1");
   loc_f2 = glGetUniformLocation(prog, "f2");
   loc_sa = glGetUniformLocation(prog, "s.a");
   loc_sd = glGetUniformLocation(prog, "s.d");
   loc_v1 = glGetUniformLocation(prog, "v[1]");

   glUniform1f(loc_f1, 5.0);
   glUniform1f(loc_f2, 10.0);
   glUniform1f(loc_sa, 15.0);
   glUniform1f(loc_sd, 20.0);
   glUniform4fv(loc_v1, 1, vVals);

   glGetUniformfv(prog, loc_f1, v);
   if (v[0] != 5.0) {
      printf("%s: wrong value for f1 (found %f, expected %f)\n",
             TestName, v[0], 5.0);
      piglit_report_result(PIGLIT_FAIL);
   }

   glGetUniformfv(prog, loc_f2, v);
   if (v[0] != 10.0) {
      printf("%s: wrong value for f2 (found %f, expected %f)\n",
             TestName, v[0], 10.0);
      piglit_report_result(PIGLIT_FAIL);
   }

   glGetUniformfv(prog, loc_sa, v);
   if (v[0] != 15.0) {
      printf("%s: wrong value for s.a (found %f, expected %f)\n",
             TestName, v[0], 15.0);
      piglit_report_result(PIGLIT_FAIL);
   }

   glGetUniformfv(prog, loc_sd, v);
   if (v[0] != 20.0) {
      printf("%s: wrong value for s.d (found %f, expected %f)\n",
             TestName, v[0], 20.0);
      piglit_report_result(PIGLIT_FAIL);
   }

   glGetUniformfv(prog, loc_v1, v);
   if (v[0] != 30.0 ||
       v[1] != 31.0 ||
       v[2] != 32.0 ||
       v[3] != 33.0) {
      printf("%s: wrong value for v[1] (found %g,%g,%g,%g, expected %g,%g,%g,%g)\n",
             TestName, v[0], v[1], v[2], v[3], 30.0, 31.0, 32.0, 33.0);
      piglit_report_result(PIGLIT_FAIL);
   }

   piglit_report_result(PIGLIT_PASS);
}
