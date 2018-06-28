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

	config.supports_gl_compat_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "double_attribs";
static const GLuint Index = 3;
static const GLdouble Zero = 0.0;
static const GLdouble Zero_vals[4] = { 0.0, 0.0, 0.0, 0.0 };

static GLuint list;

static void
gen_double_values(GLdouble values[4], GLuint size)
{
   assert(size >= 1 && size <=4 );

   values[0] = 1.7976931348623157E+308;
   values[1] = 0.0;
   values[2] = -1.3;
   values[3] = 9.88888;
}

/* doubles don't get default values */
static GLboolean
check_double_attrib(const GLuint idx, const GLdouble expected[4],
                    GLuint size, const char *func)
{
   GLdouble vals[4];
   glGetVertexAttribLdv(idx, GL_CURRENT_VERTEX_ATTRIB_ARB, vals);

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
   for (unsigned i = 0; i < size; i++)
      fprintf(stderr, " %g", expected[i]);
   fprintf(stderr, "  Found: ");
   for (unsigned i = 0; i < size; i++)
      fprintf(stderr, " %g", vals[i]);
   fprintf(stderr, "\n");
   return GL_FALSE;
}

static void
reset_attribs_to_zero()
{
   glVertexAttribL1d(Index, Zero);
   glVertexAttribL2d(Index+1, Zero, Zero);
   glVertexAttribL3d(Index+2, Zero, Zero, Zero);
   glVertexAttribL4d(Index+3, Zero, Zero, Zero, Zero);


}

static void
compile_display_list(GLenum mode, bool ptr_funcs)
{
   GLdouble vals[4];

   glNewList(list, mode);

   if (ptr_funcs) {
      gen_double_values(vals, 1);
      glVertexAttribL1dv(Index, vals);

      gen_double_values(vals, 2);
      glVertexAttribL2dv(Index+1, vals);

      gen_double_values(vals, 3);
      glVertexAttribL3dv(Index+2, vals);

      gen_double_values(vals, 4);
      glVertexAttribL4dv(Index+3, vals);
   } else {
      gen_double_values(vals, 1);
      glVertexAttribL1d(Index, vals[0]);

      gen_double_values(vals, 2);
      glVertexAttribL2d(Index+1, vals[0], vals[1]);

      gen_double_values(vals, 3);
      glVertexAttribL3d(Index+2, vals[0], vals[1], vals[2]);

      gen_double_values(vals, 4);
      glVertexAttribL4d(Index+3, vals[0], vals[1], vals[2], vals[3]);
   }

   glEndList();
}

static bool
check_dlist_results(bool ptr_funcs)
{
   GLdouble vals[4];

   if (ptr_funcs) {
      gen_double_values(vals, 1);
      if (!check_double_attrib(Index, vals, 1, "glVertexAttribL1dv"))
         return GL_FALSE;

      gen_double_values(vals, 2);
      if (!check_double_attrib(Index+1, vals, 2, "glVertexAttribL2dv"))
         return GL_FALSE;

      gen_double_values(vals, 3);
      if (!check_double_attrib(Index+2, vals, 3, "glVertexAttribL3dv"))
         return GL_FALSE;

      gen_double_values(vals, 4);
      if (!check_double_attrib(Index+3, vals, 4, "glVertexAttribL4dv"))
         return GL_FALSE;
   } else {
      gen_double_values(vals, 1);
      if (!check_double_attrib(Index, vals, 1, "glVertexAttribL1d"))
         return GL_FALSE;

      gen_double_values(vals, 2);
      if (!check_double_attrib(Index+1, vals, 2, "glVertexAttribL2d"))
         return GL_FALSE;

      gen_double_values(vals, 3);
      if (!check_double_attrib(Index+2, vals, 3, "glVertexAttribL3d"))
         return GL_FALSE;

      gen_double_values(vals, 4);
      if (!check_double_attrib(Index+3, vals, 4, "glVertexAttribL4d"))
         return GL_FALSE;
   }

   return GL_TRUE;
}

static GLboolean
test_attrib_funcs(void)
{
   list = glGenLists(1);

   reset_attribs_to_zero();

   /* Compile display list */
   compile_display_list(GL_COMPILE, false);

   /* Make sure the attribute were not updated during display list
    * compilation.
    */
   if (!check_double_attrib(Index, Zero_vals, 1, "glVertexAttribL1d"))
      return GL_FALSE;

   if (!check_double_attrib(Index+1, Zero_vals, 2, "glVertexAttribL2d"))
      return GL_FALSE;

   if (!check_double_attrib(Index+2, Zero_vals, 3, "glVertexAttribL3d"))
      return GL_FALSE;

   if (!check_double_attrib(Index+3, Zero_vals, 4, "glVertexAttribL4d"))
      return GL_FALSE;

   /* Call display list and check values were set correctly */
   glCallList(list);
   if (!check_dlist_results(false))
      return GL_FALSE;

   /* Reset attribute and call compile and execute display list */
   reset_attribs_to_zero();
   compile_display_list(GL_COMPILE_AND_EXECUTE, false);

   /* Check values were set correctly */
   if (!check_dlist_results(false))
      return GL_FALSE;

   /* Rest attribute then call display list and check values were set
    * correctly.
    */
   reset_attribs_to_zero();
   glCallList(list);
   if (!check_dlist_results(false))
      return GL_FALSE;

   /* --------------------------------------------------------------
    * Now do the same test as above but with the glVertexAttribL*v()
    * functions.
    *---------------------------------------------------------------
    */

   reset_attribs_to_zero();

   /* Compile display list */
   compile_display_list(GL_COMPILE, true);

   /* Make sure the attribute were not updated during display list
    * compilation.
    */
   if (!check_double_attrib(Index, Zero_vals, 1, "glVertexAttribL1d"))
      return GL_FALSE;

   if (!check_double_attrib(Index+1, Zero_vals, 2, "glVertexAttribL2d"))
      return GL_FALSE;

   if (!check_double_attrib(Index+2, Zero_vals, 3, "glVertexAttribL3d"))
      return GL_FALSE;

   if (!check_double_attrib(Index+3, Zero_vals, 4, "glVertexAttribL4d"))
      return GL_FALSE;

   /* Call display list and check values were set correctly */
   glCallList(list);
   if (!check_dlist_results(true))
      return GL_FALSE;

   /* Reset attribute and call compile and execute display list */
   reset_attribs_to_zero();
   compile_display_list(GL_COMPILE_AND_EXECUTE, true);

   /* Check values were set correctly */
   if (!check_dlist_results(true))
      return GL_FALSE;

   /* Rest attribute then call display list and check values were set
    * correctly.
    */
   reset_attribs_to_zero();
   glCallList(list);
   if (!check_dlist_results(true))
      return GL_FALSE;


   return GL_TRUE;
}

void
piglit_init(int argc, char **argv)
{
   int ret = PIGLIT_PASS;
   piglit_require_extension("GL_ARB_vertex_attrib_64bit");

   if (!test_attrib_funcs())
      ret = PIGLIT_FAIL;

   piglit_report_result(ret);
}

enum piglit_result
piglit_display(void)
{
   return PIGLIT_FAIL;
}
