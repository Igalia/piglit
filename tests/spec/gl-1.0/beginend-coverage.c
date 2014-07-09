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

/** @file beginend-coverage.c
 *
 * Test that various functions correctly throw errors or not while
 * inside a glBegin()/glEnd() block, whether inside or outside a
 * display list.
 *
 * From the GL 1.0 specification section 2.6.3, "GL Commands within
 * Begin/End"
 *
 *     "The only GL commands that are allowed within any Begin/End
 *      pairs are the commands for specifying vertex coordinates,
 *      vertex color, normal coordinates, and texture coordinates
 *      (Vertex, Color, Index, Normal, TexCoord), EvalCoord and
 *      EvalPoint commands (see section 5.1), commands for specifying
 *      lighting material parameters (Material commands see section
 *      2.12.2), display list invocation commands (CallList and
 *      CallLists see section 5.4), and the EdgeFlag
 *      command. Executing Begin after Begin has already been executed
 *      but before an End is issued generates the INVALID OPERATION
 *      error, as does executing End without a previous corresponding
 *      Begin. Executing any other GL command within Begin/End results
 *      in the error INVALID OPERATION."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

struct test {
	const char *name;
	void (*func)(void);
};

static uint32_t junk_storage[1024];
static void *junk = junk_storage;
static const int onei = 1;
static const float onef = 1.0;
static GLuint some_dlist;
static GLuint newlist_dlist;
static GLuint deletelists_dlist;
static GLuint fbo_attachment;
static GLint fbo_binding;

#define TEST_FUNC(name, args)			\
	static void test_##name(void)		\
	{					\
		name args;			\
	}
;
TEST_FUNC(glAccum, (GL_ADD, 1.0))
TEST_FUNC(glAlphaFunc, (GL_GREATER, 0.0))
TEST_FUNC(glArrayElement, (0))
TEST_FUNC(glBegin, (GL_POINTS))
TEST_FUNC(glBitmap, (1, 1, 0, 0, 0, 0, (const GLubyte *)junk))
TEST_FUNC(glBlendFunc, (GL_ZERO, GL_ZERO))
TEST_FUNC(glCallList, (some_dlist))
/* relies on ListBase == 0 */
TEST_FUNC(glCallLists, (1, GL_UNSIGNED_INT, &some_dlist))
TEST_FUNC(glClear, (GL_COLOR_BUFFER_BIT))
TEST_FUNC(glClearAccum, (0, 0, 0, 0))
TEST_FUNC(glClearColor, (0, 0, 0 ,0))
TEST_FUNC(glClearDepth, (0))
TEST_FUNC(glClearIndex, (0))
TEST_FUNC(glClearStencil, (0))
TEST_FUNC(glClipPlane, (GL_CLIP_PLANE0, junk))
TEST_FUNC(glColor3b, (0, 0, 0))
TEST_FUNC(glColor3bv, (junk))
TEST_FUNC(glColor3d, (0, 0, 0))
TEST_FUNC(glColor3dv, (junk))
TEST_FUNC(glColor3f, (0, 0, 0))
TEST_FUNC(glColor3fv, (junk))
TEST_FUNC(glColor3i, (0, 0, 0))
TEST_FUNC(glColor3iv, (junk))
TEST_FUNC(glColor3s, (0, 0, 0))
TEST_FUNC(glColor3sv, (junk))
TEST_FUNC(glColor3ub, (0, 0, 0))
TEST_FUNC(glColor3ubv, (junk))
TEST_FUNC(glColor3ui, (0, 0, 0))
TEST_FUNC(glColor3uiv, (junk))
TEST_FUNC(glColor3us, (0, 0, 0))
TEST_FUNC(glColor3usv, (junk))
TEST_FUNC(glColor4b, (0, 0, 0, 0))
TEST_FUNC(glColor4bv, (junk))
TEST_FUNC(glColor4d, (0, 0, 0, 0))
TEST_FUNC(glColor4dv, (junk))
TEST_FUNC(glColor4f, (0, 0, 0, 0))
TEST_FUNC(glColor4fv, (junk))
TEST_FUNC(glColor4i, (0, 0, 0, 0))
TEST_FUNC(glColor4iv, (junk))
TEST_FUNC(glColor4s, (0, 0, 0, 0))
TEST_FUNC(glColor4sv, (junk))
TEST_FUNC(glColor4ub, (0, 0, 0, 0))
TEST_FUNC(glColor4ubv, (junk))
TEST_FUNC(glColor4ui, (0, 0, 0, 0))
TEST_FUNC(glColor4uiv, (junk))
TEST_FUNC(glColor4us, (0, 0, 0, 0))
TEST_FUNC(glColor4usv, (junk))
TEST_FUNC(glColorMask, (0, 0, 0, 0))
TEST_FUNC(glColorMaterial, (GL_FRONT, GL_AMBIENT))
TEST_FUNC(glColorPointer, (4, GL_FLOAT, 0, junk))
TEST_FUNC(glCopyPixels, (0, 0, 1, 1, GL_COLOR))
TEST_FUNC(glCullFace, (GL_FRONT))
TEST_FUNC(glDepthFunc, (GL_GREATER))
TEST_FUNC(glDepthMask, (0))
TEST_FUNC(glDepthRange, (0, 1))
TEST_FUNC(glDeleteLists, (deletelists_dlist, 1))
TEST_FUNC(glDisable, (GL_DEPTH_TEST))
TEST_FUNC(glDisableClientState, (GL_VERTEX_ARRAY))
TEST_FUNC(glDrawArrays, (GL_POINTS, 0, 1))
TEST_FUNC(glDrawBuffer, (fbo_attachment))
TEST_FUNC(glDrawElements, (GL_POINTS, 1, GL_UNSIGNED_INT, junk))
TEST_FUNC(glDrawPixels, (1, 1, GL_RGBA, GL_FLOAT, junk))
TEST_FUNC(glEdgeFlag, (0))
TEST_FUNC(glEdgeFlagPointer, (0, junk))
TEST_FUNC(glEdgeFlagv, (junk))
TEST_FUNC(glEnable, (GL_DEPTH_TEST))
TEST_FUNC(glEnableClientState, (GL_VERTEX_ARRAY))
/* No particular test for End inside of a begin/end block, obviously. */
TEST_FUNC(glEndList, ())
TEST_FUNC(glFrontFace, (GL_CW))
TEST_FUNC(glGenLists, (1))
TEST_FUNC(glGetBooleanv, (GL_DEPTH_TEST, junk))
TEST_FUNC(glGetClipPlane, (0, junk))
TEST_FUNC(glGetDoublev, (GL_DEPTH_TEST, junk))
TEST_FUNC(glGetError, ())
TEST_FUNC(glGetFloatv, (GL_DEPTH_TEST, junk))
TEST_FUNC(glGetIntegerv, (GL_DEPTH_TEST, junk))
TEST_FUNC(glGetLightfv, (GL_LIGHT0, GL_SPOT_CUTOFF, junk))
TEST_FUNC(glGetLightiv, (GL_LIGHT0, GL_SPOT_CUTOFF, junk))
TEST_FUNC(glGetMaterialfv, (GL_FRONT, GL_AMBIENT, junk))
TEST_FUNC(glGetMaterialiv, (GL_FRONT, GL_AMBIENT, junk))
TEST_FUNC(glGetPolygonStipple, (junk))
TEST_FUNC(glGetString, (GL_EXTENSIONS))
TEST_FUNC(glGetPixelMapfv, (GL_PIXEL_MAP_S_TO_S, junk))
TEST_FUNC(glGetPixelMapuiv, (GL_PIXEL_MAP_S_TO_S, junk))
TEST_FUNC(glGetPixelMapusv, (GL_PIXEL_MAP_S_TO_S, junk))
TEST_FUNC(glGetPointerv, (GL_VERTEX_ARRAY_POINTER, junk))
TEST_FUNC(glGetTexEnvfv, (GL_TEXTURE_2D, GL_ALPHA_SCALE, junk))
TEST_FUNC(glGetTexEnviv, (GL_TEXTURE_2D, GL_ALPHA_SCALE, junk))
TEST_FUNC(glGetTexGendv, (GL_S, GL_OBJECT_PLANE, junk))
TEST_FUNC(glGetTexGenfv, (GL_S, GL_OBJECT_PLANE, junk))
TEST_FUNC(glGetTexGeniv, (GL_S, GL_OBJECT_PLANE, junk))
TEST_FUNC(glGetTexImage, (GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, junk))
TEST_FUNC(glGetTexLevelParameterfv, (GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, junk))
TEST_FUNC(glGetTexLevelParameteriv, (GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, junk))
TEST_FUNC(glGetTexParameterfv, (GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, junk))
TEST_FUNC(glGetTexParameteriv, (GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, junk))
TEST_FUNC(glFinish, ())
TEST_FUNC(glFlush, ())
TEST_FUNC(glFrustum, (0, 1, 0, 1, 0.1, 1))
TEST_FUNC(glHint, (GL_FOG_HINT, GL_NICEST))
TEST_FUNC(glIndexMask, (0))
TEST_FUNC(glIsEnabled, (GL_DEPTH_TEST))
TEST_FUNC(glIsList, (0))
TEST_FUNC(glIndexd, (0))
TEST_FUNC(glIndexdv, (junk))
TEST_FUNC(glIndexf, (0))
TEST_FUNC(glIndexfv, (junk))
TEST_FUNC(glIndexi, (0))
TEST_FUNC(glIndexiv, (junk))
TEST_FUNC(glIndexPointer, (GL_BYTE, 0, junk))
TEST_FUNC(glIndexs, (0))
TEST_FUNC(glIndexsv, (junk))
TEST_FUNC(glIndexub, (0))
TEST_FUNC(glIndexubv, (junk))
TEST_FUNC(glInterleavedArrays, (GL_V2F, 0, junk))
TEST_FUNC(glLightf, (GL_LIGHT0, GL_SPOT_CUTOFF, 0))
TEST_FUNC(glLightfv, (GL_LIGHT0, GL_SPOT_CUTOFF, junk))
TEST_FUNC(glLighti, (GL_LIGHT0, GL_SPOT_CUTOFF, 0))
TEST_FUNC(glLightiv, (GL_LIGHT0, GL_SPOT_CUTOFF, junk))
TEST_FUNC(glLightModelf, (GL_LIGHT_MODEL_AMBIENT, 0))
TEST_FUNC(glLightModelfv, (GL_LIGHT_MODEL_AMBIENT, junk))
TEST_FUNC(glLightModeli, (GL_LIGHT_MODEL_AMBIENT, 0))
TEST_FUNC(glLightModeliv, (GL_LIGHT_MODEL_AMBIENT, junk))
TEST_FUNC(glLineStipple, (0, 0))
TEST_FUNC(glLineWidth, (1))
TEST_FUNC(glListBase, (0))
TEST_FUNC(glLoadIdentity, ())
TEST_FUNC(glLoadMatrixd, (junk))
TEST_FUNC(glLoadMatrixf, (junk))
TEST_FUNC(glLogicOp, (GL_COPY))
TEST_FUNC(glMaterialf, (GL_FRONT, GL_AMBIENT, 0))
TEST_FUNC(glMaterialfv, (GL_FRONT, GL_AMBIENT, junk))
TEST_FUNC(glMateriali, (GL_FRONT, GL_AMBIENT, 0))
TEST_FUNC(glMaterialiv, (GL_FRONT, GL_AMBIENT, junk))
TEST_FUNC(glMatrixMode, (GL_MODELVIEW))
TEST_FUNC(glMultMatrixd, (junk))
TEST_FUNC(glMultMatrixf, (junk))
TEST_FUNC(glNewList, (newlist_dlist, GL_COMPILE))
TEST_FUNC(glNormal3d, (0, 0, 0))
TEST_FUNC(glNormal3f, (0, 0, 0))
TEST_FUNC(glNormal3i, (0, 0, 0))
TEST_FUNC(glNormal3s, (0, 0, 0))
TEST_FUNC(glNormal3dv, (junk))
TEST_FUNC(glNormal3fv, (junk))
TEST_FUNC(glNormal3iv, (junk))
TEST_FUNC(glNormal3sv, (junk))
TEST_FUNC(glNormalPointer, (GL_FLOAT, 0, junk))
TEST_FUNC(glOrtho, (0, 1, 0, 1, 0, 1))
TEST_FUNC(glReadPixels, (0, 0, 1, 1, GL_RGBA, GL_FLOAT, junk))
TEST_FUNC(glRotated, (0, 0, 0, 1))
TEST_FUNC(glRotatef, (0, 0, 0, 1))
TEST_FUNC(glScaled, (0, 0, 0))
TEST_FUNC(glScalef, (0, 0, 0))
TEST_FUNC(glShadeModel, (GL_SMOOTH))
TEST_FUNC(glTranslated, (0, 0, 0))
TEST_FUNC(glTranslatef, (0, 0, 0))
TEST_FUNC(glPixelMapfv, (GL_PIXEL_MAP_S_TO_S, 1, junk))
TEST_FUNC(glPixelMapuiv, (GL_PIXEL_MAP_S_TO_S, 1, junk))
TEST_FUNC(glPixelMapusv, (GL_PIXEL_MAP_S_TO_S, 1, junk))
TEST_FUNC(glPixelStoref, (GL_UNPACK_ROW_LENGTH, 0))
TEST_FUNC(glPixelStorei, (GL_UNPACK_ROW_LENGTH, 0))
TEST_FUNC(glPixelTransferf, (GL_MAP_COLOR, 0))
TEST_FUNC(glPixelTransferi, (GL_MAP_COLOR, 0))
TEST_FUNC(glPixelZoom, (0, 0))
TEST_FUNC(glPointSize, (1))
static void test_glPushAttrib(void) {
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	glPopAttrib();
}
static void test_glPushClientAttrib(void) {
	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
	glPopClientAttrib();
}
static void test_glPushMatrix(void) {
	glPushMatrix();
	glPopMatrix();
}
TEST_FUNC(glPolygonStipple, (junk))
TEST_FUNC(glRasterPos2d, (0, 0))
TEST_FUNC(glRasterPos2dv, (junk))
TEST_FUNC(glRasterPos2f, (0, 0))
TEST_FUNC(glRasterPos2fv, (junk))
TEST_FUNC(glRasterPos2i, (0, 0))
TEST_FUNC(glRasterPos2iv, (junk))
TEST_FUNC(glRasterPos2s, (0, 0))
TEST_FUNC(glRasterPos2sv, (junk))
TEST_FUNC(glRasterPos3d, (0, 0, 0))
TEST_FUNC(glRasterPos3dv, (junk))
TEST_FUNC(glRasterPos3f, (0, 0, 0))
TEST_FUNC(glRasterPos3fv, (junk))
TEST_FUNC(glRasterPos3i, (0, 0, 0))
TEST_FUNC(glRasterPos3iv, (junk))
TEST_FUNC(glRasterPos3s, (0, 0, 0))
TEST_FUNC(glRasterPos3sv, (junk))
TEST_FUNC(glReadBuffer, (fbo_attachment))
TEST_FUNC(glRectd, (0, 0, 0, 0))
TEST_FUNC(glRectdv, (junk, junk))
TEST_FUNC(glRectf, (0, 0, 0, 0))
TEST_FUNC(glRectfv, (junk, junk))
TEST_FUNC(glRecti, (0, 0, 0, 0))
TEST_FUNC(glRectiv, (junk, junk))
TEST_FUNC(glRects, (0, 0, 0, 0))
TEST_FUNC(glRectsv, (junk, junk))
TEST_FUNC(glRenderMode, (GL_RENDER))
TEST_FUNC(glScissor, (0, 0, 1, 1))
TEST_FUNC(glStencilFunc, (GL_ALWAYS, 0, 0))
TEST_FUNC(glStencilMask, (0))
TEST_FUNC(glStencilOp, (GL_REPLACE, GL_REPLACE, GL_REPLACE))
TEST_FUNC(glTexCoord1d, (0))
TEST_FUNC(glTexCoord1dv, (junk))
TEST_FUNC(glTexCoord1f, (0))
TEST_FUNC(glTexCoord1fv, (junk))
TEST_FUNC(glTexCoord1i, (0))
TEST_FUNC(glTexCoord1iv, (junk))
TEST_FUNC(glTexCoord1s, (0))
TEST_FUNC(glTexCoord1sv, (junk))
TEST_FUNC(glTexCoord2d, (0, 0))
TEST_FUNC(glTexCoord2dv, (junk))
TEST_FUNC(glTexCoord2f, (0, 0))
TEST_FUNC(glTexCoord2fv, (junk))
TEST_FUNC(glTexCoord2i, (0, 0))
TEST_FUNC(glTexCoord2iv, (junk))
TEST_FUNC(glTexCoord2s, (0, 0))
TEST_FUNC(glTexCoord2sv, (junk))
TEST_FUNC(glTexCoord3d, (0, 0, 0))
TEST_FUNC(glTexCoord3dv, (junk))
TEST_FUNC(glTexCoord3f, (0, 0, 0))
TEST_FUNC(glTexCoord3fv, (junk))
TEST_FUNC(glTexCoord3i, (0, 0, 0))
TEST_FUNC(glTexCoord3iv, (junk))
TEST_FUNC(glTexCoord3s, (0, 0, 0))
TEST_FUNC(glTexCoord3sv, (junk))
TEST_FUNC(glTexCoord4d, (0, 0, 0, 0))
TEST_FUNC(glTexCoord4dv, (junk))
TEST_FUNC(glTexCoord4f, (0, 0, 0, 0))
TEST_FUNC(glTexCoord4fv, (junk))
TEST_FUNC(glTexCoord4i, (0, 0, 0, 0))
TEST_FUNC(glTexCoord4iv, (junk))
TEST_FUNC(glTexCoord4s, (0, 0, 0, 0))
TEST_FUNC(glTexCoord4sv, (junk))
TEST_FUNC(glTexCoordPointer, (4, GL_FLOAT, 0, junk))
TEST_FUNC(glTexEnvf, (GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1))
TEST_FUNC(glTexEnvfv, (GL_TEXTURE_ENV, GL_ALPHA_SCALE, &onef))
TEST_FUNC(glTexEnvi, (GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1))
TEST_FUNC(glTexEnviv, (GL_TEXTURE_ENV, GL_ALPHA_SCALE, &onei))
TEST_FUNC(glTexGend, (GL_S, GL_OBJECT_PLANE, 0))
TEST_FUNC(glTexGendv, (GL_S, GL_OBJECT_PLANE, junk))
TEST_FUNC(glTexGenf, (GL_S, GL_OBJECT_PLANE, 0))
TEST_FUNC(glTexGenfv, (GL_S, GL_OBJECT_PLANE, junk))
TEST_FUNC(glTexGeni, (GL_S, GL_OBJECT_PLANE, 0))
TEST_FUNC(glTexGeniv, (GL_S, GL_OBJECT_PLANE, junk))
TEST_FUNC(glTexImage1D, (GL_TEXTURE_1D, 0, GL_RGBA, 1, 0, GL_RGBA, GL_FLOAT, NULL))
TEST_FUNC(glTexImage2D, (GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, NULL))
TEST_FUNC(glTexParameterf, (GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, 0))
TEST_FUNC(glTexParameterfv, (GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, junk))
TEST_FUNC(glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, 0))
TEST_FUNC(glTexParameteriv, (GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, junk))
TEST_FUNC(glVertex2d, (0, 0))
TEST_FUNC(glVertex2dv, (junk))
TEST_FUNC(glVertex2f, (0, 0))
TEST_FUNC(glVertex2fv, (junk))
TEST_FUNC(glVertex2i, (0, 0))
TEST_FUNC(glVertex2iv, (junk))
TEST_FUNC(glVertex2s, (0, 0))
TEST_FUNC(glVertex2sv, (junk))
TEST_FUNC(glVertex3d, (0, 0, 0))
TEST_FUNC(glVertex3dv, (junk))
TEST_FUNC(glVertex3f, (0, 0, 0))
TEST_FUNC(glVertex3fv, (junk))
TEST_FUNC(glVertex3i, (0, 0, 0))
TEST_FUNC(glVertex3iv, (junk))
TEST_FUNC(glVertex3s, (0, 0, 0))
TEST_FUNC(glVertex3sv, (junk))
TEST_FUNC(glVertex4d, (0, 0, 0, 0))
TEST_FUNC(glVertex4dv, (junk))
TEST_FUNC(glVertex4f, (0, 0, 0, 0))
TEST_FUNC(glVertex4fv, (junk))
TEST_FUNC(glVertex4i, (0, 0, 0, 0))
TEST_FUNC(glVertex4iv, (junk))
TEST_FUNC(glVertex4s, (0, 0, 0, 0))
TEST_FUNC(glVertex4sv, (junk))
TEST_FUNC(glVertexPointer, (4, GL_FLOAT, 0, junk))
TEST_FUNC(glViewport, (0, 0, 1, 1))

