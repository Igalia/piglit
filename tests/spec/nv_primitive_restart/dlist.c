/*
 * Copyright © 2020 Intel Corporation
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
 */

/**
 * \file dlist.c
 * Exercise various interactions of primitive restart with display lists.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 12;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const float verts[10 * 2] = {
	-1.0, -1.0, /* 0 */
	-0.5, -1.0,
	+0.0, -1.0,
	-1.0, -0.5, /* 3 */
	-0.5, -0.5,
	+0.0, -0.5,
	-1.0, +0.0, /* 6 */
	-0.5, +0.0,
	+0.0, +0.0,
	-1.0, -0.5, /* 9 */
};

static const float colors[10 * 3] = {
	0.0, 1.0, 0.0,
	0.0, 1.0, 0.0,
	0.0, 1.0, 0.0,
	0.0, 1.0, 0.0,
	0.0, 1.0, 0.0,
	0.0, 1.0, 0.0,
	0.0, 1.0, 0.0,
	0.0, 1.0, 0.0,
	0.0, 1.0, 0.0,
	1.0, 0.0, 1.0,
};

static const int elts[] = {
	3, 0, 4,
	1,
	5,
	2,

	9,        /* restart index */

	6, 3, 7,
	4,
	8,
	5,
};

static void
lower_left(void)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	/* Disable backface culling.  The position data and drawing indices
	 * are crafted such that the same pixels will be covered even if
	 * primitive restart state is ignored.  However, that will result in
	 * an extra triangle being drawn with incorrect colors.
	 */
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glLoadIdentity();

	glVertexPointer(2, GL_FLOAT, 0, verts);
	glColorPointer(3, GL_FLOAT, 0, colors);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glEnableClientState(GL_PRIMITIVE_RESTART_NV);
	glPrimitiveRestartIndexNV(9);

	const GLuint dlist = glGenLists(1);

	glNewList(dlist, GL_COMPILE);
	glDrawElements(GL_TRIANGLE_STRIP, ARRAY_SIZE(elts), GL_UNSIGNED_INT,
		       elts);
	glEndList();

	/* Since the restart index is client state, it should not have any
	 * effect on glCallList.
	 */
	glPrimitiveRestartIndexNV(0);
	glCallList(dlist);

	glDeleteLists(dlist, 1);

	glPopClientAttrib();
	glPopAttrib();
}

static void
lower_right(void)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
	glLoadIdentity();

	/* Disable backface culling.  The position data and drawing indices
	 * are crafted such that the same pixels will be covered even if
	 * primitive restart state is ignored.  However, that will result in
	 * an extra triangle being drawn with incorrect colors.
	 */
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glVertexPointer(2, GL_FLOAT, 0, verts);
	glColorPointer(3, GL_FLOAT, 0, colors);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	/* The real reset index is 9, but set it to 0 and disable primitive
	 * restart.
	 */
	glPrimitiveRestartIndexNV(0);
	glDisableClientState(GL_PRIMITIVE_RESTART_NV);

	const GLuint dlist = glGenLists(1);
	glNewList(dlist, GL_COMPILE);

	glBegin(GL_TRIANGLE_STRIP);

	for (unsigned i = 0; i < ARRAY_SIZE(elts); i++) {
		GLuint idx = elts[i];

		if (idx == 9) {
			/* 0 only appears once in the elts list, and that is
			 * before 9.  Once the 9 is encountered, enable
			 * primitive restart, and emit 0 (the restart index)
			 * instead of 9.
			 */
			glEnableClientState(GL_PRIMITIVE_RESTART_NV);
			idx = 0;
		}

		glArrayElement(idx);
	}

	glEnd();

	glEndList();

	glTranslatef(1.0f, 0.0f, 0.0f);
	glCallList(dlist);

	glDeleteLists(dlist, 1);

	glPopClientAttrib();
	glPopAttrib();
}

static void
upper_left(void)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
	glLoadIdentity();

	/* Disable backface culling.  The position data and drawing indices
	 * are crafted such that the same pixels will be covered even if
	 * primitive restart state is ignored.  However, that will result in
	 * an extra triangle being drawn with incorrect colors.
	 */
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glVertexPointer(2, GL_FLOAT, 0, verts);
	glColorPointer(3, GL_FLOAT, 0, colors);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glPrimitiveRestartIndexNV(0);
	glDisableClientState(GL_PRIMITIVE_RESTART_NV);

	const GLuint dlist = glGenLists(1);
	glNewList(dlist, GL_COMPILE);

	glBegin(GL_TRIANGLE_STRIP);

	for (unsigned i = 0; i < ARRAY_SIZE(elts); i++) {
		if (elts[i] == 9) {
			/* The GL_NV_primitive_restart spec doesn't explicitly
			 * say whether or not the GL_PRIMITIVE_RESTART_NV
			 * affects glPrimitiveRestartNV, but it _implies_ that
			 * it is not affected.  GL_PRIMITIVE_RESTART_NV is
			 * client state, but GLX protocol is (partially)
			 * defined for glPrimitiveRestartNV.  The idea is that
			 * when the GLX client library decomposes
			 * glDrawElements into immediate mode drawing
			 * commands, it will emit glPrimitiveRestartNV
			 * (instead of glVertex, etc.) when the restart index
			 * is encountered.
			 */
			glPrimitiveRestartNV();
		} else
			glArrayElement(elts[i]);
	}

	glEnd();

	glEndList();

	glTranslatef(0.0f, 1.0f, 0.0f);
	glCallList(dlist);

	glDeleteLists(dlist, 1);

	glPopClientAttrib();
	glPopAttrib();
}

static void
upper_right(void)
{
	static const int elts[] = {
		3, 0, 4,
		1,
		5,
		2,

		0x12345678, /* restart index */

		6, 3, 7,
		4,
		8,
		5,
	};

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	/* Disable backface culling.  The position data and drawing indices
	 * are crafted such that the same pixels will be covered even if
	 * primitive restart state is ignored.  However, that will result in
	 * an extra triangle being drawn with incorrect colors.
	 */
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glLoadIdentity();

	glVertexPointer(2, GL_FLOAT, 0, verts);
	glColorPointer(3, GL_FLOAT, 0, colors);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	const GLuint dlist = glGenLists(1);

	/* At least at the time of this writing, doing this same thing using
	 * glEnable(GL_PRIMITIVE_RESTART) and glPrimitiveRestartIndex(...)
	 * leads to a segfault during display list compilation on Mesa.
	 */
	glNewList(dlist, GL_COMPILE);
	glEnableClientState(GL_PRIMITIVE_RESTART_NV);
	glPrimitiveRestartIndexNV(0x12345678);
	glDrawElements(GL_TRIANGLE_STRIP, ARRAY_SIZE(elts), GL_UNSIGNED_INT,
		       elts);
	glEndList();

	/* Since the primitive restart enable is client state, it should not
	 * have any effect on glCallList.
	 */
	glDisableClientState(GL_PRIMITIVE_RESTART_NV);
	glTranslatef(1.0f, 1.0f, 0.0f);
	glCallList(dlist);

	glDeleteLists(dlist, 1);

	glPopClientAttrib();
	glPopAttrib();
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_NV_primitive_restart");
}

enum piglit_result
piglit_display()
{
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	lower_left();
	lower_right();
	upper_left();
	upper_right();

	static const float green[] = { 0.0, 1.0, 0.0, 1.0 };

	bool pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green);
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
