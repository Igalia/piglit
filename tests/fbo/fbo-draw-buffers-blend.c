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
 * @file fbo-draw-buffers-blend.c
 *
 * Test GL_ARB_draw_buffers_blend extension (per-buffer blend state)
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "fbo-draw-buffers-blend";

static GLint maxBuffers;
static GLuint FBO;


#define MY_ASSERT(x) my_assert(x, #x)

static void
my_assert(int test, const char *text)
{
   if (!test) {
      printf("%s: assertion %s failed\n", TestName, text);
      piglit_report_result(PIGLIT_FAIL);
   }
}


static void
check_error(int line)
{
   GLenum err = glGetError();
   if (err) {
      printf("%s: Unexpected error 0x%x at line %d\n",
              TestName, err, line);
      piglit_report_result(PIGLIT_FAIL);
   }
}


static void
create_fbo(void)
{
   GLuint rb[32];
   int i;

   glGenFramebuffersEXT(1, &FBO);
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);

   glGenRenderbuffersEXT(maxBuffers, rb);
   check_error(__LINE__);

   for (i = 0; i < maxBuffers; i++) {
      glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rb[i]);
      check_error(__LINE__);

      glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
                                   GL_COLOR_ATTACHMENT0 + i,
                                   GL_RENDERBUFFER_EXT,
                                   rb[i]);
      check_error(__LINE__);

      glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA,
                               piglit_width, piglit_height);
      check_error(__LINE__);
   }
}


static enum piglit_result
test(void)
{
   GLenum buffers[32];
   static const GLfloat dest_color[4] = { 0.75, 0.25, 0.25, 0.5 };
   static const GLfloat test_color[4] = { 1.0, 0.25, 0.75, 0.25 };
   GLfloat expected[32][4];
   int i;

   create_fbo();

   for (i = 0; i < maxBuffers; i++) {
      buffers[i] = GL_COLOR_ATTACHMENT0_EXT + i;
   }

   glDrawBuffersARB(maxBuffers, buffers);

   /* Setup blend modes and compute expected result color.
    * We only test two simple blending modes.  A more elaborate
    * test would exercise a much wider variety of modes.
    */
   for (i = 0; i < maxBuffers; i++) {
      if (i % 2 == 0) {
         float a;

         glBlendFunciARB(i, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

         a = test_color[3];
         expected[i][0] = test_color[0] * a + dest_color[0] * (1.0 - a);
         expected[i][1] = test_color[1] * a + dest_color[1] * (1.0 - a);
         expected[i][2] = test_color[2] * a + dest_color[2] * (1.0 - a);
         expected[i][3] = test_color[3] * a + dest_color[3] * (1.0 - a);
      }
      else {
         glBlendFunciARB(i, GL_ONE, GL_ONE);
         glBlendEquationiARB(i, GL_FUNC_SUBTRACT);

         expected[i][0] = test_color[0] - dest_color[0];
         expected[i][1] = test_color[1] - dest_color[1];
         expected[i][2] = test_color[2] - dest_color[2];
         expected[i][3] = test_color[3] - dest_color[3];
      }

      expected[i][0] = CLAMP(expected[i][0], 0.0, 1.0);
      expected[i][1] = CLAMP(expected[i][1], 0.0, 1.0);
      expected[i][2] = CLAMP(expected[i][2], 0.0, 1.0);
      expected[i][3] = CLAMP(expected[i][3], 0.0, 1.0);

      glEnableIndexedEXT(GL_BLEND, i);
   }

   /* query blend modes */
   for (i = 0; i < maxBuffers; i++) {
      GLint p0, p1, p2, p3;
      glGetIntegerIndexedvEXT(GL_BLEND_SRC, i, &p0);
      glGetIntegerIndexedvEXT(GL_BLEND_DST, i, &p1);
      glGetIntegerIndexedvEXT(GL_BLEND_EQUATION, i, &p2);
      glGetIntegerIndexedvEXT(GL_BLEND, i, &p3);
      if (i % 2 == 0) {
         MY_ASSERT(p0 == GL_SRC_ALPHA);
         MY_ASSERT(p1 == GL_ONE_MINUS_SRC_ALPHA);
         MY_ASSERT(p2 == GL_FUNC_ADD);
      }
      else {
         MY_ASSERT(p0 == GL_ONE);
         MY_ASSERT(p1 == GL_ONE);
         MY_ASSERT(p2 == GL_FUNC_SUBTRACT);
      }
      MY_ASSERT(p3 == GL_TRUE);
   }

   /* test drawing */
   glClearColor(dest_color[0], dest_color[1], dest_color[2], dest_color[3]);
   glClear(GL_COLOR_BUFFER_BIT);

   glColor4fv(test_color);
   piglit_draw_rect(0, 0, piglit_width, piglit_height);

   for (i = 0; i < maxBuffers; i++) {
      glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + i);
      check_error(__LINE__);

      if (!piglit_probe_pixel_rgba(5, 5, expected[i])) {
         printf("For color buffer %d\n", i);
         return PIGLIT_FAIL;
      }
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
   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

   piglit_require_extension("GL_ARB_draw_buffers_blend");

   glGetIntegerv(GL_MAX_DRAW_BUFFERS_ARB, &maxBuffers);
}
