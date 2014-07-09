/*
 * Copyright (c) 2010 VMware, Inc.
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
 * Test GL_EXT_gpu_shader4's vertex attribute and uniform functions.
 * Brian Paul
 * 27 Oct 2010
 */



#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "texture-integer";
static const GLuint Index = 3;
static GLuint Program;


#define UNSIGNED 0
#define SIGNED 1


static GLboolean
check_error(const char *file, int line)
{
   GLenum err = glGetError();
   if (err) {
      fprintf(stderr, "%s: error 0x%x at %s:%d\n", TestName, err, file, line);
      return GL_TRUE;
   }
   return GL_FALSE;
}



static void
gen_values(GLint values[4], GLuint size, int sign)
{
   assert(size >= 1 && size <=4 );

   if (sign == SIGNED) {
      /* [-100, 100] */
      values[0] = rand() % 201 - 100;
      values[1] = rand() % 201 - 100;
      values[2] = rand() % 201 - 100;
      values[3] = rand() % 201 - 100;
   }
   else {
      /* [0, 255] */
      values[0] = rand() % 255;
      values[1] = rand() % 255;
      values[2] = rand() % 255;
      values[3] = rand() % 255;
   }
   if (size < 4)
      values[3] = 1;
   if (size < 3)
      values[2] = 0;
   if (size < 2)
      values[1] = 0;
}


static GLboolean
check_attrib(const GLint expected[4], int sign, const char *func)
{
   if (sign == SIGNED) {
      GLint vals[4];
      glGetVertexAttribIivEXT(Index, GL_CURRENT_VERTEX_ATTRIB_ARB, vals);
      if (expected[0] != vals[0] ||
          expected[1] != vals[1] ||
          expected[2] != vals[2] ||
          expected[3] != vals[3]) {
         fprintf(stderr, "%s: %s failed\n", TestName, func);
         fprintf(stderr, "  Expected: %d, %d, %d, %d\n",
                 expected[0], expected[1], expected[2], expected[3]);
         fprintf(stderr, "  Found:    %d, %d, %d, %d\n",
                 vals[0], vals[1], vals[2], vals[3]);
         return GL_FALSE;
      }
   }
   else {
      GLuint vals[4];
      glGetVertexAttribIuivEXT(Index, GL_CURRENT_VERTEX_ATTRIB_ARB, vals);
      if (expected[0] != vals[0] ||
          expected[1] != vals[1] ||
          expected[2] != vals[2] ||
          expected[3] != vals[3]) {
         fprintf(stderr, "%s: %s failed\n", TestName, func);
         fprintf(stderr, "  Expected: %d, %d, %d, %d\n",
                 expected[0], expected[1], expected[2], expected[3]);
         fprintf(stderr, "  Found:    %u, %u, %u, %u\n",
                 vals[0], vals[1], vals[2], vals[3]);
         return GL_FALSE;
      }
   }
   return GL_TRUE;
}



