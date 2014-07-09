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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file createshaderprogram-attached-shaders.c
 * Call glCreateShaderProgramEXT, verify that there are 0 attached shaders
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_text[] =
	"void main() { gl_Position = gl_Vertex; }";


enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLboolean pass = GL_TRUE;
	GLint count;
	GLuint prog;
	GLenum err;

	piglit_require_gl_version(20);

	piglit_require_extension("GL_EXT_separate_shader_objects");

	prog = glCreateShaderProgramEXT(GL_VERTEX_SHADER, vs_text);

	err = glGetError();
	if (err != 0) {
		printf("Unexpected OpenGL error state 0x%04x for "
		       "glCreateShaderProgramEXT\n", err);
		pass = GL_FALSE;
	}

	count = -1;
	glGetProgramiv(prog, GL_ATTACHED_SHADERS, &count);
	if (count != 0) {
		printf("Expected attached shader count of 0, got %d.\n", count);
		pass = GL_FALSE;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