#define TEST(a) { #a, test_##a }

static struct test ok_tests[] = {
	TEST(glArrayElement),
	TEST(glCallList),
	TEST(glCallLists),
	TEST(glColor3b),
	TEST(glColor3bv),
	TEST(glColor3d),
	TEST(glColor3dv),
	TEST(glColor3f),
	TEST(glColor3fv),
	TEST(glColor3i),
	TEST(glColor3iv),
	TEST(glColor3s),
	TEST(glColor3sv),
	TEST(glColor3ub),
	TEST(glColor3ubv),
	TEST(glColor3ui),
	TEST(glColor3uiv),
	TEST(glColor3us),
	TEST(glColor3usv),
	TEST(glColor4b),
	TEST(glColor4bv),
	TEST(glColor4d),
	TEST(glColor4dv),
	TEST(glColor4f),
	TEST(glColor4fv),
	TEST(glColor4i),
	TEST(glColor4iv),
	TEST(glColor4s),
	TEST(glColor4sv),
	TEST(glColor4ub),
	TEST(glColor4ubv),
	TEST(glColor4ui),
	TEST(glColor4uiv),
	TEST(glColor4us),
	TEST(glColor4usv),
	TEST(glEdgeFlag),
	TEST(glEdgeFlagv),
	TEST(glIndexd),
	TEST(glIndexdv),
	TEST(glIndexf),
	TEST(glIndexfv),
	TEST(glIndexi),
	TEST(glIndexiv),
	TEST(glIndexs),
	TEST(glIndexsv),
	TEST(glIndexub),
	TEST(glIndexubv),
	TEST(glMaterialf),
	TEST(glMaterialfv),
	TEST(glMateriali),
	TEST(glMaterialiv),
	TEST(glNormal3d),
	TEST(glNormal3dv),
	TEST(glNormal3f),
	TEST(glNormal3fv),
	TEST(glNormal3i),
	TEST(glNormal3iv),
	TEST(glNormal3s),
	TEST(glNormal3sv),
	TEST(glTexCoord1d),
	TEST(glTexCoord1dv),
	TEST(glTexCoord1f),
	TEST(glTexCoord1fv),
	TEST(glTexCoord1i),
	TEST(glTexCoord1iv),
	TEST(glTexCoord1s),
	TEST(glTexCoord1sv),
	TEST(glTexCoord2d),
	TEST(glTexCoord2dv),
	TEST(glTexCoord2f),
	TEST(glTexCoord2fv),
	TEST(glTexCoord2i),
	TEST(glTexCoord2iv),
	TEST(glTexCoord2s),
	TEST(glTexCoord2sv),
	TEST(glTexCoord3d),
	TEST(glTexCoord3dv),
	TEST(glTexCoord3f),
	TEST(glTexCoord3fv),
	TEST(glTexCoord3i),
	TEST(glTexCoord3iv),
	TEST(glTexCoord3s),
	TEST(glTexCoord3sv),
	TEST(glTexCoord4d),
	TEST(glTexCoord4dv),
	TEST(glTexCoord4f),
	TEST(glTexCoord4fv),
	TEST(glTexCoord4i),
	TEST(glTexCoord4iv),
	TEST(glTexCoord4s),
	TEST(glTexCoord4sv),
	TEST(glVertex2d),
	TEST(glVertex2dv),
	TEST(glVertex2f),
	TEST(glVertex2fv),
	TEST(glVertex2i),
	TEST(glVertex2iv),
	TEST(glVertex2s),
	TEST(glVertex2sv),
	TEST(glVertex3d),
	TEST(glVertex3dv),
	TEST(glVertex3f),
	TEST(glVertex3fv),
	TEST(glVertex3i),
	TEST(glVertex3iv),
	TEST(glVertex3s),
	TEST(glVertex3sv),
	TEST(glVertex4d),
	TEST(glVertex4dv),
	TEST(glVertex4f),
	TEST(glVertex4fv),
	TEST(glVertex4i),
	TEST(glVertex4iv),
	TEST(glVertex4s),
	TEST(glVertex4sv),
};

