/**
 * Copyright © 2013 Intel Corporation
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
 * Test that geometry shaders only compile with valid output layout qualifiers
 *
 * Section 4.3.8.2(Output Layout Qualifiers) of the GLSL 1.50 spec says:
 * "Geometry shaders can have output layout qualifiers only on the interface
 *  qualifier out, not on an output block or variable declaration.  The layout
 *  qualifier identifiers for geometry shader outputs are
 *	points
 *	line_strip
 *	triangle_strip
 *	max_vertices = integer-constant"
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
        config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *gstemplate =
	"#version 150\n"
	"#define LAYOUT_OUT %s\n"
	"layout(points) in;\n"
	"layout(LAYOUT_OUT, max_vertices = 3) out;\n"
	"void main() {\n"
	"}\n";

char *valids[] = {"points",
		  "line_strip",
		  "triangle_strip"};

const char *layout;

static void
print_usage_and_exit(const char *prog_name)
{
	printf("Usage: %s <output_layout>\n"
	       "  where <output_layout> is the qualifier to test to see if it\n"
	       "  is a valid geometry shader output layout qualifier\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	GLuint gs = 0;
	GLint gsCompiled = GL_TRUE;
	char* gstext = NULL;
	int i = 0;
	bool pass = true;
	GLint expected_compile_result;

	/* Parse params */
	if (argc != 2) {
		print_usage_and_exit(argv[0]);
	}

	layout = argv[1];

	/* figure out if we expect compilation to be successful. */
	expected_compile_result = GL_FALSE;
	for (i = 0; i < ARRAY_SIZE(valids); i++) {
		if (strcmp(layout, valids[i]) == 0) {
			expected_compile_result = GL_TRUE;
			break;
		}
	}

	asprintf(&gstext, gstemplate, layout);
	gs = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(gs, 1, (const GLchar **) &gstext, NULL);
	glCompileShader(gs);
	free(gstext);

	/* check compile result */
	glGetShaderiv(gs, GL_COMPILE_STATUS, &gsCompiled);
	if (gsCompiled != expected_compile_result) {
		if (expected_compile_result) {
			printf("Failed to compile with output qualifier "
			       "\"%s\".\n", layout);
		} else {
			printf("\"%s\" is an invalid output qualifier "
			       "but geometry shader still compiled.\n",
			       layout);
		}
		pass = false;
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