static GLboolean
test_attrib_funcs(void)
{
   GLint vals[4];

   gen_values(vals, 1, SIGNED);
   glVertexAttribI1iEXT(Index, vals[0]);
   if (!check_attrib(vals, SIGNED, "glVertexAttribI1iEXT"))
      return GL_FALSE;

   gen_values(vals, 2, SIGNED);
   glVertexAttribI2iEXT(Index, vals[0], vals[1]);
   if (!check_attrib(vals, SIGNED, "glVertexAttribI2iEXT"))
      return GL_FALSE;

   gen_values(vals, 3, SIGNED);
   glVertexAttribI3iEXT(Index, vals[0], vals[1], vals[2]);
   if (!check_attrib(vals, SIGNED, "glVertexAttribI3iEXT"))
      return GL_FALSE;

   gen_values(vals, 4, SIGNED);
   glVertexAttribI4iEXT(Index, vals[0], vals[1], vals[2], vals[3]);
   if (!check_attrib(vals, SIGNED, "glVertexAttribI4iEXT"))
      return GL_FALSE;


   gen_values(vals, 1, SIGNED);
   glVertexAttribI1ivEXT(Index, vals);
   if (!check_attrib(vals, SIGNED, "glVertexAttribI1ivEXT"))
      return GL_FALSE;

   gen_values(vals, 2, SIGNED);
   glVertexAttribI2ivEXT(Index, vals);
   if (!check_attrib(vals, SIGNED, "glVertexAttribI2ivEXT"))
      return GL_FALSE;

   gen_values(vals, 3, SIGNED);
   glVertexAttribI3ivEXT(Index, vals);
   if (!check_attrib(vals, SIGNED, "glVertexAttribI3ivEXT"))
      return GL_FALSE;

   gen_values(vals, 4, SIGNED);
   glVertexAttribI4ivEXT(Index, vals);
   if (!check_attrib(vals, SIGNED, "glVertexAttribI4ivEXT"))
      return GL_FALSE;


   gen_values(vals, 1, UNSIGNED);
   glVertexAttribI1uiEXT(Index, vals[0]);
   if (!check_attrib(vals, UNSIGNED, "glVertexAttribI1uiEXT"))
      return GL_FALSE;

   gen_values(vals, 2, UNSIGNED);
   glVertexAttribI2uiEXT(Index, vals[0], vals[1]);
   if (!check_attrib(vals, UNSIGNED, "glVertexAttribI2uiEXT"))
      return GL_FALSE;

   gen_values(vals, 3, UNSIGNED);
   glVertexAttribI3uiEXT(Index, vals[0], vals[1], vals[2]);
   if (!check_attrib(vals, UNSIGNED, "glVertexAttribI3uiEXT"))
      return GL_FALSE;

   gen_values(vals, 4, UNSIGNED);
   glVertexAttribI4uiEXT(Index, vals[0], vals[1], vals[2], vals[3]);
   if (!check_attrib(vals, UNSIGNED, "glVertexAttribI4uiEXT"))
      return GL_FALSE;


   gen_values(vals, 1, UNSIGNED);
   glVertexAttribI1uivEXT(Index, (GLuint *) vals);
   if (!check_attrib(vals, UNSIGNED, "glVertexAttribI1uivEXT"))
      return GL_FALSE;

   gen_values(vals, 2, UNSIGNED);
   glVertexAttribI2uivEXT(Index, (GLuint *) vals);
   if (!check_attrib(vals, UNSIGNED, "glVertexAttribI2uivEXT"))
      return GL_FALSE;

   gen_values(vals, 3, UNSIGNED);
   glVertexAttribI3uivEXT(Index, (GLuint *) vals);
   if (!check_attrib(vals, UNSIGNED, "glVertexAttribI3uivEXT"))
      return GL_FALSE;

   gen_values(vals, 4, UNSIGNED);
   glVertexAttribI4uivEXT(Index, (GLuint *) vals);
   if (!check_attrib(vals, UNSIGNED, "glVertexAttribI4uivEXT"))
      return GL_FALSE;

   {
      GLbyte bvals[4];
      gen_values(vals, 4, SIGNED);
      bvals[0] = vals[0];
      bvals[1] = vals[1];
      bvals[2] = vals[2];
      bvals[3] = vals[3];
      glVertexAttribI4bvEXT(Index, bvals);
      if (!check_attrib(vals, SIGNED, "glVertexAttribI4bvEXT"))
         return GL_FALSE;
   }

   {
      GLshort svals[4];
      gen_values(vals, 4, SIGNED);
      svals[0] = vals[0];
      svals[1] = vals[1];
      svals[2] = vals[2];
      svals[3] = vals[3];
      glVertexAttribI4svEXT(Index, svals);
      if (!check_attrib(vals, SIGNED, "glVertexAttribI4svEXT"))
         return GL_FALSE;
   }

   {
      GLubyte bvals[4];
      gen_values(vals, 4, UNSIGNED);
      bvals[0] = vals[0];
      bvals[1] = vals[1];
      bvals[2] = vals[2];
      bvals[3] = vals[3];
      glVertexAttribI4ubvEXT(Index, bvals);
      if (!check_attrib(vals, UNSIGNED, "glVertexAttribI4ubvEXT"))
         return GL_FALSE;
   }

   {
      GLushort svals[4];
      gen_values(vals, 4, UNSIGNED);
      svals[0] = vals[0];
      svals[1] = vals[1];
      svals[2] = vals[2];
      svals[3] = vals[3];
      glVertexAttribI4usvEXT(Index, svals);
      if (!check_attrib(vals, UNSIGNED, "glVertexAttribI4usvEXT"))
         return GL_FALSE;
   }

   return GL_TRUE;
}



