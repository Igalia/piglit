/*
 * Copyright (c) 2010 VMware, Inc.
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
 * @file gl30basic.c
 *
 * Test basic GL 3.0 features.
 *
 * Author:
 *    Brian Paul
 */


#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 100;
	config.window_height = 100;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_STENCIL;

PIGLIT_GL_TEST_CONFIG_END

static const char *Prog = "gl30basic";



static enum piglit_result
test_version(void)
{
   const GLubyte *version = glGetString(GL_VERSION);
   GLuint iversion;
   GLint major, minor, k;

   piglit_require_gl_version(30);

   major = version[0] - '0';
   minor = version[2] - '0';

   iversion = major * 10 + minor;

   if (iversion < 30) {
      return PIGLIT_SKIP;
   }

   glGetIntegerv(GL_MAJOR_VERSION, &k);
   if (k != major) {
      printf("%s: major version mismatch (%d vs. %d)\n", Prog, k, major);
      return PIGLIT_FAIL;
   }

   glGetIntegerv(GL_MINOR_VERSION, &k);
   if (k != minor) {
      printf("%s: minor version mismatch (%d vs. %d)\n", Prog, k, minor);
      return PIGLIT_FAIL;
   }

   return PIGLIT_PASS;
}


static enum piglit_result
test_extension_list(void)
{
   GLint k, num_ext;

   glGetIntegerv(GL_NUM_EXTENSIONS, &num_ext);
   if (num_ext < 1 || num_ext > 10000) {
      printf("%s: unreasonable value for GL_NUM_EXTENSIONS: %d\n", Prog, num_ext);
      return PIGLIT_FAIL;
   }

   /* check that extension strings are reasonable */
   for (k = 0; k < num_ext; k++) {
      const GLubyte *ext = glGetStringi(GL_EXTENSIONS, k);
      if (0)
         printf("Ext[%d] = %s\n", k, (char *) ext);
      if (!ext ||
          ext[0] != 'G' ||
          ext[1] != 'L' ||
          ext[2] != '_') {
         printf("%s: bad extension string [%d]: %s\n", Prog, k, ext);
         return PIGLIT_FAIL;
      }
      if (strchr((char *) ext, ' ')) {
         printf("%s: extension string [%d] contains a space: %s\n", Prog, k, ext);
         return PIGLIT_FAIL;
      }
   }

   return PIGLIT_PASS;
}


enum piglit_result
test_clearing(void)
{
#if 0
   static const GLfloat purple[4] = {1.0, 0.0, 1.0, 1.0};
   GLuint buf = GL_FRONT;
   GLfloat color[4], z;
#endif
   GLfloat z;
   GLint stencil;
   GLenum err;

   while (glGetError() != GL_NO_ERROR)
      ;

   /* XXX this fails with NVIDIA's driver.  Is this test correct?? */
#if 0
   /* Color */
   glClearBufferfv(GL_COLOR, buf, purple);
   err = glGetError();
   if (err) {
      printf("%s: glClearBufferfv() generated error 0x%x.\n", Prog, err);
      return PIGLIT_FAIL;
   }

   glReadBuffer(buf);
   glReadPixels(20, 20, 1, 1, GL_RGBA, GL_FLOAT, color);
   if (color[0] != purple[0] ||
       color[1] != purple[1] ||
       color[2] != purple[2] ||
       color[3] != purple[3]) {
      printf("%s: glClearBufferfv() failed.\n", Prog);
      return PIGLIT_FAIL;
   }
#endif

   /* Depth & Stencil */
   glClearBufferfi(GL_DEPTH_STENCIL, 0, 0.5, 3);
   err = glGetError();
   if (err) {
      printf("%s: glClearBufferfi() generated error 0x%x.\n", Prog, err);
      return PIGLIT_FAIL;
   }

   glReadPixels(20, 20, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
   if (fabs(z - 0.5) > 0.001) {
      printf("%s: glClearBufferfi() failed (z was %f, expected 0.5).\n", Prog, z);
      return PIGLIT_FAIL;
   }

   glReadPixels(20, 20, 1, 1, GL_STENCIL_INDEX, GL_INT, &stencil);
   if (stencil != 3) {
      printf("%s: glClearBufferfi() failed (stencil was %d, expected 3).\n",
             Prog, stencil);
      return PIGLIT_FAIL;
   }

   return PIGLIT_PASS;
}


enum piglit_result
piglit_display(void)
{
   enum piglit_result res;
   
   res = test_version();
   if (res != PIGLIT_PASS)
      return res;

   res = test_extension_list();
   if (res != PIGLIT_PASS)
      return res;

   res = test_clearing();
   if (res != PIGLIT_PASS)
      return res;

   return res;
}



void piglit_init(int argc, char**argv)
{
}
