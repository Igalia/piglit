/*
 * Copyright (c) 2011 VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL VMWARE AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * Tests rendering with vertex arrays when neither GL_VERTEX_ARRAY nor generic
 * attribute 0 array is enabled.  Test legacy/fixed-function, GLSL and ARB_vp.
 *
 * Brian Paul
 * May 2011
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "pos-array";

static const GLfloat VertexData[] = {
   /* positions */
   -1, -1,
   1, -1,
   1, 1,
   -1, 1,
   /* colors */
   1, 0, 0,
   1, 0, 0,
   0, 0, 1,
   0, 0, 1
};


static GLuint
setup_vbo(void)
{
   GLuint buf;

   glGenBuffersARB(1, &buf);
   glBindBufferARB(GL_ARRAY_BUFFER_ARB, buf);
   glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(VertexData), VertexData,
                   GL_DYNAMIC_DRAW_ARB);
   return buf;
}


/** Test legacy/fixed-function vertex array drawing */
static GLboolean
test_fixedfunc_arrays(void)
{
   static const GLfloat expected[4] = {0.5, 0.0, 0.5, 1.0};
   static const GLfloat black[4] = {0.0, 0.0, 0.0, 0.0};
   GLuint buf;
   GLboolean p,pass = GL_TRUE;

   buf = setup_vbo();
   glBindBufferARB(GL_ARRAY_BUFFER_ARB, buf);

   /*
    * Draw with conventional arrays.
    */
   {
      glVertexPointer(2, GL_FLOAT, 2 * sizeof(GLfloat), (void *) 0);
      glColorPointer(3, GL_FLOAT, 3 * sizeof(GLfloat),
                     (void *) (8 * sizeof(GLfloat)));
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_COLOR_ARRAY);

      glClear(GL_COLOR_BUFFER_BIT);
      glDrawArrays(GL_QUADS, 0, 4);

      p = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, expected);
      piglit_present_results();
      if (!p) {
         printf("%s: failed when drawing with ", TestName);
         printf("conventional vertex arrays\n");
         pass = GL_FALSE;
      }

      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_COLOR_ARRAY);
   }

   /*
    * Draw with generic array[0]=position
    * XXX this should only work when the driver aliases conventional
    * vertex attributes with the generic attributes (as w/ NVIDIA).
    * XXX check for that and enable this code some day.
    */
   if (0) {
      GLuint attrib = 0;
      glVertexAttribPointerARB(attrib, 2, GL_FLOAT, GL_FALSE,
                               2 * sizeof(GLfloat), (void *) 0);
      glColorPointer(3, GL_FLOAT, 3 * sizeof(GLfloat),
                     (void *) (8 * sizeof(GLfloat)));
      glEnableVertexAttribArrayARB(attrib);
      glEnableClientState(GL_COLOR_ARRAY);

      glClear(GL_COLOR_BUFFER_BIT);
      glDrawArrays(GL_QUADS, 0, 4);

      p = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, expected);
      piglit_present_results();
      if (!p) {
         printf("%s: failed when drawing with ", TestName);
         printf("generic array [%u]\n", attrib);
         pass = GL_FALSE;
      }

      glDisableVertexAttribArrayARB(attrib);
      glDisableClientState(GL_COLOR_ARRAY);
   }

   /*
    * Draw without GL_VERTEX or GENERIC[0] array set (should NOT draw)
    */
   {
      GLuint attrib = 3;
      glVertexAttribPointerARB(attrib, 2, GL_FLOAT, GL_FALSE,
                               2 * sizeof(GLfloat), (void *) 0);
      glColorPointer(3, GL_FLOAT, 3 * sizeof(GLfloat),
                     (void *) (8 * sizeof(GLfloat)));
      glEnableVertexAttribArrayARB(attrib);
      glEnableClientState(GL_COLOR_ARRAY);

      glClear(GL_COLOR_BUFFER_BIT);
      glDrawArrays(GL_QUADS, 0, 4);

      p = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, black);
      piglit_present_results();
      if (!p) {
         printf("%s: failed when drawing with ", TestName);
         printf("generic array [%u]\n", attrib);
         pass = GL_FALSE;
      }

      glDisableVertexAttribArrayARB(attrib);
      glDisableClientState(GL_COLOR_ARRAY);
   }

   piglit_present_results();

   glDeleteBuffersARB(1, &buf);

   return pass;
}


