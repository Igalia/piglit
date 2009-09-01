/*
 * Copyright © 2009 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Ian Romanick <ian.d.romanick@intel.com>
 */

/**
 * \file sync_api.c
 * Simple test of the API for GL_ARB_sync.
 */

#include <stdio.h>
#include <string.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#if defined(FREEGLUT)
#include <GL/freeglut_ext.h>
#endif
#endif

#include "piglit-util.h"

#define FAIL_ON_ERROR(string)						\
	do {								\
		const GLenum err = glGetError();			\
		if (err != GL_NO_ERROR) {				\
			fprintf(stderr, "%s generated error 0x%04x\n", 	\
				string, err);				\
			pass = GL_FALSE;				\
			goto done;					\
		}							\
	} while (0)

#ifndef GL_ARB_sync
#define GL_ARB_sync 1

#define GL_MAX_SERVER_WAIT_TIMEOUT        0x9111
#define GL_OBJECT_TYPE                    0x9112
#define GL_SYNC_CONDITION                 0x9113
#define GL_SYNC_STATUS                    0x9114
#define GL_SYNC_FLAGS                     0x9115
#define GL_SYNC_FENCE                     0x9116
#define GL_SYNC_GPU_COMMANDS_COMPLETE     0x9117
#define GL_UNSIGNALED                     0x9118
#define GL_SIGNALED                       0x9119
#define GL_ALREADY_SIGNALED               0x911A
#define GL_TIMEOUT_EXPIRED                0x911B
#define GL_CONDITION_SATISFIED            0x911C
#define GL_WAIT_FAILED                    0x911D
#define GL_SYNC_FLUSH_COMMANDS_BIT        0x00000001
#define GL_TIMEOUT_IGNORED                0xFFFFFFFFFFFFFFFFull

typedef int64_t GLint64;
typedef uint64_t GLuint64;
typedef struct __GLsync *GLsync;

