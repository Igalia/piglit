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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <glut_egl/glut_eglint.h>

void
_glutNativeInitDisplay(void)
{
   _glut->native_dpy = XOpenDisplay(_glut->display_name);
   if (!_glut->native_dpy)
      _glutFatal("failed to initialize native display");

   _glut->surface_type = EGL_WINDOW_BIT;
}

void
_glutNativeFiniDisplay(void)
{
   XCloseDisplay(_glut->native_dpy);
}

void
_glutNativeInitWindow(struct glut_window *win, const char *title,
                       int x, int y, int w, int h)
{
   XVisualInfo *visInfo, visTemplate;
   int num_visuals;
   Window root, xwin;
   XSetWindowAttributes attr;
   unsigned long mask;
   EGLint vid;

   if (!eglGetConfigAttrib(_glut->dpy,
            win->config, EGL_NATIVE_VISUAL_ID, &vid))
      _glutFatal("failed to get visual id");

   /* The X window visual must match the EGL config */
   visTemplate.visualid = vid;
   visInfo = XGetVisualInfo(_glut->native_dpy,
         VisualIDMask, &visTemplate, &num_visuals);
   if (!visInfo)
      _glutFatal("failed to get an visual of id 0x%x", vid);

   root = RootWindow(_glut->native_dpy, DefaultScreen(_glut->native_dpy));

   /* window attributes */
   attr.background_pixel = 0;
   attr.border_pixel = 0;
   attr.colormap = XCreateColormap(_glut->native_dpy,
         root, visInfo->visual, AllocNone);
   attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
   mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

   xwin = XCreateWindow(_glut->native_dpy, root, x, y, w, h,
         0, visInfo->depth, InputOutput, visInfo->visual, mask, &attr);
   if (!xwin)
      _glutFatal("failed to create a window");

   XFree(visInfo);

   /* set hints and properties */
   {
      XSizeHints sizehints;
      sizehints.x = x;
      sizehints.y = y;
      sizehints.width  = w;
      sizehints.height = h;
      sizehints.flags = USSize | USPosition;
      XSetNormalHints(_glut->native_dpy, xwin, &sizehints);
      XSetStandardProperties(_glut->native_dpy, xwin,
            title, title, None, (char **) NULL, 0, &sizehints);
   }

   XMapWindow(_glut->native_dpy, xwin);

   win->native.u.window = xwin;
   win->native.width = w;
   win->native.height = h;
}

void
_glutNativeFiniWindow(struct glut_window *win)
{
   XDestroyWindow(_glut->native_dpy, win->native.u.window);
}

static int
lookup_keysym(KeySym sym)
{
   int special;

   switch (sym) {
   case XK_F1:
      special = GLUT_KEY_F1;
      break;
   case XK_F2:
      special = GLUT_KEY_F2;
      break;
   case XK_F3:
      special = GLUT_KEY_F3;
      break;
   case XK_F4:
      special = GLUT_KEY_F4;
      break;
   case XK_F5:
      special = GLUT_KEY_F5;
      break;
   case XK_F6:
      special = GLUT_KEY_F6;
      break;
   case XK_F7:
      special = GLUT_KEY_F7;
      break;
   case XK_F8:
      special = GLUT_KEY_F8;
      break;
   case XK_F9:
      special = GLUT_KEY_F9;
      break;
   case XK_F10:
      special = GLUT_KEY_F10;
      break;
   case XK_F11:
      special = GLUT_KEY_F11;
      break;
   case XK_F12:
      special = GLUT_KEY_F12;
      break;
   case XK_KP_Left:
   case XK_Left:
      special = GLUT_KEY_LEFT;
      break;
   case XK_KP_Up:
   case XK_Up:
      special = GLUT_KEY_UP;
      break;
   case XK_KP_Right:
   case XK_Right:
      special = GLUT_KEY_RIGHT;
      break;
   case XK_KP_Down:
   case XK_Down:
      special = GLUT_KEY_DOWN;
      break;
   default:
      special = -1;
      break;
   }

   return special;
}

static void
next_event(struct glut_window *win)
{
   int redraw = 0;
   XEvent event;

   if (!XPending(_glut->native_dpy)) {
      if (_glut->idle_cb)
         _glut->idle_cb();
      return;
   }

   XNextEvent(_glut->native_dpy, &event);

   switch (event.type) {
   case Expose:
      redraw = 1;
      break;
   case ConfigureNotify:
      win->native.width = event.xconfigure.width;
      win->native.height = event.xconfigure.height;
      if (win->reshape_cb)
         win->reshape_cb(win->native.width, win->native.height);
      break;
   case KeyPress:
      {
         char buffer[1];
         KeySym sym;
         int r;

         r = XLookupString(&event.xkey,
               buffer, sizeof(buffer), &sym, NULL);
         if (r && win->keyboard_cb) {
            win->keyboard_cb(buffer[0], event.xkey.x, event.xkey.y);
         }
         else if (!r && win->special_cb) {
            r = lookup_keysym(sym);
            if (r >= 0)
               win->special_cb(r, event.xkey.x, event.xkey.y);
         }
      }
      redraw = 1;
      break;
   default:
      ; /*no-op*/
   }

   _glut->redisplay = redraw;
}

void
_glutNativeEventLoop(void)
{
   while (1) {
      struct glut_window *win = _glut->current;

      next_event(win);

      if (_glut->redisplay) {
         _glut->redisplay = 0;

         if (win->display_cb)
            win->display_cb();
         eglSwapBuffers(_glut->dpy, win->surface);
      }
   }
}