/** Test drawing with GLSL shaders */
static GLboolean
test_glsl_arrays(void)
{
   static const char *vertShaderText =
      "attribute vec4 color, pos; \n"
      "varying vec4 colorVar; \n"
      "void main() \n"
      "{ \n"
      "   colorVar = color; \n"
      "   gl_Position = gl_ModelViewProjectionMatrix * pos; \n"
      "} \n";

   static const char *fragShaderText =
      "varying vec4 colorVar; \n"
      "void main() \n"
      "{ \n"
      "   gl_FragColor = colorVar; \n"
      "} \n";

   static const GLfloat expected[4] = {0.5, 0.0, 0.5, 1.0};
   GLuint buf;
   GLboolean p, pass = GL_TRUE;
   GLint posAttrib, colorAttrib;
   GLuint vertShader, fragShader, program;

   buf = setup_vbo();
   glBindBufferARB(GL_ARRAY_BUFFER_ARB, buf);

   vertShader = piglit_compile_shader_text(GL_VERTEX_SHADER, vertShaderText);
   fragShader = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fragShaderText);
   program = piglit_link_simple_program(vertShader, fragShader);

   glUseProgram(program);

   /*
    * Draw with compiler-assigned attribute locations
    */
   {
      posAttrib = glGetAttribLocation(program, "pos");
      colorAttrib = glGetAttribLocation(program, "color");

      if (0)
         printf("%s: GLSL posAttrib = %d  colorAttrib = %d\n",
                TestName, posAttrib, colorAttrib);

      glVertexAttribPointerARB(posAttrib, 2, GL_FLOAT, GL_FALSE,
                               2 * sizeof(GLfloat), (void *) 0);
      glVertexAttribPointerARB(colorAttrib, 3, GL_FLOAT, GL_FALSE,
                               3 * sizeof(GLfloat),
                               (void *) (8 * sizeof(GLfloat)));
      glEnableVertexAttribArrayARB(posAttrib);
      glEnableVertexAttribArrayARB(colorAttrib);

      glClear(GL_COLOR_BUFFER_BIT);
      glDrawArrays(GL_QUADS, 0, 4);

      p = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, expected);
      piglit_present_results();
      if (!p) {
         printf("%s: failed when drawing with ", TestName);
         printf("compiler-assigned attribute locations\n");
         pass = GL_FALSE;
      }

      glDisableVertexAttribArrayARB(posAttrib);
      glDisableVertexAttribArrayARB(colorAttrib);
   }

   /*
    * Draw with user-defined attribute bindings, not using 0.
    */
   {
      posAttrib = 5;
      colorAttrib = 7;

      glBindAttribLocation(program, posAttrib, "pos");
      glBindAttribLocation(program, colorAttrib, "color");

      glLinkProgram(program);

      glVertexAttribPointerARB(posAttrib, 2, GL_FLOAT, GL_FALSE,
                               2 * sizeof(GLfloat), (void *) 0);
      glVertexAttribPointerARB(colorAttrib, 3, GL_FLOAT, GL_FALSE,
                               3 * sizeof(GLfloat),
                               (void *) (8 * sizeof(GLfloat)));
      glEnableVertexAttribArrayARB(posAttrib);
      glEnableVertexAttribArrayARB(colorAttrib);

      glClear(GL_COLOR_BUFFER_BIT);
      glDrawArrays(GL_QUADS, 0, 4);

      p = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, expected);
      piglit_present_results();
      if (!p) {
         printf("%s: failed when drawing with ", TestName);
         printf("user-assigned attribute locations\n");
         pass = GL_FALSE;
      }

      glDisableVertexAttribArrayARB(posAttrib);
      glDisableVertexAttribArrayARB(colorAttrib);
   }

   glDeleteShader(vertShader);
   glDeleteProgram(program);
   glDeleteBuffersARB(1, &buf);

   return pass;
}


/**
 * Test drawing with GLSL shaders and no vertex arrays.
 * Use a vertex shader with a hard-coded vertex position.
 */