static struct test error_tests[] = {
	TEST(glAlphaFunc),
	TEST(glBlendFunc),
	TEST(glBitmap),
	TEST(glClear),
	TEST(glClearAccum),
	TEST(glClearColor),
	TEST(glClearDepth),
	TEST(glClearIndex),
	TEST(glClearStencil),
	TEST(glClipPlane),
	TEST(glColorMask),
	TEST(glColorMaterial),
	TEST(glCopyPixels),
	TEST(glCullFace),
	TEST(glDepthFunc),
	TEST(glDepthMask),
	TEST(glDepthRange),
	TEST(glDisable),
	TEST(glDrawArrays),
	TEST(glDrawBuffer),
	TEST(glDrawElements),
	TEST(glDrawPixels),
	TEST(glEnable),
	TEST(glFrontFace),
	TEST(glFrustum),
	TEST(glHint),
	TEST(glIndexMask),
	TEST(glLightf),
	TEST(glLightfv),
	TEST(glLighti),
	TEST(glLightiv),
	TEST(glLightModelf),
	TEST(glLightModelfv),
	TEST(glLightModeli),
	TEST(glLightModeliv),
	TEST(glLineStipple),
	TEST(glLineWidth),
	TEST(glListBase),
	TEST(glLoadIdentity),
	TEST(glLoadMatrixd),
	TEST(glLoadMatrixf),
	TEST(glLogicOp),
	TEST(glMatrixMode),
	TEST(glMultMatrixd),
	TEST(glMultMatrixf),
	TEST(glOrtho),
	TEST(glRotated),
	TEST(glRotatef),
	TEST(glScaled),
	TEST(glScalef),
	TEST(glShadeModel),
	TEST(glTranslated),
	TEST(glTranslatef),
	TEST(glPixelMapfv),
	TEST(glPixelMapuiv),
	TEST(glPixelMapusv),
	TEST(glPixelTransferf),
	TEST(glPixelTransferi),
	TEST(glPixelZoom),
	TEST(glPointSize),
	TEST(glPushAttrib),
	TEST(glPushMatrix),
	TEST(glPolygonStipple),
	TEST(glRasterPos2d),
	TEST(glRasterPos2dv),
	TEST(glRasterPos2f),
	TEST(glRasterPos2fv),
	TEST(glRasterPos2i),
	TEST(glRasterPos2iv),
	TEST(glRasterPos2s),
	TEST(glRasterPos2sv),
	TEST(glRasterPos3d),
	TEST(glRasterPos3dv),
	TEST(glRasterPos3f),
	TEST(glRasterPos3fv),
	TEST(glRasterPos3i),
	TEST(glRasterPos3iv),
	TEST(glRasterPos3s),
	TEST(glRasterPos3sv),
	TEST(glReadBuffer),
	TEST(glRectd),
	TEST(glRectdv),
	TEST(glRectf),
	TEST(glRectfv),
	TEST(glRecti),
	TEST(glRectiv),
	TEST(glRects),
	TEST(glRectsv),
	TEST(glScissor),
	TEST(glStencilFunc),
	TEST(glStencilMask),
	TEST(glStencilOp),
	TEST(glTexEnvf),
	TEST(glTexEnvfv),
	TEST(glTexEnvi),
	TEST(glTexEnviv),
	TEST(glTexGend),
	TEST(glTexGendv),
	TEST(glTexGenf),
	TEST(glTexGenfv),
	TEST(glTexGeni),
	TEST(glTexGeniv),
	TEST(glTexImage1D),
	TEST(glTexImage2D),
	TEST(glTexParameterf),
	TEST(glTexParameterfv),
	TEST(glTexParameteri),
	TEST(glTexParameteriv),
	TEST(glViewport),
};

