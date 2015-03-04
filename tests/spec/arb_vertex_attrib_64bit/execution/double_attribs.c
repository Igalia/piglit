/*
 * Copyright (c) 2010 VMware, Inc.
 * Copyright (c) 2015 Red Hat Inc.
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
 * Test GL_ARB_vertex_attrib_64bit vertex attributes.
 * derived from Brian's gpu_shader4 tests.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "double_attribs";
static const GLuint Index = 3;

static void
gen_double_values(GLdouble values[4], GLuint size)
{
   assert(size >= 1 && size <=4 );

   values[0] = 1.7976931348623157E+308;
   values[1] = 0.0;
   values[2] = -1.3;
   values[3] = 9.88888;
}

static void
gen_float_values(GLfloat values[4], GLuint size)
{
   assert(size >= 1 && size <=4 );

   values[0] = 0.5;
   values[1] = 0.25;
   values[2] = -1.3;
   values[3] = 9.88888;
   if (size < 4)
      values[3] = 1;
   if (size < 3)
      values[2] = 0;
   if (size < 2)
      values[1] = 0;
}

/* doubles don't get default values */
static GLboolean
check_double_attrib(const GLdouble expected[4], GLuint size, const char *func)
{
   GLdouble vals[4];
   int i;
   glGetVertexAttribLdv(Index, GL_CURRENT_VERTEX_ATTRIB_ARB, vals);

   switch (size) {
   case 4:
      if (expected[3] != vals[3])
	 goto fail_print;
   case 3:
      if (expected[2] != vals[2])
	 goto fail_print;
   case 2:
      if (expected[1] != vals[1])
	 goto fail_print;
   case 1:
      if (expected[0] != vals[0])
	 goto fail_print;
   }
   return GL_TRUE;
fail_print:

   fprintf(stderr, "%s: %s failed\n", TestName, func);
   fprintf(stderr, "  Expected: ");
   for (i = 0; i < size; i++)
      fprintf(stderr, " %g", expected[i]);
   fprintf(stderr, "  Found: ");
   for (i = 0; i < size; i++)
      fprintf(stderr, " %g", vals[i]);
   fprintf(stderr, "\n");
   return GL_FALSE;
}

static GLboolean
check_float_attrib(const GLfloat expected[4])
{
   GLfloat vals[4];
   glGetVertexAttribfv(Index, GL_CURRENT_VERTEX_ATTRIB_ARB, vals);
   if (expected[0] != vals[0] ||
       expected[1] != vals[1] ||
       expected[2] != vals[2] ||
       expected[3] != vals[3]) {
      fprintf(stderr, "%s: failed\n", TestName);
      fprintf(stderr, "  Expected: %f, %f, %f, %f\n",
	      expected[0], expected[1], expected[2], expected[3]);
      fprintf(stderr, "  Found:    %f, %f, %f, %f\n",
	      vals[0], vals[1], vals[2], vals[3]);
      return GL_FALSE;
   }
   return GL_TRUE;
}


static GLboolean
test_attrib_funcs(void)
{
   GLdouble vals[4];

   gen_double_values(vals, 1);
   glVertexAttribL1d(Index, vals[0]);
   if (!check_double_attrib(vals, 1, "glVertexAttribL1d"))
      return GL_FALSE;

   gen_double_values(vals, 2);
   glVertexAttribL2d(Index, vals[0], vals[1]);
   if (!check_double_attrib(vals, 2, "glVertexAttribL2d"))
      return GL_FALSE;

   gen_double_values(vals, 3);
   glVertexAttribL3d(Index, vals[0], vals[1], vals[2]);
   if (!check_double_attrib(vals, 3, "glVertexAttribL3d"))
      return GL_FALSE;

   gen_double_values(vals, 4);
   glVertexAttribL4d(Index, vals[0], vals[1], vals[2], vals[3]);
   if (!check_double_attrib(vals, 4, "glVertexAttribL4d"))
      return GL_FALSE;


   gen_double_values(vals, 1);
   glVertexAttribL1dv(Index, vals);
   if (!check_double_attrib(vals, 1, "glVertexAttribL1dv"))
      return GL_FALSE;

   gen_double_values(vals, 2);
   glVertexAttribL2dv(Index, vals);
   if (!check_double_attrib(vals, 2, "glVertexAttribL2dv"))
      return GL_FALSE;

   gen_double_values(vals, 3);
   glVertexAttribL3dv(Index, vals);
   if (!check_double_attrib(vals, 3, "glVertexAttribL3dv"))
      return GL_FALSE;

   gen_double_values(vals, 4);
   glVertexAttribL4dv(Index, vals);
   if (!check_double_attrib(vals, 4, "glVertexAttribL4dv"))
      return GL_FALSE;

   return GL_TRUE;
}

