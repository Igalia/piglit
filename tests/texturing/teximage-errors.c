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
 * @file
 * Tests glTexImage functions for invalid values, error reporting.
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.window_width = 100;
	config.window_height = 100;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END



/** Test target params to glTexImage functions */
static GLboolean
test_targets(void)
{
   /* all of these should generate GL_INVALID_ENUM */

   glTexImage1D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 0, GL_RGBA, GL_FLOAT, NULL);
   if (!piglit_check_gl_error(GL_INVALID_ENUM))
      return GL_FALSE;

   glTexImage2D(GL_TEXTURE_3D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_FLOAT, NULL);
   if (!piglit_check_gl_error(GL_INVALID_ENUM))
      return GL_FALSE;

   glTexImage3D(GL_TEXTURE_1D, 0, GL_RGBA, 16, 16, 16, 0, GL_RGBA, GL_FLOAT, NULL);
   if (!piglit_check_gl_error(GL_INVALID_ENUM))
      return GL_FALSE;


   glTexSubImage1D(GL_TEXTURE_2D, 0, 6, 10, GL_RGBA, GL_FLOAT, NULL);
   if (!piglit_check_gl_error(GL_INVALID_ENUM))
      return GL_FALSE;

   glTexSubImage1D(GL_PROXY_TEXTURE_1D, 0, 6, 10, GL_RGBA, GL_FLOAT, NULL);
   if (!piglit_check_gl_error(GL_INVALID_ENUM))
      return GL_FALSE;

   glTexSubImage2D(GL_PROXY_TEXTURE_2D, 0, 6, 6, 10, 10, GL_RGBA, GL_FLOAT, NULL);
   if (!piglit_check_gl_error(GL_INVALID_ENUM))
      return GL_FALSE;

   glTexSubImage3D(GL_PROXY_TEXTURE_2D, 0, 6, 6, 6, 10, 10, 10, GL_RGBA, GL_FLOAT, NULL);
   if (!piglit_check_gl_error(GL_INVALID_ENUM))
      return GL_FALSE;


   glCopyTexImage1D(GL_PROXY_TEXTURE_1D, 0, GL_RGBA, 4, 4, 16, 0);
   if (!piglit_check_gl_error(GL_INVALID_ENUM))
      return GL_FALSE;

   glCopyTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, 4, 4, 16, 16, 0);
   if (!piglit_check_gl_error(GL_INVALID_ENUM))
      return GL_FALSE;

   glCopyTexImage2D(GL_TEXTURE_1D, 0, GL_RGBA, 4, 4, 16, 16, 0);
   if (!piglit_check_gl_error(GL_INVALID_ENUM))
      return GL_FALSE;


   glCopyTexSubImage1D(GL_PROXY_TEXTURE_1D, 0, 4, 4, 6, 10);
   if (!piglit_check_gl_error(GL_INVALID_ENUM))
      return GL_FALSE;

   glCopyTexSubImage2D(GL_PROXY_TEXTURE_2D, 0, 4, 4, 6, 6, 10, 10);
   if (!piglit_check_gl_error(GL_INVALID_ENUM))
      return GL_FALSE;

   glCopyTexSubImage3D(GL_PROXY_TEXTURE_3D, 0, 4, 4, 4, 6, 6, 10, 10);
   if (!piglit_check_gl_error(GL_INVALID_ENUM))
      return GL_FALSE;


   return GL_TRUE;
}


/** Test texture size errors and subtexture position errors */
static GLboolean
test_pos_and_sizes(void)
{
   /* all of these should generate GL_INVALID_VALUE */

   glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, -16, 0, GL_RGBA, GL_FLOAT, NULL);
   if (!piglit_check_gl_error(GL_INVALID_VALUE))
      return GL_FALSE;

   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, -6, -5, 0, GL_RGBA, GL_FLOAT, NULL);
   if (!piglit_check_gl_error(GL_INVALID_VALUE))
      return GL_FALSE;

   glTexImage2D(GL_TEXTURE_2D, -2, GL_RGBA, 16, 16, 0, GL_RGBA, GL_FLOAT, NULL);
   if (!piglit_check_gl_error(GL_INVALID_VALUE))
      return GL_FALSE;

   glTexImage2D(GL_TEXTURE_2D, 2000, GL_RGBA, 16, 16, 0, GL_RGBA, GL_FLOAT, NULL);
   if (!piglit_check_gl_error(GL_INVALID_VALUE))
      return GL_FALSE;


   glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 1<<28, 1<<28, 0);
   if (!piglit_check_gl_error(GL_INVALID_VALUE))
      return GL_FALSE;


   /* setup valid 2D texture for subsequent TexSubImage calls */
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_FLOAT, NULL);


   glTexSubImage2D(GL_TEXTURE_2D, 0, 6, 6, 100, 100, GL_RGBA, GL_FLOAT, NULL);
   if (!piglit_check_gl_error(GL_INVALID_VALUE))
      return GL_FALSE;

   glTexSubImage2D(GL_TEXTURE_2D, 0, -6, -6, 10, 10, GL_RGBA, GL_FLOAT, NULL);
   if (!piglit_check_gl_error(GL_INVALID_VALUE))
      return GL_FALSE;


   glCopyTexSubImage2D(GL_TEXTURE_2D, 0, -6, -6, 2, 2, 10, 10);
   if (!piglit_check_gl_error(GL_INVALID_VALUE))
      return GL_FALSE;

   glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 6, 6, 2, 2, 200, 200);
   if (!piglit_check_gl_error(GL_INVALID_VALUE))
      return GL_FALSE;

   /* mipmap level 1 doesn't exist */
   glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, 8, 8, GL_RGBA, GL_FLOAT, NULL);
   if (!piglit_check_gl_error(GL_INVALID_OPERATION))
      return GL_FALSE;

   /* mipmap level 2 doesn't exist */
   glCopyTexSubImage2D(GL_TEXTURE_2D, 2, 0, 0, 0, 0, 4, 4);
   if (!piglit_check_gl_error(GL_INVALID_OPERATION))
      return GL_FALSE;


   return GL_TRUE;
}


enum piglit_result
piglit_display(void)
{
   if (!test_targets())
      return PIGLIT_FAIL;

   if (!test_pos_and_sizes())
      return PIGLIT_FAIL;

   return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
