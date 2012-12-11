/*
 * Copyright © 2012 Intel Corporation
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
 * \file
 * \brief Workarounds for building with GLES.
 *
 * When building shader_runner against GLES3 and libpiglitutil_gles3, there
 * are many macros and symbols that are not defined. This header defines such
 * macros to have the same value found in <GL/gl*.h>, and defines such
 * functions to print an error message and then report PIGLIT_SKIP, just as
 * piglit-dispatch does for unsupported extension functions.
 */

#include <stdio.h>
#include "piglit-util.h"

#define GL_CLIP_PLANE0 0x3000
#define GL_CLIP_PLANE1 0x3001
#define GL_CLIP_PLANE2 0x3002
#define GL_CLIP_PLANE3 0x3003
#define GL_CLIP_PLANE4 0x3004
#define GL_CLIP_PLANE5 0x3005
#define GL_COMPARE_R_TO_TEXTURE 0x884E
#define GL_DEPTH_TEXTURE_MODE 0x884B
#define GL_FLAT 0x1D00
#define GL_FRAGMENT_PROGRAM_ARB 0x8804
#define GL_GEOMETRY_SHADER 0x8DD9
#define GL_INTENSITY 0x8049
#define GL_MAX_CLIP_PLANES 0x0D32
#define GL_POLYGON 0x0009
#define GL_POLYGON_OFFSET_EXT 0x8037
#define GL_QUADS 0x0007
#define GL_QUAD_STRIP 0x0008
#define GL_SMOOTH 0x1D01
#define GL_TEXTURE_1D 0x0DE0
#define GL_TEXTURE_1D_ARRAY 0x8C18
#define GL_TEXTURE_1D_ARRAY_EXT 0x8C18
#define GL_TEXTURE_CUBE_MAP_ARRAY 0x9009
#define GL_TEXTURE_RECTANGLE 0x84F5
#define GL_VERTEX_ARRAY 0x8074
#define GL_VERTEX_PROGRAM_ARB 0x8620
#define GL_VERTEX_PROGRAM_ARB 0x8620
#define GL_VERTEX_PROGRAM_TWO_SIDE 0x8643
#define GL_WRITE_ONLY 0x88B9

static void
#if defined(__GNUC__)
__attribute__((unused))
#endif
unsupported_function(const char *name)
{
	printf("Function \"%s\" not supported on this implementation\n", name);
	piglit_report_result(PIGLIT_SKIP);
}

/**
 * This macro should be sufficient for most functions. If one of the actual
 * function's parameters causes an unused-variable warning, you must
 * special-case the function. See glBindProgramARB for example.
 *
 * GLES doesn't exist on Windows. So we're free to use the GCC/Clang extension
 * for statement expressions.
 */
#define UNSUPPORTED_FUNCTION(name, return_value) \
	({ \
	 	unsupported_function(#name); \
	 	return_value; \
	 })

#if defined(PIGLIT_USE_OPENGL_ES3)

#define piglit_frustum_projection(...) UNSUPPORTED_FUNCTION(piglit_frustum_projection, 0)
#define piglit_gen_ortho_projection(...) UNSUPPORTED_FUNCTION(piglit_gen_ortho_projection, 0)
#define piglit_miptree_texture(...) UNSUPPORTED_FUNCTION(piglit_miptree_texture, 0)
#define piglit_depth_texture(...) UNSUPPORTED_FUNCTION(piglit_depth_texture, 0)
#define piglit_ortho_projection(...) UNSUPPORTED_FUNCTION(piglit_ortho_projection, 0)
#define piglit_compile_program(...) UNSUPPORTED_FUNCTION(piglit_compile_program, 0)

#define glClipPlane(...) 				UNSUPPORTED_FUNCTION(glClipPlane, 0)
#define glDisableClientState(...) 			UNSUPPORTED_FUNCTION(glDisableClientState, 0)
#define glEnableClientState(...) 			UNSUPPORTED_FUNCTION(glEnableClientState, 0)
#define glProgramEnvParameter4fvARB(...) 		UNSUPPORTED_FUNCTION(glProgramEnvParameter4fvARB, 0)
#define glProgramLocalParameter4fvARB(...) 		UNSUPPORTED_FUNCTION(glProgramLocalParameter4fvARB, 0)
#define glShadeModel(...) 				UNSUPPORTED_FUNCTION(glShadeModel, 0)

#define glBindProgramARB(a, b) \
	/* Custom definition to suppress unused-variable warnings. */ \
	({ \
	 	(void) a; \
	 	(void) b; \
		unsupported_function("glBindProgramARB"); \
	 })

#define glVertexPointer(a, b, c, d) \
	/* Custom definition to suppress unused-variable warnings. */ \
	({ \
	 	(void) a; \
	 	(void) b; \
	 	(void) c; \
	 	(void) d; \
		unsupported_function("glVertexPointer"); \
	 })

static GLvoid*
glMapBuffer(GLenum target, GLbitfield access)
{
	/* Emulate with glMapBufferRange. */

	GLsizeiptr length = 0;

	glGetBufferParameteri64v(target, GL_BUFFER_SIZE, (GLint64*) &length);
	if (piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return glMapBufferRange(target, 0, length, access);
}

#endif /*PIGLIT_USE_OPENGL*/
