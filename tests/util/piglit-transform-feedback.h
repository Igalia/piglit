/*
 * Copyright © 2011 Intel Corporation
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

#pragma once

/**
 * \name Portable wrappers for transform feedback functions
 *
 * \note
 * \c piglit_require_transform_feedback must be called before using
 * these wrappers.
 */
/*@{*/
#if defined(USE_OPENGL_ES1)
#define piglit_BeginTransformFeedback assert(!"glBeginTransformFeedback does not exist in ES1")
#define piglit_BindBufferBase assert(!"glBindBufferBase does not exist in ES1")
#define piglit_BindBufferRange assert(!"glBindBufferRange does not exist in ES1")
#define piglit_EndTransformFeedback assert(!"glEndTransformFeedback does not exist in ES1")
#define piglit_GetBooleani_v assert(!"glGetBooleani_v does not exist in ES1")
#define piglit_GetIntegeri_v assert(!"glGetIntegeri_v does not exist in ES1")
#define piglit_GetTransformFeedbackVarying assert(!"glGetTransformFeedbackVarying does not exist in ES1")
#define piglit_TransformFeedbackVaryings assert(!"glTransformFeedbackVaryings does not exist in ES1")
#elif defined(USE_OPENGL_ES2)
#define piglit_BeginTransformFeedback assert(!"glBeginTransformFeedback does not exist in ES2")
#define piglit_BindBufferBase assert(!"glBindBufferBase does not exist in ES2")
#define piglit_BindBufferRange assert(!"glBindBufferRange does not exist in ES2")
#define piglit_EndTransformFeedback assert(!"glEndTransformFeedback does not exist in ES2")
#define piglit_GetBooleani_v assert(!"glGetBooleani_v does not exist in ES2")
#define piglit_GetIntegeri_v assert(!"glGetIntegeri_v does not exist in ES2")
#define piglit_GetTransformFeedbackVarying assert(!"glGetTransformFeedbackVarying does not exist in ES2")
#define piglit_TransformFeedbackVaryings assert(!"glTransformFeedbackVaryings does not exist in ES2")
#else
extern PFNGLBEGINTRANSFORMFEEDBACKPROC piglit_BeginTransformFeedback;
extern PFNGLBINDBUFFERBASEPROC piglit_BindBufferBase;
extern PFNGLBINDBUFFERRANGEPROC piglit_BindBufferRange;
extern PFNGLENDTRANSFORMFEEDBACKPROC piglit_EndTransformFeedback;
extern PFNGLGETBOOLEANI_VPROC piglit_GetBooleani_v;
extern PFNGLGETINTEGERI_VPROC piglit_GetIntegeri_v;
extern PFNGLGETTRANSFORMFEEDBACKVARYINGPROC piglit_GetTransformFeedbackVarying;
extern PFNGLTRANSFORMFEEDBACKVARYINGSPROC piglit_TransformFeedbackVaryings;
#endif
/*@}*/

/**
 * Require transform feedback.
 *
 * Transform feedback may either be provided by GL 3.0 or
 * EXT_transform_feedback.
 */
extern void piglit_require_transform_feedback(void);
