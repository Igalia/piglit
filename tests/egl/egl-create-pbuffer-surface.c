/*
 * Copyright © 2014 Intel Corporation
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
 *
 * Author: Juha-Pekka Heikkilä <juhapekka.heikkila@gmail.com>
 */

/** @file egl-create-pbuffer-surface.c
 *
 * Test EGLCreatePBufferSurface behaviour.
 */

#include "piglit-util-gl.h"
#include "egl-util.h"

static enum piglit_result
draw(struct egl_state *state)
{
   EGLSurface surf;
   const float purple[] = {1.0, 0.0, 1.0, 1.0};

   const EGLint srfPbufferAttr[] =
   {
      EGL_WIDTH, 256,
      EGL_HEIGHT, 256,
      EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
      EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
      EGL_LARGEST_PBUFFER, EGL_TRUE,
      EGL_NONE
   };

   surf = eglCreatePbufferSurface(state->egl_dpy, state->cfg,
                                   srfPbufferAttr);

   if (eglGetError() != EGL_SUCCESS || surf == EGL_NO_SURFACE) {
      fprintf(stderr, "eglCreatePbufferSurface failed\n");
      piglit_report_result(PIGLIT_FAIL);
   }

   glEnable(GL_TEXTURE_2D);

   eglMakeCurrent(state->egl_dpy, state->surf, state->surf, state->ctx);
   glClearColor(1.0, 1.0, 1.0, 0.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   eglBindTexImage(state->egl_dpy, surf, EGL_BACK_BUFFER);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

   glViewport(0, 0, state->width, state->height);
   piglit_ortho_projection(state->width, state->height, GL_FALSE);

   eglMakeCurrent(state->egl_dpy, surf, surf, state->ctx);
   glClearColor(purple[0], purple[1], purple[2], purple[3]);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   eglMakeCurrent(state->egl_dpy, state->surf, state->surf, state->ctx);
   piglit_draw_rect_tex(0, 0, 256, 256, 0, 0, 1, 1);
   eglSwapBuffers(state->egl_dpy, state->surf);

   if (!piglit_probe_rect_rgba(0, 0, 256, 256, purple))
      piglit_report_result(PIGLIT_FAIL);

   piglit_report_result(PIGLIT_PASS);
}

int
main(int argc, char *argv[])
{
   struct egl_test test;
   const EGLint test_attribs[] =
   {
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
      EGL_NONE
   };

   egl_init_test(&test);
   test.draw = draw;
   test.stop_on_failure = true;
   test.config_attribs = test_attribs;

   if (egl_util_run(&test, argc, argv) != PIGLIT_PASS)
      return EXIT_FAILURE;
   return EXIT_SUCCESS;
}
