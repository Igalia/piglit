/* Copyright 2012 Intel Corporation
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

/** \file piglit-dispatch.h
 *
 * Dispatch mechanism providing Piglit with access to:
 *
 * - Enum values defined by OpenGL, GLES, and extensions.  The enums
 *   are declared as simple #defines.
 *
 * - Functions defined by OpenGL, GLES, and extensions.  Each function
 *   is represented by a function pointer, which initially points to a
 *   stub function.  When the stub function is called, it looks up the
 *   appropriate function in the GL or GLES implementation, and
 *   updates the function pointer to point to it.  Then it defers to
 *   that function.
 *
 * The dispatch mechanism understands function aliases.  So, for
 * example, since glMapBuffer and glMapBufferARB are synonymous, you
 * may safely call either function.  The dispatch mechanism will
 * transparently map to whichever function the implementation
 * supports.
 *
 * You may also translate a function name to a function pointer at run
 * time by calling piglit_dispatch_resolve_function().
 *
 * The dispatch mechanism must be initialized before its first use.
 * The initialization function, piglit_dispatch_init(), allows the
 * caller to specify which API is in use, how to look up function
 * pointers, what to do in the event of an error, and what to do if an
 * unsupported function is requested.
 */

#ifndef __piglit_dispatch_h__
#define __piglit_dispatch_h__

#include <stdint.h>
#include <stddef.h>

#ifndef _WIN32

/* APIENTRY and GLAPIENTRY are not used on Linux or Mac. */
#define APIENTRY
#define GLAPIENTRY

#else /* _WIN32 */

#include <windows.h> // APIENTRY

#ifndef GLAPIENTRY
#define GLAPIENTRY APIENTRY
#endif

#ifndef GLAPI
#define GLAPI extern
#endif

#endif /* _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int GLenum;
typedef unsigned int GLbitfield;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef int GLfixed;
typedef unsigned char GLboolean;
typedef signed char GLbyte;
typedef short GLshort;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned long GLulong;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;
typedef int64_t GLint64EXT;
typedef uint64_t GLuint64EXT;
typedef GLint64EXT  GLint64;
typedef GLuint64EXT GLuint64;
typedef struct __GLsync *GLsync;

typedef char GLchar;

typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;

typedef ptrdiff_t GLintptrARB;
typedef ptrdiff_t GLsizeiptrARB;

typedef char GLcharARB;
typedef unsigned int GLhandleARB;

struct _cl_context;
struct _cl_event;

typedef GLintptr GLvdpauSurfaceNV;
typedef unsigned short GLhalfNV;

typedef void *GLeglImageOES;

typedef void (APIENTRY *GLDEBUGPROC)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,GLvoid *userParam);

typedef void (APIENTRY *GLDEBUGPROCARB)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,GLvoid *userParam);

typedef void (APIENTRY *GLDEBUGPROCAMD)(GLuint id,GLenum category,GLenum severity,GLsizei length,const GLchar *message,GLvoid *userParam);

typedef void (APIENTRY *GLDEBUGPROCKHR)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,GLvoid *userParam);

typedef void (APIENTRY *piglit_dispatch_function_ptr)(void);

typedef piglit_dispatch_function_ptr (*piglit_get_core_proc_address_function_ptr)(const char *, int);

typedef piglit_dispatch_function_ptr (*piglit_get_ext_proc_address_function_ptr)(const char *);

typedef piglit_dispatch_function_ptr (*piglit_dispatch_resolver_ptr)();

typedef void (*piglit_error_function_ptr)(const char *);

typedef enum {
	PIGLIT_DISPATCH_GL,
	PIGLIT_DISPATCH_ES1,
	PIGLIT_DISPATCH_ES2
} piglit_dispatch_api;

void piglit_dispatch_init(piglit_dispatch_api api,
			  piglit_get_core_proc_address_function_ptr get_core_proc,
			  piglit_get_ext_proc_address_function_ptr get_ext_proc,
			  piglit_error_function_ptr unsupported_proc,
			  piglit_error_function_ptr failure_proc);

piglit_dispatch_function_ptr
piglit_dispatch_resolve_function(const char *name);

#include "piglit-dispatch-gen.h"

void piglit_dispatch_default_init(piglit_dispatch_api api);

/* Prevent gl.h from being included, since it will attempt to define
 * the functions we've already defined.
 */
#define __gl_h_
#define __gltypes_h_
#define __glext_h_

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* __piglit_dispatch_h__ */
