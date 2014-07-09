/*
 * Copyright © 2013 Marek Olšák <maraeo@gmail.com>
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
 * Tests if PRIMITIVES_GENERATED works with transform feedback disabled.
 *
 * From EXT_transform_feedback:
 *    "the primitives-generated count is incremented every time a primitive
 *     reaches the Discarding Rasterization stage"
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const char *vstext = {
	"void main() {"
	"  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
	"  gl_FrontColor = vec4(1.0);"
	"}"
};

GLuint prog;
GLuint q;

void piglit_init(int argc, char **argv)
{
	GLuint vs;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	/* Check the driver. */
	piglit_require_gl_version(15);
	piglit_require_GLSL();
	piglit_require_transform_feedback();

	glGenQueries(1, &q);

	/* Create shaders. */
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}
        glUseProgram(prog);

	glClearColor(0.2, 0.2, 0.2, 1.0);
}

enum piglit_result piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	unsigned qresult;
	int expected = 2;

	glClear(GL_COLOR_BUFFER_BIT);
	glBeginQuery(GL_PRIMITIVES_GENERATED, q);
        piglit_draw_rect(10, 10, 10, 10);
	glEndQuery(GL_PRIMITIVES_GENERATED_EXT);
	glGetQueryObjectuiv(q, GL_QUERY_RESULT, &qresult);

        if (qresult != expected) {
		printf("Primitives generated: %i,  Expected: %i\n", qresult, expected);
		pass = GL_FALSE;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