static GLboolean
test_glsl_no_arrays(void)
{
   static const char *noVertexVertShaderText =
      "varying vec4 colorVar; \n"
      "void main() \n"
      "{ \n"
      "   colorVar = vec4(1.0, 1.0, 0.0, 1.0); \n"
      "   gl_Position = vec4(0.0, 0.0, 0.0, 1.0); \n"
      "} \n";

   static const char *fragShaderText =
      "varying vec4 colorVar; \n"
      "void main() \n"
      "{ \n"
      "   gl_FragColor = colorVar; \n"
      "} \n";

   static const GLfloat expected[4] = {1.0, 1.0, 0.0, 1.0};
   GLboolean p, pass = GL_TRUE;
   GLuint vertShader, fragShader, program;

   vertShader = piglit_compile_shader_text(GL_VERTEX_SHADER,
                                           noVertexVertShaderText);
   fragShader = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fragShaderText);
   program = piglit_link_simple_program(vertShader, fragShader);

   glUseProgram(program);

   glClear(GL_COLOR_BUFFER_BIT);
   glPointSize(3.0);
   glDrawArrays(GL_POINTS, 0, 1);
   glPointSize(1.0);

   p = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, expected);
   piglit_present_results();
   if (!p) {
      printf("%s: failed when drawing with GLSL and no vertex arrays\n",
             TestName);
      pass = GL_FALSE;
   }

   glDeleteShader(vertShader);
   glDeleteProgram(program);

   return pass;
}