typedef GLsync (APIENTRYP PFNGLFENCESYNCPROC) (GLenum condition, GLbitfield flags);
typedef GLboolean (APIENTRYP PFNGLISSYNCPROC) (GLsync sync);
typedef void (APIENTRYP PFNGLDELETESYNCPROC) (GLsync sync);
typedef GLenum (APIENTRYP PFNGLCLIENTWAITSYNCPROC) (GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void (APIENTRYP PFNGLWAITSYNCPROC) (GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void (APIENTRYP PFNGLGETINTEGER64VPROC) (GLenum pname, GLint64 *params);
typedef void (APIENTRYP PFNGLGETSYNCIVPROC) (GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
#endif

static PFNGLFENCESYNCPROC pglFenceSync = NULL;
static PFNGLISSYNCPROC pglIsSync = NULL;
static PFNGLDELETESYNCPROC pglDeleteSync = NULL;
static PFNGLCLIENTWAITSYNCPROC pglClientWaitSync = NULL;
static PFNGLWAITSYNCPROC pglWaitSync = NULL;
static PFNGLGETINTEGER64VPROC pglGetInteger64v = NULL;
static PFNGLGETSYNCIVPROC pglGetSynciv = NULL;


#if defined(_MSC_VER)
#define GET_PROC_ADDR(s) wglGetProcAddress(s)
#else

#define GET_PROC_ADDR(s) glutGetProcAddress(s);
#endif

static GLboolean Automatic = GL_FALSE;

static void
init(void)
{
	piglit_require_extension("GL_ARB_sync");

	pglFenceSync = (PFNGLFENCESYNCPROC) GET_PROC_ADDR("glFenceSync");
	pglIsSync = (PFNGLISSYNCPROC) GET_PROC_ADDR("glIsSync");
	pglDeleteSync = (PFNGLDELETESYNCPROC) GET_PROC_ADDR("glDeleteSync");
	pglClientWaitSync = (PFNGLCLIENTWAITSYNCPROC) GET_PROC_ADDR("glClientWaitSync");
	pglWaitSync = (PFNGLWAITSYNCPROC) GET_PROC_ADDR("glWaitSync");
	pglGetInteger64v = (PFNGLGETINTEGER64VPROC) GET_PROC_ADDR("glGetInteger64v");
	pglGetSynciv = (PFNGLGETSYNCIVPROC) GET_PROC_ADDR("glGetSynciv");

	glClearColor(0.1, 0.1, 0.3, 0.0);
}


static void
reshape(int width, int height)
{
	glViewport(0, 0, (GLint) width, (GLint) height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -0.5, 1000.0);
	glMatrixMode(GL_MODELVIEW);
}


GLboolean
test_GetSynciv(GLsync sync, GLenum pname, GLint expect)
{
	GLboolean pass = GL_TRUE;
	GLint val;
	GLsizei len;

	(*pglGetSynciv)(sync, pname, 1, & len, & val);
	FAIL_ON_ERROR("glGetSynciv");
	if (len != 1) {
		fprintf(stderr, "glGetSynciv length of 0x%04x was %d\n",
			pname, len);
		pass = GL_FALSE;
	} else if (val != expect) {
		fprintf(stderr, "glGetSynciv of 0x%04x expected 0x%08x, "
			"got 0x%08x\n", pname, expect, val);
		pass = GL_FALSE;
	}

done:
	return pass;
}

static void
display(void)
{
	GLboolean pass = GL_TRUE;
	GLenum wait_val;
	GLsync sync;

	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_TRIANGLES);
	glColor3f(.8,0,0);
	glVertex3f(-0.9, -0.9, -30.0);
	glColor3f(0,.9,0);
	glVertex3f( 0.9, -0.9, -30.0);
	glColor3f(0,0,.7);
	glVertex3f( 0.0,  0.9, -30.0);
	glEnd();

	glGetError();

	sync = (*pglFenceSync)(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	FAIL_ON_ERROR("glFenceSync");

	if (! (*pglIsSync)(sync)) {
		fprintf(stderr, "IsSync(%p) failed\n", sync);
		pass = GL_FALSE;
		goto done;
	}
	FAIL_ON_ERROR("glIsSync");

	if (! test_GetSynciv(sync, GL_SYNC_CONDITION,
			     GL_SYNC_GPU_COMMANDS_COMPLETE)) {
		pass = GL_FALSE;
		goto done;
	}

	if (! test_GetSynciv(sync, GL_SYNC_FLAGS, 0)) {
		pass = GL_FALSE;
		goto done;
	}

	glFinish();

	/* After the glFinish, the sync *must* be signaled!
	 */
	if (! test_GetSynciv(sync, GL_SYNC_STATUS, GL_SIGNALED)) {
		pass = GL_FALSE;
		goto done;
	}


	/* Since the sync has already been signaled, the wait should return
	 * GL_ALREADY_SIGNALED.
	 */
	wait_val = (*pglClientWaitSync)(sync, 0, 1);
	FAIL_ON_ERROR("glClientWaitSync");

	if (wait_val != GL_ALREADY_SIGNALED) {
		fprintf(stderr, "glClientWaitSync expected 0x%08x, "
			"got 0x%08x\n", GL_ALREADY_SIGNALED, wait_val);
		pass = GL_FALSE;
	}

	(*pglDeleteSync)(sync);
	FAIL_ON_ERROR("glDeleteSync");

done:
	if (Automatic)
		piglit_report_result(pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE);
}


int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	if (argc == 2 && !strcmp(argv[1], "-auto"))
		Automatic = GL_TRUE;

	glutInitWindowSize(400, 300);
	glutInitDisplayMode(GLUT_RGB);
	glutCreateWindow("GL_ARB_sync API test");
	glutReshapeFunc(reshape);
	glutKeyboardFunc(piglit_escape_exit_key);
	glutDisplayFunc(display);

	init();

	glutMainLoop();
	return 0;
}
