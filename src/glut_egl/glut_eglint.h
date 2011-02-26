/*
 * Copyright (C) 2010 LunarG Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#ifndef _GLUT_EGLINT_H_
#define _GLUT_EGLINT_H_

#include "EGL/egl.h"
#include "glut_egl.h"

struct glut_egl_window {
   EGLConfig config;
   EGLContext context;

   /* initialized by native display */
   struct {
      union {
         EGLNativeWindowType window;
         EGLNativePixmapType pixmap;
         EGLSurface surface; /* pbuffer or screen surface */
      } u;
      int width, height;
   } native;

   EGLSurface surface;

   int index;

   GLUT_EGLreshapeCB reshape_cb;
   GLUT_EGLdisplayCB display_cb;
   GLUT_EGLkeyboardCB keyboard_cb;
   GLUT_EGLspecialCB special_cb;
};

struct glut_egl_state {
   int api_mask;
   int display_mode;
   int window_width, window_height;
   const char *display_name;
   int verbose;
   int init_time;

   GLUT_EGLidleCB idle_cb;

   int num_windows;

   /* initialized by native display */
   EGLNativeDisplayType native_dpy;
   EGLint surface_type;

   EGLDisplay dpy;
   EGLint major, minor;

   struct glut_egl_window *current;

   int redisplay;
};

extern struct glut_egl_state *_glut_egl;

void
_glut_eglFatal(char *format, ...);

int
_glut_eglNow(void);

void
_glut_eglNativeInitDisplay(void);

void
_glut_eglNativeFiniDisplay(void);

void
_glut_eglNativeInitWindow(struct glut_egl_window *win, const char *title,
                       int x, int y, int w, int h);

void
_glut_eglNativeFiniWindow(struct glut_egl_window *win);

void
_glut_eglNativeEventLoop(void);

#endif /* _GLUT_EGLINT_H_ */
