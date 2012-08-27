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

#pragma once

enum glut_display_mode {
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

/* used by glutInitAPIMask */
enum glut_api {
	GLUT_OPENGL_BIT     = 0x1,
	GLUT_OPENGL_ES1_BIT = 0x2,
	GLUT_OPENGL_ES2_BIT = 0x4,
};

typedef void (*GLUT_EGLreshapeCB)(int, int);
typedef void (*GLUT_EGLdisplayCB)(void);
typedef void (*GLUT_EGLkeyboardCB)(unsigned char, int, int);

void glutInitAPIMask(int mask);
void glutInitDisplayMode(unsigned int mode);
void glutInitWindowPosition(int x, int y);
void glutInitWindowSize(int width, int height);
void glutInit(int *argcp, char **argv);

void glutPostRedisplay(void);

void glutMainLoop(void);

/**
 * Create the window, but do not show it.
 */
int glutCreateWindow(const char *title);

void glutDestroyWindow(int win);
void glutShowWindow(int win);

void glutDisplayFunc(GLUT_EGLdisplayCB func);
void glutReshapeFunc(GLUT_EGLreshapeCB func);
void glutKeyboardFunc(GLUT_EGLkeyboardCB func);
void glutSwapBuffers(void);
