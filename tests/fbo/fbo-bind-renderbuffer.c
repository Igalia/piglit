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

/* The GL_EXT_framebuffer_object spec says:
 *
 *    "<renderbuffer> must be either zero or the name of an existing
 *    renderbuffer object of type <renderbuffertarget>, otherwise
 *    INVALID_OPERATION is generated."
 *
 * This sequence should generate GL_INVALID_OPERATION since the renderbuffer
 * was never bound:
 *
 *   glGenFramebuffers(1, &fb);
 *   glGenRenderbuffers(1, &rb);
 *   glBindFramebuffer(GL_FRAMEBUFFER, fb);
 *   glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
 *                             GL_RENDERBUFFER_EXT, rb);
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

/* Test binding and return error code */
static GLenum
test_binding(GLboolean bindRenderbuffer)
{
   GLuint fb, rb;
   GLenum err;

   glGenFramebuffersEXT(1, &fb);
   glGenRenderbuffersEXT(1, &rb);
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

   err = glGetError();
   if (err != GL_NO_ERROR)
      return PIGLIT_FAIL;

   /* enabling this should prevent the GL error */
   if (bindRenderbuffer)
      glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rb);

   glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                GL_RENDERBUFFER_EXT, rb);

   err = glGetError();

   return err;
}


enum piglit_result
piglit_display(void)
{
   if (test_binding(GL_TRUE) != GL_NO_ERROR) {
      printf("fbo-bind-renderbuffer: generated unexpected error\n");
      return PIGLIT_FAIL;
   }

   if (test_binding(GL_FALSE) != GL_INVALID_OPERATION) {
      printf("fbo-bind-renderbuffer: failed to generate expected error\n");
      return PIGLIT_FAIL;
   }

   return PIGLIT_PASS;

}


void
piglit_init(int argc, char **argv)
{
   piglit_require_extension("GL_EXT_framebuffer_object");
}