static GLboolean
check_uniform(const GLint expected[4], GLuint size, int sign,
              GLint loc, const char *func)
{
   if (sign == SIGNED) {
      GLint vals[4];
      glGetUniformivARB(Program, loc, vals);
      if (size < 4)
         vals[3] = 1;
      if (size < 3)
         vals[2] = 0;
      if (size < 2)
         vals[1] = 0;
      if (expected[0] != vals[0] ||
          expected[1] != vals[1] ||
          expected[2] != vals[2] ||
          expected[3] != vals[3]) {
         fprintf(stderr, "%s: %s failed\n", TestName, func);
         fprintf(stderr, "  Expected: %d, %d, %d, %d\n",
                 expected[0], expected[1], expected[2], expected[3]);
         fprintf(stderr, "  Found:    %d, %d, %d, %d\n",
                 vals[0], vals[1], vals[2], vals[3]);
         return GL_FALSE;
      }
   }
   else {
      GLuint vals[4];
      glGetUniformuivEXT(Program, loc, vals);
      if (size < 4)
         vals[3] = 1;
      if (size < 3)
         vals[2] = 0;
      if (size < 2)
         vals[1] = 0;
      if (expected[0] != vals[0] ||
          expected[1] != vals[1] ||
          expected[2] != vals[2] ||
          expected[3] != vals[3]) {
         fprintf(stderr, "%s: %s failed\n", TestName, func);
         fprintf(stderr, "  Expected: %d, %d, %d, %d\n",
                 expected[0], expected[1], expected[2], expected[3]);
         fprintf(stderr, "  Found:    %u, %u, %u, %u\n",
                 vals[0], vals[1], vals[2], vals[3]);
         return GL_FALSE;
      }
   }
   return GL_TRUE;
}


static GLboolean
test_uniform_funcs(void)
{
   static const char *signedFragText =
      "uniform int value1; \n"
      "uniform ivec2 value2; \n"
      "uniform ivec3 value3; \n"
      "uniform ivec4 value4; \n"
      "void main() \n"
      "{ \n"
      "   vec4 t = vec4(value4); \n"
      "   t += vec4(value3, 0.0); \n"
      "   t += vec4(value2, 0.0, 0.0); \n"
      "   t += vec4(value1, 0.0, 0.0, 0.0); \n"
      " gl_FragColor = 0.01 * t; \n"
      "} \n";

   static const char *unsignedFragText =
      "#extension GL_EXT_gpu_shader4: enable \n"
      "uniform unsigned int value1; \n"
      "uniform uvec2 value2; \n"
      "uniform uvec3 value3; \n"
      "uniform uvec4 value4; \n"
      "void main() \n"
      "{ \n"
      "   vec4 t = vec4(value4); \n"
      "   t += vec4(value3, 0.0); \n"
      "   t += vec4(value2, 0.0, 0.0); \n"
      "   t += vec4(value1, 0.0, 0.0, 0.0); \n"
      " gl_FragColor = 0.01 * t; \n"
      "} \n";


   GLint vals[4], loc1, loc2, loc3, loc4;
   GLuint shader;

   /*
    * Signed integer tests.
    */
   shader = piglit_compile_shader_text(GL_FRAGMENT_SHADER, signedFragText);
   assert(shader);

   Program = piglit_link_simple_program(0, shader);
   assert(Program);

   glUseProgram(Program);
   check_error(__FILE__, __LINE__);

   loc1 = glGetUniformLocation(Program, "value1");
   assert(loc1 >= 0);

   loc2 = glGetUniformLocation(Program, "value2");
   assert(loc2 >= 0);

   loc3 = glGetUniformLocation(Program, "value3");
   assert(loc3 >= 0);

   loc4 = glGetUniformLocation(Program, "value4");
   assert(loc4 >= 0);

   check_error(__FILE__, __LINE__);

   gen_values(vals, 1, SIGNED);
   glUniform1iARB(loc1, vals[0]);
   if (!check_uniform(vals, 1, SIGNED, loc1, "glUniform1iARB"))
      return GL_FALSE;

   gen_values(vals, 2, SIGNED);
   glUniform2iARB(loc2, vals[0], vals[1]);
   if (!check_uniform(vals, 2, SIGNED, loc2, "glUniform2iARB"))
      return GL_FALSE;

   gen_values(vals, 3, SIGNED);
   glUniform3iARB(loc3, vals[0], vals[1], vals[2]);
   if (!check_uniform(vals, 3, SIGNED, loc3, "glUniform3iARB"))
      return GL_FALSE;

   gen_values(vals, 4, SIGNED);
   glUniform4iARB(loc4, vals[0], vals[1], vals[2], vals[3]);
   if (!check_uniform(vals, 4, SIGNED, loc4, "glUniform4iARB"))
      return GL_FALSE;


   /*
    * Unsigned integer tests.
    */
   shader = piglit_compile_shader_text(GL_FRAGMENT_SHADER, unsignedFragText);
   assert(shader);

   Program = piglit_link_simple_program(0, shader);
   assert(Program);

   glUseProgram(Program);
   check_error(__FILE__, __LINE__);

   loc1 = glGetUniformLocation(Program, "value1");
   assert(loc1 >= 0);

   loc2 = glGetUniformLocation(Program, "value2");
   assert(loc2 >= 0);

   loc3 = glGetUniformLocation(Program, "value3");
   assert(loc3 >= 0);

   loc4 = glGetUniformLocation(Program, "value4");
   assert(loc4 >= 0);

   check_error(__FILE__, __LINE__);

   gen_values(vals, 1, UNSIGNED);
   glUniform1uiEXT(loc1, vals[0]);
   if (!check_uniform(vals, 1, UNSIGNED, loc1, "glUniform1uiEXT"))
      return GL_FALSE;

   gen_values(vals, 2, UNSIGNED);
   glUniform2uiEXT(loc2, vals[0], vals[1]);
   if (!check_uniform(vals, 2, UNSIGNED, loc2, "glUniform2uiEXT"))
      return GL_FALSE;

   gen_values(vals, 3, UNSIGNED);
   glUniform3uiEXT(loc3, vals[0], vals[1], vals[2]);
   if (!check_uniform(vals, 3, UNSIGNED, loc3, "glUniform3uiEXT"))
      return GL_FALSE;

   gen_values(vals, 4, UNSIGNED);
   glUniform4uiEXT(loc4, vals[0], vals[1], vals[2], vals[3]);
   if (!check_uniform(vals, 4, UNSIGNED, loc4, "glUniform4uiEXT"))
      return GL_FALSE;

   return GL_TRUE;
}


