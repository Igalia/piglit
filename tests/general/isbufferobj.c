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

/*
 * Author:
 *    Brian Paul
 */

/**
 * @file isbufferobj.c
 *
 * Test glIsBuffer()
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "isbufferobj";


static enum piglit_result
test(void)
{
   GLuint buffers[2];

   if (glIsBuffer(0)) {
      printf("%s: glIsBuffer(0) returned true instead of false.\n", TestName);
      return PIGLIT_FAIL;
   }

   glGenBuffers(2, buffers);
   if (buffers[0] == 0 ||
       buffers[1] == 0 ||
       buffers[0] == buffers[1]) {
      printf("%s: glGenBuffers failed\n", TestName);
      return PIGLIT_FAIL;
   }

   if (piglit_is_extension_supported("GL_EXT_pixel_buffer_object")) {
      glBindBuffer(GL_PIXEL_PACK_BUFFER_EXT, buffers[0]);
      if (glGetError()) {
         printf("%s: glBindBuffer failed\n", TestName);
         return PIGLIT_FAIL;
      }

      if (!glIsBuffer(buffers[0])) {
         printf("%s: glIsBuffer(%u) returned false instead of true.\n",
                TestName, buffers[0]);
         return PIGLIT_FAIL;
      }
   }

   if (glIsBuffer(buffers[1])) {
      printf("%s: glIsBuffer(%u) returned true instead of false.\n",
             TestName, buffers[1]);
      return PIGLIT_FAIL;
   }

   return PIGLIT_PASS;
}


enum piglit_result
piglit_display(void)
{
   return test();
}


void
piglit_init(int argc, char**argv)
{
   piglit_require_gl_version(15);
}
