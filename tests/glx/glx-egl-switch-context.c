/*
 * Copyright © 2019 Intel Corporation
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
 * \file glx-egl-switch-context.c
 *
 * Test aims to reproduce SIGSEGV on i965 appearing on certain
 * sequences of switching glx and egl contexts. Particular sequence
 * that leads to crash:
 *
 * 1. Make glx context current
 * 2. Make egl context current
 * 3. Drop glx context
 * 4. Make egl context current
 *
 * In order to reproduce the crash you also need to export the loader:
 * export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PATH_TO_MESA_BUILD/lib
 *
 * \author Yevhenii Kolesnikov <yevhenii.kolesnikov@globallogic.com>
 */

#include "piglit-util-gl.h"
#include "piglit-util-egl.h"
#include "piglit-glx-util.h"

EGLint major_version, minor_version;

int piglit_width = 160,
    piglit_height = 160;

int main(int argc, char **argv)
{
    Display *dpy;
    EGLDisplay dpy_egl;
    XVisualInfo *visinfo;
    GLXFBConfig fbconfig;
    Window win;
    GLXWindow glxWin;
    GLXContext ctx_glx;
    EGLContext ctx_egl;

    bool pass = true;

    dpy = piglit_get_glx_display();
    visinfo = piglit_get_glx_visual(dpy);
    fbconfig = piglit_glx_get_fbconfig_for_visinfo(dpy, visinfo);
    win = piglit_get_glx_window(dpy, visinfo);
    glxWin = glXCreateWindow(dpy, fbconfig, win, NULL);

    dpy_egl = eglGetDisplay(dpy);
    if (!dpy_egl)
        piglit_report_result(PIGLIT_SKIP);
    if (!eglInitialize(dpy_egl, &major_version, &minor_version))
        piglit_report_result(PIGLIT_SKIP);

    ctx_glx = glXCreateContext(dpy, visinfo, NULL, True);
    ctx_egl = eglCreateContext(dpy_egl, EGL_NO_CONFIG_KHR, EGL_NO_CONTEXT, NULL);

    glXMakeContextCurrent(dpy, glxWin, glxWin, ctx_glx);
    eglMakeCurrent(dpy_egl, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx_egl);
    glXMakeContextCurrent(dpy, None, None, NULL);
    eglMakeCurrent(dpy_egl, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx_egl);
    eglMakeCurrent(dpy_egl, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    glXDestroyContext(dpy, ctx_glx);
    eglDestroyContext(dpy_egl, ctx_egl);

    eglTerminate(dpy_egl);

    XFree(visinfo);

    piglit_report_result(PIGLIT_PASS);

    return pass;
}
