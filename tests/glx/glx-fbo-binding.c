/*
 * Copyright (c) 2011, VMware, Inc.
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
 * Test framebuffer binding state across glXMakeCurrent calls.
 * Brian Paul
 * June 15, 2011
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_width = 50, piglit_height = 50;
static const char *TestName = "glx-fbo-binding";

static Window Windows[2];
static GLXContext ctx;


enum piglit_result
draw(Display *dpy)
{
   GLuint fbo = 0;
   GLint b = 0;

   /* bind first window, create FBO, bind it */
   glXMakeCurrent(dpy, Windows[0], ctx);
   glGenFramebuffersEXT(1, &fbo);
   if (fbo == 0) {
      printf("%s: glGenFramebuffer() failed (%u)\n",
             TestName, fbo);
      return PIGLIT_FAIL;
   }
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
   glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &b);
   if (fbo != b) {
      printf("%s: glBindFramebuffer() #1 failed (%u vs %d)\n",
             TestName, fbo, b);
      return PIGLIT_FAIL;
   }

   /* bind second window, test FBO binding (should be unchanged) */
   glXMakeCurrent(dpy, Windows[1], ctx);
   glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &b);
   if (fbo != b) {
      printf("%s: glBindFramebuffer() #2 failed (%u vs %d)\n",
             TestName, fbo, b);
      return PIGLIT_FAIL;
   }

   /* bind second window again, test FBO binding */
   glXMakeCurrent(dpy, Windows[1], ctx);
   glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &b);
   if (fbo != b) {
      printf("%s: glBindFramebuffer() #3 failed (%u vs %d)\n",
             TestName, fbo, b);
      return PIGLIT_FAIL;
   }

   return PIGLIT_PASS;
}


int
main(int argc, char **argv)
{
   Display *dpy;
   XVisualInfo *visinfo;
   int i;

   for (i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-auto") == 0) {
         piglit_automatic = 1;
         break;
      }
   }

   dpy = XOpenDisplay(NULL);
   if (!dpy) {
      fprintf(stderr, "Failed to open display\n");
      piglit_report_result(PIGLIT_FAIL);
   }

   visinfo = piglit_get_glx_visual(dpy);
   Windows[0] = piglit_get_glx_window(dpy, visinfo);
   Windows[1] = piglit_get_glx_window(dpy, visinfo);

   XMapWindow(dpy, Windows[0]);
   XMapWindow(dpy, Windows[1]);

   ctx = piglit_get_glx_context(dpy, visinfo);

   glXMakeCurrent(dpy, Windows[0], ctx);
   piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

   piglit_glx_event_loop(dpy, draw);

   return 0;
}