/** Check which datatypes are accepted by glVertexAttribIPointer() */
static GLboolean
test_attrib_array(void)
{
   static const GLenum goodTypes[] = {
      GL_BYTE, GL_UNSIGNED_BYTE,
      GL_SHORT, GL_UNSIGNED_SHORT,
      GL_INT, GL_UNSIGNED_INT
   };
   static const GLenum badTypes[] = {
      GL_FLOAT, GL_DOUBLE, GL_HALF_FLOAT_ARB, GL_BGRA
   };
   GLint i;
   GLubyte data[100];
   GLuint index = 1;
   GLint size = 4;
   GLsizei stride = 0;
   GLenum err;

   /* clear any prev errors */
   while (glGetError() != GL_NO_ERROR)
      ;

   /* These should not generate a GL error */
   for (i = 0; i < ARRAY_SIZE(goodTypes); i++) {
      glVertexAttribIPointerEXT(index, size, goodTypes[i], stride, data);
      err = glGetError();
      if (err != GL_NO_ERROR) {
         fprintf(stderr,
                 "%s: glVertexAttribIPointer(type=0x%x) generated error 0x%x\n",
                 TestName, goodTypes[i], err);
         return GL_FALSE;
      }
   }

   for (i = 0; i < ARRAY_SIZE(badTypes); i++) {
      glVertexAttribIPointerEXT(index, size, badTypes[i], stride, data);
      err = glGetError();
      if (err != GL_INVALID_ENUM) {
         fprintf(stderr,
                 "%s: glVertexAttribIPointer(type=0x%x) failed to generate "
                 "GL_INVALID_ENUM\n",
                 TestName, badTypes[i]);
         return GL_FALSE;
      }
   }

   return GL_TRUE;
}


enum piglit_result
piglit_display(void)
{
   if (!test_attrib_funcs())
      return PIGLIT_FAIL;

   if (!test_uniform_funcs())
      return PIGLIT_FAIL;

   if (!test_attrib_array())
      return PIGLIT_FAIL;

   return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
   piglit_require_extension("GL_EXT_gpu_shader4");

   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
