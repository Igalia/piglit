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

#ifndef USE_OPENGL
#	error USE_OPENGL is undefined
#endif

#if defined(_MSC_VER)
#include <windows.h>
#endif

#include "piglit-util-gl-common.h"

PFNGLBEGINTRANSFORMFEEDBACKPROC piglit_BeginTransformFeedback = NULL;
PFNGLBINDBUFFERBASEPROC piglit_BindBufferBase = NULL;
PFNGLBINDBUFFERRANGEPROC piglit_BindBufferRange = NULL;
PFNGLENDTRANSFORMFEEDBACKPROC piglit_EndTransformFeedback = NULL;
PFNGLGETBOOLEANI_VPROC piglit_GetBooleani_v = NULL;
PFNGLGETINTEGERI_VPROC piglit_GetIntegeri_v = NULL;
PFNGLGETTRANSFORMFEEDBACKVARYINGPROC piglit_GetTransformFeedbackVarying = NULL;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC piglit_TransformFeedbackVaryings = NULL;

static void
init_functions_from_core(void)
{
	piglit_BeginTransformFeedback = glBeginTransformFeedback;
	piglit_BindBufferBase = glBindBufferBase;
	piglit_BindBufferRange = glBindBufferRange;
	piglit_EndTransformFeedback = glEndTransformFeedback;
	piglit_GetBooleani_v = glGetBooleani_v;
	piglit_GetIntegeri_v = glGetIntegeri_v;
	piglit_GetTransformFeedbackVarying = glGetTransformFeedbackVarying;
	piglit_TransformFeedbackVaryings = glTransformFeedbackVaryings;
}

static void
init_functions_from_ext(void)
{
	piglit_BeginTransformFeedback = glBeginTransformFeedbackEXT;
	piglit_BindBufferBase = glBindBufferBaseEXT;
	piglit_BindBufferRange = glBindBufferRangeEXT;
	piglit_EndTransformFeedback = glEndTransformFeedbackEXT;
	piglit_GetBooleani_v = glGetBooleanIndexedvEXT;
	piglit_GetIntegeri_v = glGetIntegerIndexedvEXT;
	piglit_GetTransformFeedbackVarying = glGetTransformFeedbackVaryingEXT;
	piglit_TransformFeedbackVaryings = glTransformFeedbackVaryingsEXT;
}

void
piglit_require_transform_feedback(void)
{
	if (piglit_get_gl_version() >= 30) {
		init_functions_from_core();
	} else if (piglit_is_extension_supported("GL_EXT_transform_feedback")) {
		init_functions_from_ext();
	} else {
		printf("Transform feedback not supported.\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}
}