/* From the GL 1.2.1 specification, section 5.4 ("Display Lists"):
 *
 *     "Certain commands, when called while compiling a display list,
 *      are not compiled into the display list but are executed
 *      immediately. These are: IsList, GenLists, DeleteLists,
 *      FeedbackBuffer, SelectBuffer, RenderMode, VertexPointer,
 *      NormalPointer, ColorPointer, IndexPointer, TexCoordPointer,
 *      EdgeFlagPointer, InterleavedArrays, EnableClientState,
 *      DisableClientState, PushClientAttrib, PopClientAttrib,
 *      ReadPixels, PixelStore, GenTextures, DeleteTextures,
 *      AreTexturesResident, IsTexture, Flush, Finish, as well as
 *      IsEnabled and all of the Get commands (see Chapter 6)."
 *
 * The 1.0 spec didn't mention the Pointer or ClientAttrib functions,
 * but this appears to be a correction.
 */
static struct test nondlist_error_tests[] = {
	TEST(glColorPointer),
	TEST(glDeleteLists),
	TEST(glDisableClientState),
	TEST(glEdgeFlagPointer),
	TEST(glEnableClientState),
	TEST(glIndexPointer),
	TEST(glNewList),
	TEST(glNormalPointer),
	TEST(glGenLists),
	TEST(glGetBooleanv),
	TEST(glGetClipPlane),
	TEST(glGetDoublev),
	TEST(glGetError),
	TEST(glGetFloatv),
	TEST(glGetIntegerv),
	TEST(glGetLightfv),
	TEST(glGetLightiv),
	TEST(glGetMaterialfv),
	TEST(glGetMaterialiv),
	TEST(glGetPolygonStipple),
	TEST(glGetString),
	TEST(glGetPixelMapfv),
	TEST(glGetPixelMapuiv),
	TEST(glGetPixelMapusv),
	TEST(glGetPointerv),
	TEST(glGetTexEnvfv),
	TEST(glGetTexEnviv),
	TEST(glGetTexGendv),
	TEST(glGetTexGenfv),
	TEST(glGetTexGeniv),
	TEST(glGetTexImage),
	TEST(glGetTexLevelParameterfv),
	TEST(glGetTexLevelParameteriv),
	TEST(glGetTexParameterfv),
	TEST(glGetTexParameteriv),
	TEST(glFinish),
	TEST(glFlush),
	TEST(glInterleavedArrays),
	TEST(glIsEnabled),
	TEST(glIsList),
	TEST(glPixelStoref),
	TEST(glPixelStorei),
	TEST(glPushClientAttrib),
	TEST(glReadPixels),
	TEST(glRenderMode),
	TEST(glTexCoordPointer),
	TEST(glVertexPointer),
};