/** Test drawing with GL_ARB_vertex_program */
static GLboolean
test_arbvp_arrays(void)
{
   /* Use legacy vertex array/attribute */
   static const char *legacyVertProgramText =
      "!!ARBvp1.0 \n"
      "ATTRIB iPos = vertex.position;\n"
      "OUTPUT oPos = result.position;\n"
      "PARAM mvp[4] = { state.matrix.mvp };\n"
      "DP4 oPos.x, mvp[0], iPos;\n"
      "DP4 oPos.y, mvp[1], iPos;\n"
      "DP4 oPos.z, mvp[2], iPos;\n"
      "DP4 oPos.w, mvp[3], iPos;\n"
      "MOV result.color, vertex.color;\n"
      "END";

   /* Use generic vertex array/attribute[0] */
   static const char *generic0VertProgramText =
      "!!ARBvp1.0 \n"
      "ATTRIB iPos = vertex.attrib[0];\n"
      "OUTPUT oPos = result.position;\n"
      "PARAM mvp[4] = { state.matrix.mvp };\n"
      "DP4 oPos.x, mvp[0], iPos;\n"
      "DP4 oPos.y, mvp[1], iPos;\n"
      "DP4 oPos.z, mvp[2], iPos;\n"
      "DP4 oPos.w, mvp[3], iPos;\n"
      "MOV result.color, vertex.color;\n"
      "END";

   /* Use generic vertex array/attribute[6] */
   static const char *generic6VertProgramText =
      "!!ARBvp1.0 \n"
      "ATTRIB iPos = vertex.attrib[6];\n"
      "OUTPUT oPos = result.position;\n"
      "PARAM mvp[4] = { state.matrix.mvp };\n"
      "DP4 oPos.x, mvp[0], iPos;\n"
      "DP4 oPos.y, mvp[1], iPos;\n"
      "DP4 oPos.z, mvp[2], iPos;\n"
      "DP4 oPos.w, mvp[3], iPos;\n"
      "MOV result.color, vertex.color;\n"
      "END";

   static const char *fragProgramText =
      "!!ARBfp1.0 \n"
      "MOV result.color, fragment.color;\n"
      "END";

   static const GLfloat expected[4] = {0.5, 0.0, 0.5, 1.0};
   GLuint buf;
   GLboolean p, pass = GL_TRUE;
   GLuint vertProg, fragProg;

   buf = setup_vbo();
   glBindBufferARB(GL_ARRAY_BUFFER_ARB, buf);

   fragProg = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, fragProgramText);
   glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fragProg);

   glEnable(GL_FRAGMENT_PROGRAM_ARB);
   glEnable(GL_VERTEX_PROGRAM_ARB);

   /*
    * Draw with pos in conventional arrays.
    */
   {
      vertProg = piglit_compile_program(GL_VERTEX_PROGRAM_ARB,
                                        legacyVertProgramText);
      glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vertProg);
      glVertexPointer(2, GL_FLOAT, 2 * sizeof(GLfloat), (void *) 0);
      glColorPointer(3, GL_FLOAT, 3 * sizeof(GLfloat),
                     (void *) (8 * sizeof(GLfloat)));
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_COLOR_ARRAY);

      glClear(GL_COLOR_BUFFER_BIT);
      glDrawArrays(GL_QUADS, 0, 4);

      p = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, expected);
      piglit_present_results();
      if (!p) {
         printf("%s: failed when drawing with ", TestName);
         printf("ARB VP and conventional vertex arrays\n");
         pass = GL_FALSE;
      }

      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_COLOR_ARRAY);
      glDeleteProgramsARB(1, &vertProg);
   }

   /*
    * Draw with pos in generic array[0].
    */
   {
      GLuint attrib = 0;
      vertProg = piglit_compile_program(GL_VERTEX_PROGRAM_ARB,
                                        generic0VertProgramText);
      glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vertProg);
      glVertexAttribPointerARB(attrib, 2, GL_FLOAT, GL_FALSE,
                               2 * sizeof(GLfloat), (void *) 0);
      glColorPointer(3, GL_FLOAT, 3 * sizeof(GLfloat),
                     (void *) (8 * sizeof(GLfloat)));
      glEnableVertexAttribArrayARB(attrib);
      glEnableClientState(GL_COLOR_ARRAY);

      glClear(GL_COLOR_BUFFER_BIT);
      glDrawArrays(GL_QUADS, 0, 4);

      p = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, expected);
      piglit_present_results();
      if (!p) {
         printf("%s: failed when drawing with ", TestName);
         printf("ARB VP and generic vertex array[%u]\n", attrib);
         pass = GL_FALSE;
      }

      glDisableVertexAttribArrayARB(attrib);
      glDisableClientState(GL_COLOR_ARRAY);
      glDeleteProgramsARB(1, &vertProg);
   }

   /*
    * Draw with pos in generic array[6].
    */
   {
      GLuint attrib = 6;
      vertProg = piglit_compile_program(GL_VERTEX_PROGRAM_ARB,
                                        generic6VertProgramText);
      glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vertProg);
      glVertexAttribPointerARB(attrib, 2, GL_FLOAT, GL_FALSE,
                               2 * sizeof(GLfloat), (void *) 0);
      glColorPointer(3, GL_FLOAT, 3 * sizeof(GLfloat),
                     (void *) (8 * sizeof(GLfloat)));
      glEnableVertexAttribArrayARB(attrib);
      glEnableClientState(GL_COLOR_ARRAY);

      glClear(GL_COLOR_BUFFER_BIT);
      glDrawArrays(GL_QUADS, 0, 4);

      p = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, expected);
      piglit_present_results();
      if (!p) {
         printf("%s: failed when drawing with ", TestName);
         printf("ARB VP and generic vertex array[%u]\n", attrib);
         pass = GL_FALSE;
      }

      glDisableVertexAttribArrayARB(attrib);
      glDisableClientState(GL_COLOR_ARRAY);
      glDeleteProgramsARB(1, &vertProg);
   }

   glDisable(GL_FRAGMENT_PROGRAM_ARB);
   glDisable(GL_VERTEX_PROGRAM_ARB);

   glDeleteProgramsARB(1, &fragProg);
   glDeleteBuffersARB(1, &buf);

   return pass;
}


enum piglit_result
piglit_display(void)
{
   GLboolean pass = GL_TRUE;

   pass &= test_fixedfunc_arrays();

   if (piglit_is_extension_supported("GL_ARB_vertex_program") &&
       piglit_is_extension_supported("GL_ARB_fragment_program")) {
      pass &= test_arbvp_arrays();
   }

   if (piglit_is_extension_supported("GL_ARB_shader_objects") &&
       piglit_is_extension_supported("GL_ARB_vertex_shader") &&
       piglit_is_extension_supported("GL_ARB_fragment_shader")) {
      pass &= test_glsl_arrays();
      pass &= test_glsl_no_arrays();
   }

   return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
   piglit_require_extension("GL_ARB_vertex_buffer_object");
}
