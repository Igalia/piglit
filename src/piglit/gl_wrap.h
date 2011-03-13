/*
 * Copyright (c) 2010 Intel Corporation
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
 *
 * Authors:
 *     Chad Versace <chad.versace@intel.com>
 */

/**
 * \file gl_wrap.h
 * \brief Convenience header that includes the actual OpenGL headers.
 *
 * The actual OpenGL headers are chosen according to the macro definitions
 * USE_OPENGL and USE_OPENGL_ES2.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#include <windows.h>
#endif

#ifdef USE_OPENGL
#	include "glew.h"
	/* Include the real headers too, in case GLEW misses something. */
#	ifdef __APPLE__
#		include <OpenGL/gl.h>
#		include <OpenGL/glu.h>
#		include <OpenGL/glext.h>
#	else
#		include <GL/gl.h>
#		include <GL/glu.h>
#		include <GL/glext.h>
#	endif
#endif

#ifdef USE_OPENGL_ES2
#	include <GLES2/gl2.h>
#	include <GLES2/gl2ext.h>
#endif

#ifdef __cplusplus
} /* end extern "C" */
#endif
