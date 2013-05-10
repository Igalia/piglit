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
 * \brief Workarounds for building with GLES2 and GLES3.
 *
 * When building shader_runner against GLESX and libpiglitutil_glesX, there
 * are many macros and symbols that are not defined. This header defines such
 * macros to have the same value found in <GL/gl*.h>, and defines such
 * functions to print an error message and then report PIGLIT_SKIP, just as
 * piglit-dispatch does for unsupported extension functions.
 */

#include <stdio.h>
#include "piglit-util.h"

static void
#if defined(__GNUC__)
__attribute__((unused))
#endif
unsupported_function(const char *name, ...)
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
#define UNSUPPORTED_FUNCTION(name, return_value, ...) \
	({ \
		unsupported_function(#name, __VA_ARGS__); \
		return_value; \
	 })

#if defined(PIGLIT_USE_OPENGL_ES3) || defined(PIGLIT_USE_OPENGL_ES2)

#define piglit_frustum_projection(...) UNSUPPORTED_FUNCTION(piglit_frustum_projection, 0, __VA_ARGS__)
#define piglit_gen_ortho_projection(...) UNSUPPORTED_FUNCTION(piglit_gen_ortho_projection, 0, __VA_ARGS__)
#define piglit_miptree_texture() UNSUPPORTED_FUNCTION(piglit_miptree_texture, 0, 0)
#define piglit_depth_texture(...) UNSUPPORTED_FUNCTION(piglit_depth_texture, 0, __VA_ARGS__)
#define piglit_ortho_projection(...) UNSUPPORTED_FUNCTION(piglit_ortho_projection, 0, __VA_ARGS__)
#define piglit_compile_program(...) UNSUPPORTED_FUNCTION(piglit_compile_program, 0, __VA_ARGS__)

#if defined(PIGLIT_USE_OPENGL_ES3)
#undef glMapBuffer

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
#endif /* PIGLIT_USE_OPENGL_ES3 */

#endif /* PIGLIT_USE_OPENGL_ES3 || PIGLIT_USE_OPENGL_ES2 */
