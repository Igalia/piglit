/*
 * Copyright © 2016 Intel Corporation
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


/** @file depthcoverage.c
 *
 * Verifies that post_depth_coverage & inner_coverage layout
 * qualifiers are mutually exclusives.
 */

#include "piglit-util-gl.h"
#include "piglit-shader.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

#if defined(PIGLIT_USE_OPENGL)
	config.supports_gl_core_version = 42;
#elif defined(PIGLIT_USE_OPENGL_ES3)
	config.supports_gl_es_version = 31;
#endif

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_INTEL_conservative_rasterization");
	piglit_require_extension("GL_ARB_post_depth_coverage");

	GLuint invalid_prog = piglit_compile_shader_text_nothrow(
		GL_FRAGMENT_SHADER,
#if defined(PIGLIT_USE_OPENGL)
		"#version 430\n"
#elif defined(PIGLIT_USE_OPENGL_ES3)
		"#version 310 es\n"
#endif
		"#extension GL_ARB_post_depth_coverage: enable\n"
		"#extension GL_INTEL_conservative_rasterization: enable\n"
		"layout(inner_coverage) in;\n"
		"layout(post_depth_coverage) in;\n"
		"out vec4 color;\n"
		"void main()\n"
		"{\n"
		"  color = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}\n");
	if (invalid_prog)
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(PIGLIT_PASS);
}
