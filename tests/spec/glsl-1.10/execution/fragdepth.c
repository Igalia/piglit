/*
 * Copyright 2014 VMware, Inc.
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
 * Test GLSL gl_FragDepth output.
 * We draw overlapping red and green quads.  The red quad is at Z=0
 * while the green quad's fragment depths vary from left to right.
 * Should see intersecting quads.
 *
 * Brian Paul
 * 5 August 2014
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 20;
	config.window_visual = (PIGLIT_GL_VISUAL_RGBA |
				PIGLIT_GL_VISUAL_DEPTH |
				PIGLIT_GL_VISUAL_DOUBLE);
PIGLIT_GL_TEST_CONFIG_END


static const char *vs_text =
	"varying float z; \n"
	"void main() {\n"
	"   gl_FrontColor = gl_Color; \n"
	"   gl_Position = gl_Vertex;\n"
	"   // convert Z from [-1, 1] to [0, 1] \n"
	"   z = gl_Vertex.x * 0.5 + 0.5; \n"
	"}\n";
static const char *fs_text =
	"varying float z; \n"
	"void main() { \n"
	"   gl_FragDepth = z; \n"
	"   gl_FragColor = gl_Color; \n"
	"}\n";

static GLuint program;


enum piglit_result
piglit_display(void)
{
	static const float red[3] = {1.0, 0.0, 0.0};
	static const float green[3] = {0.0, 1.0, 0.0};
	int x = piglit_width / 2;
	int y = piglit_height / 2;
	bool pass = true;

	glViewport(0, 0, piglit_width, piglit_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Draw a red quad at z = 0 (will be 0.5 in depth range [0,1]) */
	glUseProgram(0);
	glColor3f(1, 0, 0);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(-0.5, -0.5);
	glVertex2f( 0.5, -0.5);
	glVertex2f( 0.5,  0.5);
	glVertex2f(-0.5,  0.5);
	glEnd();

	/* Draw green quad w/ fragment shader which writes gl_FragDepth*/
	glUseProgram(program);
	glColor3f(0, 1, 0);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(-0.75, -0.25);
	glVertex2f( 0.75, -0.25);
	glVertex2f( 0.75,  0.25);
	glVertex2f(-0.75,  0.25);
	glEnd();

	pass = piglit_probe_pixel_rgb(x-10, y, green) && pass;

	pass = piglit_probe_pixel_rgb(x+10, y, red) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_GLSL();

	program = piglit_build_simple_program(vs_text, fs_text);

	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
	glEnable(GL_DEPTH_TEST);
}