static struct test error_only_tests[] = {
	/* No accum buffer is bound */
	TEST(glAccum),
	/* If it doesn't error out, it would need state cleanup. */
	TEST(glBegin),
};

static struct test endlist_test = TEST(glEndList);

enum piglit_result
piglit_display()
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

static bool
test_beginend(struct test *test, GLenum expected_error)
{
	printf("  Testing within begin/end\n");

	glBegin(GL_POINTS);
	test->func();
	glEnd();
	if (!piglit_check_gl_error(expected_error)) {
		fprintf(stderr, "after glEnd()\n");
		return false;
	}

	return true;
}

static bool
test_dlist_exec(struct test *test, GLenum expected_error)
{
	GLuint dlist;

	printf("  Testing glCallList() inside begin/end\n");

	dlist = glGenLists(1);
	glNewList(dlist, GL_COMPILE);
	test->func();
	glEndList();
	/* Nothing was executed yet, so no error should be present. */
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		fprintf(stderr, "after glEndList()\n");
		return false;
	}

	glBegin(GL_POINTS);
	glCallList(dlist);
	glEnd();
	if (!piglit_check_gl_error(expected_error)) {
		fprintf(stderr, "after glCallList()\n");
		return false;
	}

	glDeleteLists(dlist, 1);

	return true;
}

