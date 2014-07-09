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
 * Try copying a large texture image from a small window.  The driver will
 * have to do some clipping to avoid reading out of bounds.
 * XXX we should also do some rendering with the texture and check the results.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static GLboolean
test(void)
{
   const int texWidth = 512, texHeight = 512;
   int i;

   for (i = 0; i < 20; i++) {
      int x = rand() % 300 - 100;
      int y = rand() % 300 - 100;
      int w, h;

      glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y,
                       texWidth, texHeight, 0);

      glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
      glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
      if (w != texWidth || h != texHeight)
         return GL_FALSE;
   }

   return GL_TRUE;
}


enum piglit_result
piglit_display(void)
{
   if (test())
      return PIGLIT_PASS;
   else
      return PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{

}
