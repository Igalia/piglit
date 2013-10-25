/* Copyright © 2013 Intel Corporation
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

#pragma once
#ifndef QUERY_RENDERER_COMMON_H
#define QUERY_RENDERER_COMMON_H

#ifndef GLX_MESA_query_renderer
#define GLX_MESA_query_renderer 1

#define GLX_RENDERER_VENDOR_ID_MESA                      0x8183
#define GLX_RENDERER_DEVICE_ID_MESA                      0x8184
#define GLX_RENDERER_VERSION_MESA                        0x8185
#define GLX_RENDERER_ACCELERATED_MESA                    0x8186
#define GLX_RENDERER_VIDEO_MEMORY_MESA                   0x8187
#define GLX_RENDERER_UNIFIED_MEMORY_ARCHITECTURE_MESA    0x8188
#define GLX_RENDERER_PREFERRED_PROFILE_MESA              0x8189
#define GLX_RENDERER_OPENGL_CORE_PROFILE_VERSION_MESA    0x818A
#define GLX_RENDERER_OPENGL_COMPATIBILITY_PROFILE_VERSION_MESA    0x818B
#define GLX_RENDERER_OPENGL_ES_PROFILE_VERSION_MESA      0x818C
#define GLX_RENDERER_OPENGL_ES2_PROFILE_VERSION_MESA     0x818D
#define GLX_RENDERER_ID_MESA                             0x818E

typedef Bool (*PFNGLXQUERYRENDERERINTEGERMESAPROC) (Display *dpy, int screen, int renderer, int attribute, unsigned int *value);
typedef Bool (*PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC) (int attribute, unsigned int *value);
typedef const char *(*PFNGLXQUERYRENDERERSTRINGMESAPROC) (Display *dpy, int screen, int renderer, int attribute);
typedef const char *(*PFNGLXQUERYCURRENTRENDERERSTRINGMESAPROC) (int attribute);
#endif /* GLX_MESA_query_renderer */

PFNGLXQUERYRENDERERSTRINGMESAPROC piglit_glXQueryRendererStringMESA;
PFNGLXQUERYCURRENTRENDERERSTRINGMESAPROC piglit_glXQueryCurrentRendererStringMESA;
PFNGLXQUERYRENDERERINTEGERMESAPROC piglit_glXQueryRendererIntegerMESA;
PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC piglit_glXQueryCurrentRendererIntegerMESA;
PFNGLXCREATECONTEXTATTRIBSARBPROC piglit_glXCreateContextAttribsARB;

#define glXQueryRendererStringMESA(dpy, screen, renderer, attribute) \
	(*piglit_glXQueryRendererStringMESA)(dpy, screen, renderer, attribute)

#define glXQueryCurrentRendererStringMESA(attribute) \
	(*piglit_glXQueryCurrentRendererStringMESA)(attribute)

#define glXQueryRendererIntegerMESA(dpy, screen, renderer, attribute, value) \
	(*piglit_glXQueryRendererIntegerMESA)(dpy, screen, renderer, attribute, value)

#define glXQueryCurrentRendererIntegerMESA(attribute, value) \
	(*piglit_glXQueryCurrentRendererIntegerMESA)(attribute, value)

#define glXCreateContextAttribsARB(dpy, fbconfig, share, direct, attrib) \
	(*piglit_glXCreateContextAttribsARB)(dpy, fbconfig, share, direct, attrib)

void initialize_function_pointers(Display *);

#endif /* QUERY_RENDERER_COMMON_H */
