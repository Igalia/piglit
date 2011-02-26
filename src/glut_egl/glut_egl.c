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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>

#include "EGL/egl.h"
#include "EGL/eglext.h"

#include "glut_eglint.h"

static struct glut_state _glut_state = {
   .api_mask = GLUT_OPENGL_ES1_BIT,
   .display_mode = GLUT_RGB,
   .window_width = 300,
   .window_height = 300,
   .verbose = 0,
   .num_windows = 0,
};

struct glut_state *_glut = &_glut_state;

void
_glutFatal(char *format, ...)
{
  va_list args;

  va_start(args, format);

  fprintf(stderr, "GLUT_EGL: ");
  vfprintf(stderr, format, args);
  va_end(args);
  putc('\n', stderr);

  exit(1);
}

/* return current time (in milliseconds) */
int
_glutNow(void)
{
   struct timeval tv;
#ifdef __VMS
   (void) gettimeofday(&tv, NULL );
#else
   struct timezone tz;
   (void) gettimeofday(&tv, &tz);
#endif
   return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static void
_glutDestroyWindow(struct glut_window *win)
{
   if (_glut->surface_type != EGL_PBUFFER_BIT &&
       _glut->surface_type != EGL_SCREEN_BIT_MESA)
      eglDestroySurface(_glut->dpy, win->surface);

   _glutNativeFiniWindow(win);

   eglDestroyContext(_glut->dpy, win->context);
}

static EGLConfig
_glutChooseConfig(void)
{
   EGLConfig config;
   EGLint config_attribs[32];
   EGLint renderable_type, num_configs, i;

   i = 0;
   config_attribs[i++] = EGL_RED_SIZE;
   config_attribs[i++] = 1;
   config_attribs[i++] = EGL_GREEN_SIZE;
   config_attribs[i++] = 1;
   config_attribs[i++] = EGL_BLUE_SIZE;
   config_attribs[i++] = 1;

   config_attribs[i++] = EGL_ALPHA_SIZE;
   if (_glut->display_mode & GLUT_ALPHA)
      config_attribs[i++] = 1;
   else
      config_attribs[i++] = 0;

   config_attribs[i++] = EGL_DEPTH_SIZE;
   if (_glut->display_mode & GLUT_DEPTH)
      config_attribs[i++] = 1;
   else
      config_attribs[i++] = 0;

   config_attribs[i++] = EGL_STENCIL_SIZE;
   if (_glut->display_mode & GLUT_STENCIL)
      config_attribs[i++] = 1;
   else
      config_attribs[i++] = 0;

   config_attribs[i++] = EGL_SURFACE_TYPE;
   config_attribs[i++] = _glut->surface_type;

   config_attribs[i++] = EGL_RENDERABLE_TYPE;
   renderable_type = 0x0;
   if (_glut->api_mask & GLUT_OPENGL_BIT)
      renderable_type |= EGL_OPENGL_BIT;
   if (_glut->api_mask & GLUT_OPENGL_ES1_BIT)
      renderable_type |= EGL_OPENGL_ES_BIT;
   if (_glut->api_mask & GLUT_OPENGL_ES2_BIT)
      renderable_type |= EGL_OPENGL_ES2_BIT;
   if (_glut->api_mask & GLUT_OPENVG_BIT)
      renderable_type |= EGL_OPENVG_BIT;
   config_attribs[i++] = renderable_type;

   config_attribs[i] = EGL_NONE;

   if (!eglChooseConfig(_glut->dpy,
            config_attribs, &config, 1, &num_configs) || !num_configs)
      _glutFatal("failed to choose a config");

   return config;
}

static struct glut_window *
_glutCreateWindow(const char *title, int x, int y, int w, int h)
{
   struct glut_window *win;
   EGLint context_attribs[4];
   EGLint api, i;

   win = calloc(1, sizeof(*win));
   if (!win)
      _glutFatal("failed to allocate window");

   win->config = _glutChooseConfig();

   i = 0;
   context_attribs[i] = EGL_NONE;

   /* multiple APIs? */

   api = EGL_OPENGL_ES_API;
   if (_glut->api_mask & GLUT_OPENGL_BIT) {
      api = EGL_OPENGL_API;
   }
   else if (_glut->api_mask & GLUT_OPENVG_BIT) {
      api = EGL_OPENVG_API;
   }
   else if (_glut->api_mask & GLUT_OPENGL_ES2_BIT) {
      context_attribs[i++] = EGL_CONTEXT_CLIENT_VERSION;
      context_attribs[i++] = 2;
   }

   context_attribs[i] = EGL_NONE;

   eglBindAPI(api);
   win->context = eglCreateContext(_glut->dpy,
         win->config, EGL_NO_CONTEXT, context_attribs);
   if (!win->context)
      _glutFatal("failed to create context");

   _glutNativeInitWindow(win, title, x, y, w, h);
   switch (_glut->surface_type) {
   case EGL_WINDOW_BIT:
      win->surface = eglCreateWindowSurface(_glut->dpy,
            win->config, win->native.u.window, NULL);
      break;
   case EGL_PIXMAP_BIT:
      win->surface = eglCreatePixmapSurface(_glut->dpy,
            win->config, win->native.u.pixmap, NULL);
      break;
   case EGL_PBUFFER_BIT:
   case EGL_SCREEN_BIT_MESA:
      win->surface = win->native.u.surface;
      break;
   default:
      break;
   }
   if (win->surface == EGL_NO_SURFACE)
      _glutFatal("failed to create surface");

   return win;
}

void
glutInitAPIMask(int mask)
{
   _glut->api_mask = mask;
}

void
glutInitDisplayMode(unsigned int mode)
{
   _glut->display_mode = mode;
}

void
glutInitWindowPosition(int x, int y)
{
}

void
glutInitWindowSize(int width, int height)
{
   _glut->window_width = width;
   _glut->window_height = height;
}

void
glutInit(int *argcp, char **argv)
{
   int i;

   for (i = 1; i < *argcp; i++) {
      if (strcmp(argv[i], "-display") == 0)
         _glut->display_name = argv[++i];
      else if (strcmp(argv[i], "-info") == 0) {
         _glut->verbose = 1;
      }
   }

   _glutNativeInitDisplay();
   _glut->dpy = eglGetDisplay(_glut->native_dpy);

   if (!eglInitialize(_glut->dpy, &_glut->major, &_glut->minor))
      _glutFatal("failed to initialize EGL display");

   _glut->init_time = _glutNow();

   printf("EGL_VERSION = %s\n", eglQueryString(_glut->dpy, EGL_VERSION));
   if (_glut->verbose) {
      printf("EGL_VENDOR = %s\n", eglQueryString(_glut->dpy, EGL_VENDOR));
      printf("EGL_EXTENSIONS = %s\n",
            eglQueryString(_glut->dpy, EGL_EXTENSIONS));
      printf("EGL_CLIENT_APIS = %s\n",
            eglQueryString(_glut->dpy, EGL_CLIENT_APIS));
   }
}

int
glutGet(int state)
{
   int val;

   switch (state) {
   case GLUT_ELAPSED_TIME:
      val = _glutNow() - _glut->init_time;
      break;
   default:
      val = -1;
      break;
   }

   return val;
}

void
glutIdleFunc(GLUT_EGLidleCB func)
{
   _glut->idle_cb = func;
}

void
glutPostRedisplay(void)
{
   _glut->redisplay = 1;
}

void
glutMainLoop(void)
{
   struct glut_window *win = _glut->current;

   if (!win)
      _glutFatal("no window is created\n");

   if (win->reshape_cb)
      win->reshape_cb(win->native.width, win->native.height);

   _glutNativeEventLoop();
}

static void
_glutFini(void)
{
   eglTerminate(_glut->dpy);
   _glutNativeFiniDisplay();
}

void
glutDestroyWindow(int win)
{
   struct glut_window *window = _glut->current;

   if (window->index != win)
      return;

   /* XXX it causes some bug in st/egl KMS backend */
   if ( _glut->surface_type != EGL_SCREEN_BIT_MESA)
      eglMakeCurrent(_glut->dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

   _glutDestroyWindow(_glut->current);
}

static void
_glutDefaultKeyboard(unsigned char key, int x, int y)
{
   if (key == 27) {
      if (_glut->current)
         glutDestroyWindow(_glut->current->index);
      _glutFini();

      exit(0);
   }
}

int
glutCreateWindow(const char *title)
{
   struct glut_window *win;

   win = _glutCreateWindow(title, 0, 0,
         _glut->window_width, _glut->window_height);

   win->index = _glut->num_windows++;
   win->reshape_cb = NULL;
   win->display_cb = NULL;
   win->keyboard_cb = _glutDefaultKeyboard;
   win->special_cb = NULL;

   if (!eglMakeCurrent(_glut->dpy, win->surface, win->surface, win->context))
      _glutFatal("failed to make window current");
   _glut->current = win;

   return win->index;
}

int
glutGetWindowWidth(void)
{
   struct glut_window *win = _glut->current;
   return win->native.width;
}

int
glutGetWindowHeight(void)
{
   struct glut_window *win = _glut->current;
   return win->native.height;
}

void
glutDisplayFunc(GLUT_EGLdisplayCB func)
{
   struct glut_window *win = _glut->current;
   win->display_cb = func;

}

void
glutReshapeFunc(GLUT_EGLreshapeCB func)
{
   struct glut_window *win = _glut->current;
   win->reshape_cb = func;
}

void
glutKeyboardFunc(GLUT_EGLkeyboardCB func)
{
   struct glut_window *win = _glut->current;
   win->keyboard_cb = func;
}

void
glutSpecialFunc(GLUT_EGLspecialCB func)
{
   struct glut_window *win = _glut->current;
   win->special_cb = func;
}

void
glutSwapBuffers(void)
{
}