static bool
test_dlist_compile(struct test *test, GLenum expected_error)
{
	GLuint dlist;

	printf("  Testing glNewList(GL_COMPILE) with begin/end inside\n");

	dlist = glGenLists(1);
	glNewList(dlist, GL_COMPILE);
	glBegin(GL_POINTS);
	test->func();
	glEnd();
	glEndList();
	/* Nothing was executed yet, so no error should be present. */
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		fprintf(stderr, "after glEndList()\n");
		return false;
	}

	glCallList(dlist);
	if (!piglit_check_gl_error(expected_error)) {
		fprintf(stderr, "after glCallList()\n");
		return false;
	}

	glDeleteLists(dlist, 1);

	return true;
}

static bool
test_dlist_compile_exec(struct test *test, GLenum expected_error)
{
	GLuint dlist;

	printf("  Testing glNewList(GL_COMPILE_AND_EXEC) with begin/end "
	       "inside\n");

	dlist = glGenLists(1);
	glNewList(dlist, GL_COMPILE_AND_EXECUTE);
	glBegin(GL_POINTS);
	test->func();
	glEnd();
	glEndList();
	if (!piglit_check_gl_error(expected_error)) {
		fprintf(stderr, "after glEndList()\n");
		return false;
	}

	glCallList(dlist);
	if (!piglit_check_gl_error(expected_error)) {
		fprintf(stderr, "after glCallList()\n");
		return false;
	}

	glDeleteLists(dlist, 1);

	return true;
}

