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

#ifndef GLUT_EGL_H
#define GLUT_EGL_H

enum {
   GLUT_RGB = 0,
   GLUT_RGBA = 0,
   GLUT_INDEX = 1,
   GLUT_SINGLE = 0,
   GLUT_DOUBLE = 2,
   GLUT_ACCUM = 4,
   GLUT_ALPHA = 8,
   GLUT_DEPTH = 16,
   GLUT_STENCIL = 32,
};

/* used by glut_eglInitAPIMask */
enum {
   GLUT_EGL_OPENGL_BIT     = 0x1,
   GLUT_EGL_OPENGL_ES1_BIT = 0x2,
   GLUT_EGL_OPENGL_ES2_BIT = 0x4,
   GLUT_EGL_OPENVG_BIT     = 0x8
};

/* used by GLUT_EGLspecialCB */
enum {
   /* function keys */
   GLUT_KEY_F1  = 1,
   GLUT_KEY_F2  = 2,
   GLUT_KEY_F3  = 3,
   GLUT_KEY_F4  = 4,
   GLUT_KEY_F5  = 5,
   GLUT_KEY_F6  = 6,
   GLUT_KEY_F7  = 7,
   GLUT_KEY_F8  = 8,
   GLUT_KEY_F9  = 9,
   GLUT_KEY_F10 = 10,
   GLUT_KEY_F11 = 11,
   GLUT_KEY_F12 = 12,

   /* directional keys */
   GLUT_KEY_LEFT = 100,
   GLUT_KEY_UP = 101,
   GLUT_KEY_RIGHT = 102,
   GLUT_KEY_DOWN = 103,
};

/* used by glutGet */
enum {
   GLUT_EGL_ELAPSED_TIME
};

typedef void (*GLUT_EGLidleCB)(void);
typedef void (*GLUT_EGLreshapeCB)(int, int);
typedef void (*GLUT_EGLdisplayCB)(void);
typedef void (*GLUT_EGLkeyboardCB)(unsigned char, int, int);
typedef void (*GLUT_EGLspecialCB)(int, int, int);

void glut_eglInitAPIMask(int mask);
void glutInitDisplayMode(unsigned int mode);
void glutInitWindowPosition(int x, int y);
void glutInitWindowSize(int width, int height);
void glutInit(int *argcp, char **argv);

int glutGet(int state);

void glutIdleFunc(GLUT_EGLidleCB func);
void glutPostRedisplay(void);

void glutMainLoop(void);

int glutCreateWindow(const char *title);
void glutDestroyWindow(int win);

int glut_eglGetWindowWidth(void);
int glut_eglGetWindowHeight(void);

void glutDisplayFunc(GLUT_EGLdisplayCB func);
void glutReshapeFunc(GLUT_EGLreshapeCB func);
void glutKeyboardFunc(GLUT_EGLkeyboardCB func);
void glutSpecialFunc(GLUT_EGLspecialCB func);
void glutSwapBuffers(void);

#endif /* GLUT_EGL_H */
