/*
 * Copyright © 2010 Intel Corporation
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

/**
 * \file arb_es2_compatibility-getshaderprecisionformat.c
 * Validate data returned by glGetShaderPrecisionFormat
 *
 * Tests all of the shader targets and all of the precision modes.
 *
 * \warning
 * This test will need to be modified for OpenGL ES 2.0.  The mode
 * \c GL_HIGH_FLOAT is only available with \c GL_FRAGMENT_SHADER
 * if \c GL_OES_fragment_precision_high is supported.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 10;
	config.window_height = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
#ifdef GL_ARB_ES2_compatibility
	GLboolean pass = GL_TRUE;
	GLint status;
	unsigned i;
	unsigned j;
	static const GLenum shaderTypes[] = {
		GL_VERTEX_SHADER,
		GL_FRAGMENT_SHADER
	};
	static const struct {
		GLenum type;
		GLint range[2];
		GLint precision;
	} precision[] = {
		{ GL_LOW_FLOAT,    {  1,  1 },  8 },
		{ GL_MEDIUM_FLOAT, { 14, 14 }, 10 },
		{ GL_HIGH_FLOAT,   { 62, 62 }, 16 },
		{ GL_LOW_INT,      {  8,  8 },  0 },
		{ GL_MEDIUM_INT,   { 10, 10 },  0 },
		{ GL_HIGH_INT,     { 16, 16 },  0 }
	};

	piglit_require_gl_version(20);

	if (!piglit_is_extension_supported("GL_ARB_ES2_compatibility")) {
		printf("Requires ARB_ES2_compatibility\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	for (i = 0; i < ARRAY_SIZE(shaderTypes); i++) {
		for (j = 0; j < ARRAY_SIZE(precision); j++) {
			GLint r[2];
			GLint p;

			r[0] = 0;
			r[1] = 0;
			p = 0;

			glGetShaderPrecisionFormat(GL_VERTEX_SHADER,
						   precision[j].type,
						   r, & p);
			status = glGetError();
			if (status != GL_NO_ERROR) {
				printf("glGetShaderPrecisionFormat(0x%04x, "
				       "0x%04x) "
				       "got GL error of 0x%04x\n",
				       shaderTypes[i],
				       precision[j].type,
				       status);
				pass = GL_FALSE;
			}

			if (r[0] < precision[j].range[0]
			    || r[1] < precision[j].range[1]
			    || p < precision[j].precision) {
				printf("glGetShaderPrecisionFormat(0x%04x, "
				       "0x%04x) "
				       "returned invalid values:\n"
				       "    range = { %d, %d }\n"
				       "    precision = %d\n"
				       "expected at least:\n"
				       "    range = { %d, %d }\n"
				       "    precision = %d\n",
				       shaderTypes[i],
				       precision[j].type,
				       r[0], r[1],
				       p,
				       precision[j].range[0],
				       precision[j].range[1],
				       precision[j].precision);
				pass = GL_FALSE;
			}
		}
	}
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
#else
	piglit_report_result(PIGLIT_SKIP);
#endif /* GL_ARB_ES2_compatibility */
}
