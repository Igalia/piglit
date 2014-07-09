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
 * via a geometry shader.  Scissoring is used to restrict quads to a smaller
 * area on the surface.  Confirm that each area of the surface delineated by
 * a scissor rectangle for viewport index renders the correct color. Both
 * indexed scissor tests and indexed scissor enables  are used. Geometry
 * shader is used to expand a single rectangle to N rectangles.
 * OpenGL 4.3 Core Profile spec section 13.7.2 covers this test:
 *    "The scissor test determines if (xw , yw ) lies within the scissor
 *    rectangle defined by four values for each viewport."
 *    "If left ≤ xw < left + width and bottom ≤ yw < bottom + height for the
 *    selected scissor rectangle, then the scissor test passes. Otherwise, the
 *    test fails and the fragment is discarded. For points, lines, and
 *    polygons, the scissor rectangle for a primitive is selected in the same
 *    manner as the viewport (see section 13.6.1)."
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/* number of viewport/scissor rectangle divisons in x and y */
static const int divX=2, divY=3;

/**
 * Helper  function to draw a quad and check the results for divX*divY areas
 * on the screen.
 */
static bool
draw_check_pixels(void)
{
	bool pass = true;
	int i, j;
	GLfloat w = (GLfloat) piglit_width / (GLfloat) divX;
	GLfloat h = (GLfloat) piglit_height / (GLfloat) divY;

	/* draw single quad, expanded to divX*divY quads via geometry shader */
	piglit_draw_rect(-1, -1, 2, 2);
	piglit_present_results();

	/* check rendering results: greyscale RGB == 1.0 / (index +1)*/
	for (i = 0; i < divX; i++) {
		for (j = 0; j < divY; j++) {
			GLfloat expected[4];
			int p;
			expected[0] =
			expected[1] =
			expected[2] = 1.0 / (GLfloat) (1 + j + i*divY);
			expected[3] = 1.0;
			p = piglit_probe_rect_rgba(i * w + 1, j * h + 1,
						   w-2, h-2, expected);
			if (!p) {
				printf("Wrong color for viewport i,j %d %d\n",
				       i, j);
				pass = false;
			}
		}
	}
	return pass;
}

/**
 * Draws a single quad full window size, with different scissor rectangles.
 * Scissor rectangles restrict drawing to sub-area of full window.
 * Geometry shader is responsible for expanding primitives to cover all
 * divX * divY viewport/scissor indices.  The function reads back the expected
 * color to test if the scissored drawing was correct.
 */
static bool
draw_multi_scissor_rect(void)
{
	bool pass = true;
	int i, j;
	GLfloat w = (GLfloat) piglit_width / (GLfloat) divX;
	GLfloat h = (GLfloat) piglit_height / (GLfloat) divY;

	glViewport(0, 0, piglit_width, piglit_height);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_SCISSOR_TEST);
	/* setup scissor rectangles for viewport indices */
	for (i = 0; i < divX; i++) {
		for (j = 0; j < divY; j++) {
			glScissorIndexed(j + i*divY, i * w, j * h, w, h);
		}
	}

	/* draw full viewport sized quads scissored down and check results */
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	pass = draw_check_pixels() & pass;
	glDisable(GL_SCISSOR_TEST);
	return pass;
}

/**
 * Draws a single quad full window size, with different scissor rectangles
 * and different scissor test enables for each viewport index.
 * Scissor rectangles or viewport restrict drawing to sub-area of full
 * window surface.  Geometry shader is responsible for exapnding primitves
 * to cover all divX * divY viewport/scissor indices.  The function reads
 * back the expected color to test if the scissored drawing was correct.
 */
static bool
draw_multi_viewport_scissor(void)
{
	bool pass = true, scEnabled;
	int i, j;
	GLfloat w = (GLfloat) piglit_width / (GLfloat) divX;
	GLfloat h = (GLfloat) piglit_height / (GLfloat) divY;

	/* Setup scissor/viewport rectangles and enables for indices.
	 * Every other index has SCISSOR_TEST enabled with a scissor
	 * rectangle that restricts rendering to the sub-region (wxh).
	 * The other indices  restrict rendering by making the viewport
	 * restricted to the sub-region (wxh sized).  For the indices with
	 * viewport restricted rendering, the SCISSOR_TEST is alternatively
	 * enabled/disabled.
	 */
	glScissor(0, 0, piglit_width, piglit_height);
	glViewport(0, 0, piglit_width, piglit_height);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_SCISSOR_TEST);
	scEnabled = false;
	for (i = 0; i < divX; i++) {
		for (j = 0; j < divY; j++) {
			GLuint idx = j + i*divY;
			if (scEnabled) {
				/* use viewport to restrict rendering */
				if (i & 1)
					glDisablei(GL_SCISSOR_TEST, idx);
				glViewportIndexedf(idx, i * w, j * h, w, h);
				scEnabled = false;
			} else {
				/* use scissor to restrict rendering */
				glScissorIndexed(idx, i * w, j * h, w, h);
				scEnabled = true;
			}
		}
	}

	/* draw restricted size quads with scissoring enabled/disabled */
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	pass = draw_check_pixels() & pass;
	glDisable(GL_SCISSOR_TEST);
	return pass;
}

#define X(f, desc)					     	\
	do {							\
		const bool subtest_pass = (f);			\
		piglit_report_subtest_result(subtest_pass	\
					     ? PIGLIT_PASS : PIGLIT_FAIL, \
					     (desc));		\
		pass = pass && subtest_pass;			\
	} while (0)

enum piglit_result
piglit_display(void)
{
	bool pass= true;

	X(draw_multi_scissor_rect(), "Render multi-scissor rectangles");
	X(draw_multi_viewport_scissor(), "Render multi-viewport scissor test");
#undef X
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint program;
	char *vsSource;
	char *gsSource;
	char *fsSource;

	piglit_require_extension("GL_ARB_viewport_array");

	asprintf(&vsSource,
		 "#version 150\n"
		 "in vec4 piglit_vertex;\n"
		 "void main() {\n"
		 "	gl_Position = piglit_vertex;\n"
		 "}\n");

	asprintf(&gsSource,
		 "#version 150\n"
		 "#extension GL_ARB_viewport_array : enable\n"
		 "layout(triangles) in;\n"
		 "layout(triangle_strip, max_vertices = 18) out;\n"
		 "out vec3 color;\n"
		 "\n"
		 "void main()\n"
		 "{\n"
		 "	for (int j = 0; j < %d; j++) {\n"
		 "		gl_ViewportIndex = j;\n"
		 "		color = vec3(1.0 / (j+1), 1.0 / (j+1), 1.0 / (j+1));\n"
		 "		for(int i = 0; i < gl_in.length(); i++) {\n"
		 "			gl_Position = gl_in[i].gl_Position;\n"
		 "			EmitVertex();\n"
		 "		}\n"
		 "		EndPrimitive();\n"
		 "	}\n"
		 "}\n", divX * divY);

	asprintf(&fsSource,
		 "#version 150\n"
		 "in vec3 color;\n"
		 "void main() {\n"
		 "	gl_FragColor = vec4(color.xyz, 1.0);\n"
		 "}\n");


	program = piglit_build_simple_program_multiple_shaders(
					GL_VERTEX_SHADER, vsSource,
					GL_GEOMETRY_SHADER, gsSource,
					GL_FRAGMENT_SHADER, fsSource,
					0);
	glUseProgram(program);
}
