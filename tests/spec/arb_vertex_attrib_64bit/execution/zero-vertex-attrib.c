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
 *
 */

/**
 * \file zero-vertex-attrib.c
 *
 * Test based on this paragraph of ARB_vertex_attrib_64bit spec:
 * "  void GetVertexAttribLdv(uint index, enum pname, double *params);
 * <skip>
 *  The error INVALID_OPERATION is generated if index
 *  is zero, as there is no current value for generic attribute zero."
 *
 * Also found on 4.1 spec, section 6.1, page 338:
 * "The error INVALID_OPERATION is generated if index is zero and
 * pname is CURRENT_VERTEX_ATTRIB , since there is no current value
 * for generic attribute zero."
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 33;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define CHECK_GL_INVALID_OPERATION \
	if (glGetError() != GL_INVALID_OPERATION) return PIGLIT_FAIL; \
	else printf("max-vertex-attrib test %d passed\n", ++test);

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static GLboolean
run_test(void)
{
        enum piglit_result result = PIGLIT_PASS;
        GLdouble doublev[] = { 1.0, 1.0, 1.0, 1.0 };
        GLfloat floatv[] = { 1.0, 1.0, 1.0, 1.0 };

        glGetVertexAttribfv(0, GL_CURRENT_VERTEX_ATTRIB, floatv);
        if (glGetError () != GL_INVALID_OPERATION) {
                fprintf(stderr, "GL_INVALID_OPERATION expected when calling "
                        "GetVertexAttribfv with index 0 and pname GL_CURRENT_VERTEX_ATTRIB.\n");
                result = PIGLIT_FAIL;
        }

        glGetVertexAttribLdv(0, GL_CURRENT_VERTEX_ATTRIB, doublev);
        if (glGetError () != GL_INVALID_OPERATION) {
                fprintf(stderr, "GL_INVALID_OPERATION expected when calling "
                        "GetVertexAttribLdv with index 0 and pname GL_CURRENT_VERTEX_ATTRIB.\n");
                result = PIGLIT_FAIL;
        }

        return result;
}

void piglit_init(int argc, char **argv)
{
        piglit_require_gl_version(20);

        piglit_require_extension("GL_ARB_vertex_attrib_64bit");

        piglit_report_result(run_test());
}
