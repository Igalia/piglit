/*
 * Copyright © 2013 LunarG, Inc.
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
 * Author: Jon Ashburn <jon@lunarg.com>
 */

/**
 * Tests rendering into a single framebuffer surface with multiple viewports
 * via a geometry shader.  Confirm that each area of the surface delineated by
 * a viewport renders the correct color. Use multiple draw
 * calls to replicate geometry rather than the geometry shader.
 * From the extension registry for ARB_viewport_array:
 *    "This extension enhances OpenGL by providing a mechanism to expose
 *    multiple viewports. Each viewport is specified as a rectangle. The
 *    destination viewport may be selected per-primitive by the geometry
 *    shader. This allows the Geometry Shader to produce different versions
 *    of primitives destined for separate viewport rectangles on the same
 *    surface.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

const char *vsSource = {
	"#version 150\n"
	"in vec4 piglit_vertex;\n"
	"void main() {\n"
	"	gl_Position = piglit_vertex;\n"
	"}\n"
};

const char *gsSource = {
	"#version 150\n"
	"#extension GL_ARB_viewport_array : enable\n"
	"layout(triangles) in;\n"
	"layout(triangle_strip, max_vertices = 3) out;\n"
	"uniform int idx;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_ViewportIndex = idx;\n"
	"	for(int i = 0; i < gl_in.length(); i++) {\n"
	"		gl_Position = gl_in[i].gl_Position;\n"
	"		EmitVertex();\n"
	"	}\n"
	"	EndPrimitive();\n"
	"}\n"
};

const char *fsSource = {
	"#version 150\n"
	"uniform vec3 color;\n"
	"void main() {\n"
	"	gl_FragColor = vec4(color.xyz, 1.0);\n"
	"}\n"
};

static GLint colorLoc;
static GLint vpIndexLoc;

/**
 * Draws a single quad into multiple viewport  each with a different
 * color.  Reads back the expected color  to test if the drawing was correct.
 * @param  changeVPloc;   if true then the geometry shader viewport location
 *         is changed with each loop, otherwise viewport location is fixed.
 */
static bool
draw_multi_viewport(const bool changeVPLoc)
{
	bool pass = true;
	int i, j;
	const int divX=2, divY=4;
	GLfloat w = (GLfloat) piglit_width / (GLfloat) divX;
	GLfloat h = (GLfloat) piglit_height / (GLfloat) divY;
	const GLfloat colors[][3] = {{0.0, 0.0, 1.0},
				  {0.0, 1.0, 0.0},
				  {1.0, 0.0, 0.0},
				  {1.0, 1.0, 0.0},
				  {0.0, 1.0, 1.0},
				  {1.0, 0.0, 1.0},
				  {1.0, 1.0, 1.0},
				  {0.0, 0.0, 0.5},
				  {0.0, 0.0, 0.0}};

	assert(ARRAY_SIZE(colors) == divX*divY + 1);

	glViewport(0, 0, piglit_width, piglit_height); /* for glClear() */
	glClear(GL_COLOR_BUFFER_BIT);
	glUniform1i(vpIndexLoc, divX * divY);
	glViewportIndexedf(divX * divY, -10.0, -30.0, piglit_width, 20.0);
	for (i = 0; i < divX; i++) {
		for (j = 0; j < divY; j++) {
			int p;
			GLfloat *expected;
			glUniform3fv(colorLoc, 1, &colors[j + i*divY][0]);
			if (changeVPLoc) {
				glUniform1i(vpIndexLoc, j + i*divY);
				expected = (GLfloat *) &colors[j + i*divY][0];
			} else  {
				expected = (GLfloat *) &colors[divX * divY][0];
			}
			glViewportIndexedf(j + i*divY, i * w, j * h, w, h);
			piglit_draw_rect(-1, -1, 2, 2);
			pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
			p = piglit_probe_pixel_rgb(i * w + w/2, j * h + h/2,
						   expected);
			piglit_present_results();
			if (!p) {
				printf("Wrong color for viewport i,j %d %d changeVP=%d\n",
				       i, j, changeVPLoc);
				pass = false;
			}
		}
	}
	return pass;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	pass = draw_multi_viewport(true);
	pass = draw_multi_viewport(false) && pass;
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint program;

	piglit_require_extension("GL_ARB_viewport_array");

	program = piglit_build_simple_program_multiple_shaders(
					GL_VERTEX_SHADER, vsSource,
					GL_GEOMETRY_SHADER, gsSource,
					GL_FRAGMENT_SHADER, fsSource,
					0);
	glUseProgram(program);
	colorLoc = glGetUniformLocation(program, "color");
	vpIndexLoc = glGetUniformLocation(program, "idx");
}