static bool
test_dlist_compile_exec_after(struct test *test)
{
	GLuint dlist;

	printf("  Testing glNewList(GL_COMPILE_AND_EXEC) with Begin/End "
	       "inside and the command after End\n");

	dlist = glGenLists(1);
	glNewList(dlist, GL_COMPILE_AND_EXECUTE);
	glBegin(GL_POINTS);
	glEnd();
	test->func();
	glEndList();
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		fprintf(stderr, "after glEndList()\n");
		return false;
	}

	glCallList(dlist);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		fprintf(stderr, "after glCallList()\n");
		return false;
	}

	glDeleteLists(dlist, 1);

	return true;
}

static bool
run_tests(struct test *tests, int num_tests, GLenum expected_error)
{
	int i;
	bool pass = true;

	for (i = 0; i < num_tests; i++) {
		bool test_pass = true;

		printf("testing %s()\n", tests[i].name);
		test_pass = test_pass &&
			test_beginend(&tests[i], expected_error);

		if (tests != nondlist_error_tests) {
			test_pass = test_pass &&
				test_dlist_compile(&tests[i], expected_error);
			test_pass = test_pass &&
				test_dlist_exec(&tests[i], expected_error);
			test_pass = test_pass &&
				test_dlist_compile_exec(&tests[i],
							expected_error);

			/* Sanity check that we can actually do our call in a
			 * display list without triggering an error (unless
			 * it's impossible)
			 */
			if (tests != error_only_tests) {
				test_pass = test_pass &&
					test_dlist_compile_exec_after(&tests[i]);
			}
		}

		piglit_report_subtest_result(test_pass ?
					     PIGLIT_PASS : PIGLIT_FAIL,
					     "%s", tests[i].name);

		pass = test_pass && pass;
	}

	return pass;
}

