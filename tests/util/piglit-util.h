/*
 * Copyright (c) The Piglit project 2007
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#if defined(FREEGLUT)
#include <GL/freeglut_ext.h>
#endif
#endif

#if defined(_MSC_VER)
#include <GL/glext.h>
#endif

enum piglit_result {
	PIGLIT_SUCCESS,
	PIGLIT_FAILURE,
	PIGLIT_SKIP
};

#include "piglit-framework.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

int FindLine(const char *program, int position);
void piglit_report_result(enum piglit_result result);
void piglit_require_extension(const char *name);
int piglit_probe_pixel_rgb(int x, int y, const float* expected);
int piglit_probe_pixel_rgba(int x, int y, const float* expected);

#if defined(__APPLE__)
extern void (*pglGenProgramsARB)(GLsizei n, GLuint *programs);
extern void (*pglProgramStringARB)(GLenum target, GLenum format, GLsizei len, const GLvoid *string);
extern void (*pglBindProgramARB)(GLenum target, GLuint program);
extern GLboolean (*pglIsProgramARB)(GLuint program);
extern void (*pglDeleteProgramsARB)(GLsizei n, const GLuint *programs);
extern void (*pglProgramLocalParameter4fvARB)(GLenum target, GLuint index, const GLfloat *params);
extern void (*pglProgramLocalParameter4dARB)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void (*pglGetProgramivARB)(GLenum target, GLenum pname, GLint *params);
extern void (*pglGetProgramLocalParameterdvARB)(GLenum target, GLuint index, GLdouble *params);
#else
extern PFNGLGENPROGRAMSARBPROC pglGenProgramsARB;
extern PFNGLPROGRAMSTRINGARBPROC pglProgramStringARB;
extern PFNGLBINDPROGRAMARBPROC pglBindProgramARB;
extern PFNGLISPROGRAMARBPROC pglIsProgramARB;
extern PFNGLDELETEPROGRAMSARBPROC pglDeleteProgramsARB;
extern PFNGLPROGRAMLOCALPARAMETER4FVARBPROC pglProgramLocalParameter4fvARB;
extern PFNGLPROGRAMLOCALPARAMETER4DARBPROC pglProgramLocalParameter4dARB;
extern PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC pglGetProgramLocalParameterdvARB;
extern PFNGLGETPROGRAMIVARBPROC pglGetProgramivARB;
#endif

int piglit_use_fragment_program(void);
int piglit_use_vertex_program(void);
void piglit_require_fragment_program(void);
void piglit_require_vertex_program(void);
GLuint piglit_compile_program(GLenum target, const char* text);
GLint piglit_compile_shader(GLenum target, char *filename);
GLint piglit_link_simple_program(GLint vs, GLint fs);
GLvoid piglit_draw_rect(float x, float y, float w, float h);
GLvoid piglit_draw_rect_tex(float x, float y, float w, float h,
                            float tx, float ty, float tw, float th);
void piglit_escape_exit_key(unsigned char key, int x, int y);

char *piglit_load_text_file(const char *file_name, unsigned *size);

void piglit_ortho_projection(int w, int h, GLboolean push);

extern GLfloat cube_face_texcoords[6][4][3];
extern const char *cube_face_names[6];
extern const GLenum cube_face_targets[6];

/**
 * Common vertex program code to perform a model-view-project matrix transform
 */
#define PIGLIT_VERTEX_PROGRAM_MVP_TRANSFORM		\
	"ATTRIB	iPos = vertex.position;\n"		\
	"OUTPUT	oPos = result.position;\n"		\
	"PARAM	mvp[4] = { state.matrix.mvp };\n"	\
	"DP4	oPos.x, mvp[0], iPos;\n"		\
	"DP4	oPos.y, mvp[1], iPos;\n"		\
	"DP4	oPos.z, mvp[2], iPos;\n"		\
	"DP4	oPos.w, mvp[3], iPos;\n"

/**
 * Handle to a generic fragment program that passes the input color to output
 *
 * \note
 * Either \c piglit_use_fragment_program or \c piglit_require_fragment_program
 * must be called before using this program handle.
 */
extern GLint piglit_ARBfp_pass_through;
