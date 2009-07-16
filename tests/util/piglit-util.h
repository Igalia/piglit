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

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

enum piglit_result {
	PIGLIT_SUCCESS,
	PIGLIT_FAILURE,
	PIGLIT_SKIP
};

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
void piglit_escape_exit_key(unsigned char key, int x, int y);

char *piglit_load_text_file(const char *file_name, unsigned *size);

extern GLfloat cube_face_texcoords[6][4][3];
extern const char *cube_face_names[6];
extern const GLenum cube_face_targets[6];