/**
 * Special-case testing for glEndList().
 *
 * If we call glEndList without a glNewList active, we will get
 * GL_INVALID_OPERATION anyway.  If we call glNewList in our begin/end
 * block, then we have a GL_INVALID_OPERATION from that.  So, to test
 * glEndList() throwing GL_INVALID_OPERATION inside a begin/end
 * specifically because it's inside a begin/end, we need the begin/end 
 */
bool
test_endlist()
{
	GLuint dlist;

	if (!test_beginend(&endlist_test, GL_INVALID_OPERATION))
		return false;

	/* We can't use test_dlist_compile_exec because that expects
	 * the command to be compiled.  We also need to distinguish
	 * the EndList inside the Begin/End failing like it's supposed
	 * to, from the one that's supposed to terminate the list
	 * failing in case the one inside Begin/End happened to
	 * execute.
	 */
	printf("  Testing glNewList(GL_COMPILE_AND_EXEC) with begin/end "
	       "inside\n");

	dlist = glGenLists(1);
	glNewList(dlist, GL_COMPILE_AND_EXECUTE);
	glBegin(GL_POINTS);
	glEndList();
	glEnd();
	if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
		fprintf(stderr, "after glEnd()\n");
		return false;
	}
	glEndList();

	return true;
}

void
piglit_init(int argc, char **argv)
{
	bool pass;

	/* Set up some state to be used by our various test
	 * functions
	 */
	newlist_dlist = glGenLists(1);
	deletelists_dlist = glGenLists(1);

	some_dlist = glGenLists(1);
	glNewList(some_dlist, GL_COMPILE);
	glEndList();

	if (piglit_is_extension_supported("GL_ARB_framebuffer_object"))
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo_binding);
	else
		fbo_binding = 0;

	if (fbo_binding)
		fbo_attachment = GL_COLOR_ATTACHMENT0;
	else
		fbo_attachment = GL_FRONT;

	pass = run_tests(ok_tests, ARRAY_SIZE(ok_tests), GL_NO_ERROR);
	pass = run_tests(error_tests, ARRAY_SIZE(error_tests),
			 GL_INVALID_OPERATION) && pass;
	pass = run_tests(error_only_tests, ARRAY_SIZE(error_only_tests),
			 GL_INVALID_OPERATION) && pass;

	if (test_endlist()) {
		piglit_report_subtest_result(PIGLIT_PASS, "glEndList");
	} else {
		piglit_report_subtest_result(PIGLIT_FAIL, "glEndList");
		pass = false;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
