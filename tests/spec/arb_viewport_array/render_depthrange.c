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
 * Draws quads into a single framebuffer surface with multiple viewports
 * via a geometry shader.  Each different viewport has a different DepthRange.
 * The fragment shader outputs a color based on the depthRange, z, and viewport
 * index. Confirm that each quad has the correct color and therefore has
 * the correct depthRange for that viewport index.
 * From the OpenGL 4.3 Core3 Profile Spec section 13.6:
 *     "DepthRangeIndexed specifies the depth range for a single viewport and
 *     is equivalent (assuming no errors are generated) to:
 *         double v[] = { n, f };
 *         DepthRangeArrayv(index, 1, v);"
 *     "DepthRange sets the depth range for all viewports to the same values
 *     and is equivalent (assuming no errors are generated) to:
 *         for (uint i = 0; i < MAX_VIEWPORTS; i++)
 *         DepthRangeIndexed(i, n, f );"
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
	"flat out int ViewportIndex;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_ViewportIndex = idx;\n"
	"	ViewportIndex = idx;\n"
	"	for(int i = 0; i < gl_in.length(); i++) {\n"
	"		gl_Position = gl_in[i].gl_Position;\n"
	"		EmitVertex();\n"
	"	}\n"
	"	EndPrimitive();\n"
	"}\n"
};

const char *fsSource = {
	"#version 150\n"
	"#extension GL_ARB_viewport_array : enable\n"
	"uniform vec3 color;\n"
	"flat in int ViewportIndex;\n"
	"void main() {\n"
	"	float idx = ViewportIndex / 10.0;\n"
	"	gl_FragColor = vec4(gl_FragCoord.z, gl_DepthRange.far, idx, 1.0);\n"
	"}\n"
};

static GLint colorLoc;
static GLint vpIndexLoc;

#define DIVX 2
#define DIVY 4

/**
 * Draws a single quad into multiple viewport each with a different
 * depthRange and fixed Z plane.  Reads back the expected color (which is a
 * a function (funcOf(Z, depthRange, viewportIndex)) to test if the drawing
 * with different depthRanges per viewport index is correct.
 * From GLSL 4.30.6 Spec section 7.4:
 *     "Depth range in window coordinates,
 *      section 13.6.1 “Controlling the Viewport” in the
 *      OpenGL Graphics System Specification.
 *      Note: Depth-range state is only for viewport 0."
 *
 */
static bool
draw_multi_viewport(void)
{
	bool pass = true;
	int i, j;
	GLfloat w = (GLfloat) piglit_width / (GLfloat) DIVX;
	GLfloat h = (GLfloat) piglit_height / (GLfloat) DIVY;
	GLfloat zVal = 0.25f;
	GLfloat drFar = 0.6f;
	GLfloat colors[DIVX * DIVY][3];
	const GLdouble depthRange[][2] = {{0.5, 1.0},
				    {0.0, 0.8},
				    {1.0, 0.75},
				    {0.3, 0.8},
				    {0.7, 0.6},
				    {0.9, 0.1},
				    {0.1, 0.9},
				    {0.2, 0.4}};

	assert(ARRAY_SIZE(depthRange) == DIVX*DIVY);

	glViewport(0, 0, piglit_width, piglit_height); /* for glClear() */
	glClearDepthf(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthRangeIndexed(0, 0.4, drFar);
	glDepthFunc(GL_ALWAYS);

	/* initialize expected colors
	 * Frag shader uses FragCoord.z for the Red, DepthRange[0].far for
	 * Green color, and Blue is viewportIndex / 10.0
	 */
	for (i = 0; i < DIVX * DIVY; i++) {
		GLfloat nearZ = (GLfloat) depthRange[i][0];
		GLfloat farZ = (GLfloat) depthRange[i][1];
		colors[i][0] = (((farZ - nearZ) * zVal)  + nearZ + farZ) / 2.0f;
		colors[i][1] = drFar;
		colors[i][2] = (GLfloat) (i + 1) / 10.0f;
	}

	/* draw with varying viewports and depth ranges */
	for (i = 0; i < DIVX; i++) {
		for (j = 0; j < DIVY; j++) {
			int p, idx;
			/* start at index 1 instead of zero, since index 0
			 *  contains the Frag Shader gl_DepthRange value
			 */
			idx = j + 1 + i*DIVY;
			glUniform3fv(colorLoc, 1, &colors[idx-1][0]);
			glUniform1i(vpIndexLoc, idx);
			glViewportIndexedf(idx, i * w, j * h, w, h);
			glDepthRangeIndexed(idx, depthRange[idx-1][0],
					    depthRange[idx-1][1]);
			piglit_draw_rect_z(zVal, -1.0, -1.0, 2.0, 2.0);
			pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
			p = piglit_probe_pixel_rgb(i * w + w/2, j * h + h/2,
						   &colors[idx-1][0]);
			piglit_present_results();
			if (!p) {
				printf("Wrong color for viewport i,j %d %d\n",
				       i, j);
				pass = false;
			}
		}
	}
	return pass;
}

enum piglit_result
piglit_display(void)
{
	bool pass= true;

	pass = draw_multi_viewport();
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
