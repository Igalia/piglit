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


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

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
   static const GLfloat purple[] = {1.0, 0.0, 1.0};
   static const GLfloat blue[] = {0.0, 0.0, 1.0};
   static const GLfloat green[] = {0.0, 1.0, 0.0};
   GLenum err;
   GLboolean pass = GL_TRUE;

   while (glGetError() != GL_NO_ERROR)
      ;

   /* Front buffer. */
   glDrawBuffer(GL_FRONT);
   glClearBufferfv(GL_COLOR, 0, purple);
   err = glGetError();
   if (err) {
      printf("%s: glClearBufferfv(GL_FRONT) generated error 0x%x.\n", Prog, err);
      return PIGLIT_FAIL;
   }

   glReadBuffer(GL_FRONT);
   if (!piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height, purple)) {
      printf("  from glClearBufferfv(GL_FRONT) failed.\n");
      pass = GL_FALSE;
   }

   /* Back buffer. */
   glDrawBuffer(GL_BACK);
   glClearBufferfv(GL_COLOR, 0, blue);
   err = glGetError();
   if (err) {
      printf("%s: glClearBufferfv(GL_BACK) generated error 0x%x.\n", Prog, err);
      return PIGLIT_FAIL;
   }

   glReadBuffer(GL_BACK);
   if (!piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height, blue)) {
      printf("  from glClearBufferfv(GL_BACK) failed.\n");
      pass = GL_FALSE;
   }

   /* Front and back buffer. */
   glDrawBuffer(GL_FRONT_AND_BACK);
   glClearBufferfv(GL_COLOR, 0, green);
   err = glGetError();
   if (err) {
      printf("%s: glClearBufferfv(GL_FRONT_AND_BACK) generated error 0x%x.\n", Prog, err);
      return PIGLIT_FAIL;
   }

   glReadBuffer(GL_FRONT);
   if (!piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height, green)) {
      printf("  the front buffer from glClearBufferfv(GL_FRONT_AND_BACK) failed.\n");
      pass = GL_FALSE;
   }

   glReadBuffer(GL_BACK);
   if (!piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height, green)) {
      printf("  the back buffer from glClearBufferfv(GL_FRONT_AND_BACK) failed.\n");
      pass = GL_FALSE;
   }

   /* Depth & Stencil */
   glClearBufferfi(GL_DEPTH_STENCIL, 0, 0.5, 3);
   err = glGetError();
   if (err) {
      printf("%s: glClearBufferfi() generated error 0x%x.\n", Prog, err);
      return PIGLIT_FAIL;
   }

   if (!piglit_probe_rect_depth(0, 0, piglit_width, piglit_height, 0.5)) {
      printf("  from glClearBufferfi() failed.\n");
      pass = GL_FALSE;
   }

   if (!piglit_probe_rect_stencil(0, 0, piglit_width, piglit_height, 3)) {
      printf("  from glClearBufferfi() failed.\n");
      pass = GL_FALSE;
   }

   return pass ? PIGLIT_PASS : PIGLIT_FAIL;
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