/** Check which datatypes are accepted by glVertexAttribLPointer() */
static GLboolean
test_attrib_array(void)
{
   static const GLenum badTypes[] = {
      GL_BYTE, GL_UNSIGNED_BYTE,
      GL_SHORT, GL_UNSIGNED_SHORT,
      GL_INT, GL_UNSIGNED_INT, GL_FLOAT,
      GL_HALF_FLOAT_ARB, GL_BGRA,
   };
   static const GLenum goodTypes[] = {
      GL_DOUBLE,
   };
   GLint i;
   GLubyte data[100];
   GLuint index = 1;
   GLint size = 4;
   GLsizei stride = 0;
   GLenum err;
   GLuint vao, vbo;

   glGenVertexArrays(1, &vao);
   glBindVertexArray(vao);

   glGenBuffers(1, &vbo);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);

   /* clear any prev errors */
   while (glGetError() != GL_NO_ERROR)
      ;

   /* These should not generate a GL error */
   for (i = 0; i < ARRAY_SIZE(goodTypes); i++) {
      glVertexAttribLPointer(index, size, goodTypes[i], stride, data);
      err = glGetError();
      if (err != GL_NO_ERROR) {
         fprintf(stderr,
                 "%s: glVertexAttribLPointer(type=0x%x) generated error 0x%x\n",
                 TestName, goodTypes[i], err);
         return GL_FALSE;
      }
   }

   for (i = 0; i < ARRAY_SIZE(badTypes); i++) {
      glVertexAttribLPointer(index, size, badTypes[i], stride, data);
      err = glGetError();
      if (err != GL_INVALID_ENUM) {
         fprintf(stderr,
                 "%s: glVertexAttribLPointer(type=0x%x) failed to generate "
                 "GL_INVALID_ENUM\n",
                 TestName, badTypes[i]);
         return GL_FALSE;
      }
   }

   return GL_TRUE;
}


/* these tests try and exercise the mesa vbo code */
/* write a double to an attribute slot,
 * then write some floats, then rewrite the double,
 * and read it back */
static GLboolean
test_attrib_mixed_1(void)
{
   GLdouble vals[4];
   GLfloat fvals[4];

   gen_double_values(vals, 4);
   glVertexAttribL4dv(Index, vals);

   if (!check_double_attrib(vals, 1, "glVertexAttribL1d"))
      return GL_FALSE;

   gen_float_values(fvals, 4);
   glVertexAttrib4fv(Index, fvals);

   gen_double_values(vals, 4);
   glVertexAttribL4dv(Index, vals);

   if (!check_double_attrib(vals, 1, "glVertexAttribL1d"))
      return GL_FALSE;
   return GL_TRUE;
}

/* write a double to an attribute slot,
 * then write some floats, read them back,
 * then rewrite the double, and read it back */
static GLboolean
test_attrib_mixed_2(void)
{
   GLdouble vals[4];
   GLfloat fvals[4];

   gen_double_values(vals, 4);
   glVertexAttribL4dv(Index, vals);

   if (!check_double_attrib(vals, 1, "glVertexAttribL1d"))
      return GL_FALSE;

   gen_float_values(fvals, 4);
   glVertexAttrib4fv(Index, fvals);

   if (!check_float_attrib(fvals))
      return GL_FALSE;

   gen_double_values(vals, 4);
   glVertexAttribL4dv(Index, vals);

   if (!check_double_attrib(vals, 1, "glVertexAttribL1d"))
      return GL_FALSE;
   return GL_TRUE;
}

/* write a float to an attribute slot,
 * then write a double. and read it back */
static GLboolean
test_attrib_mixed_3(void)
{
   GLdouble vals[4];
   GLfloat fvals[4];

   gen_float_values(fvals, 4);
   glVertexAttrib4fv(Index, fvals);

   gen_double_values(vals, 4);
   glVertexAttribL4dv(Index, vals);

   if (!check_double_attrib(vals, 1, "glVertexAttribL1d"))
      return GL_FALSE;
   return GL_TRUE;
}

enum piglit_result
piglit_display(void)
{
   return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
   int ret = PIGLIT_PASS;
   piglit_require_extension("GL_ARB_vertex_attrib_64bit");

   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

   if (!test_attrib_funcs())
      ret = PIGLIT_FAIL;

   if (!test_attrib_array())
      ret = PIGLIT_FAIL;

   if (!test_attrib_mixed_1())
      ret = PIGLIT_FAIL;

   if (!test_attrib_mixed_2())
      ret = PIGLIT_FAIL;

   if (!test_attrib_mixed_3())
      ret = PIGLIT_FAIL;

   piglit_report_result(ret);
}
