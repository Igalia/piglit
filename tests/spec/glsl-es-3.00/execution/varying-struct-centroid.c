/*
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file varying-struct-centroid.c
 *
 * Test that varying structs work properly.
 *
 * From the GLSL ES 3.00 specification, section 4.3.4 ("Input Variables"):
 *
 *     Fragment inputs can only be signed and unsigned integers and
 *     integer vectors, float, floating-point vectors, matrices, or
 *     arrays or structures of these.
 *
 * And from section 4.3.6 ("Output Variables"):
 *
 *     Vertex output variables ... can only be float, floating-point
 *     vectors, matrices, signed or unsigned integers or integer
 *     vectors, or arrays or structures of any these.
 *
 * This tests that the elements of varying structs properly respect the
 * "centroid" keyword.
 *
 * The test functions as follows:
 *
 * - Create a vertex and fragment shader whose varyings are (1) a vec4
 *   using normal interpolation, (2) a vec4 using centroid
 *   interpolation, (3) a struct using normal interpolation, and (4) a
 *   struct using centroid interpolation.  Both structs contain a
 *   single vec4.  The fragment shader compares the vec4's inside the
 *   structs with the corresponding non-structured vec4's, and outputs
 *   red or green depending whether they match.
 *
 * - Create a multisampled renderbuffer.
 *
 * - Draw a rectangle that covers the entire renderbuffer.
 *
 * - Draw a triangle over the top of this rectangle, where the
 *   coordinates have been chosen to ensure that at least some pixels
 *   are less than 50% covered (these pixels will have their
 *   centroid-interpolated varyings differ from their
 *   non-centroid-interpolated varyings since the center of the pixel
 *   is not covered).
 *
 * - Use a blit to downsample the image to the screen.
 *
 * - Check that all pixels are green.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_text[] =
	"#version 300 es\n"
	"in vec4 piglit_vertex;\n"
	"struct Foo {\n"
	"  vec4 v;\n"
	"};\n"
	"out Foo foo;\n"
	"centroid out Foo foo_centroid;\n"
	"out vec4 ref;\n"
	"centroid out vec4 ref_centroid;\n"
	"void main()\n"
	"{\n"
	"  gl_Position = piglit_vertex;\n"
	"  foo.v = piglit_vertex;\n"
	"  foo_centroid.v = piglit_vertex;\n"
	"  ref = piglit_vertex;\n"
	"  ref_centroid = piglit_vertex;\n"
	"}\n";

static const char fs_text[] =
	"#version 300 es\n"
	"precision mediump float;\n"
	"struct Foo {\n"
	"  vec4 v;\n"
	"};\n"
	"in Foo foo;\n"
	"centroid in Foo foo_centroid;\n"
	"in vec4 ref;\n"
	"centroid in vec4 ref_centroid;\n"
	"out vec4 color;\n"
	"void main()\n"
	"{\n"
	"  if (distance(foo.v, ref) > 0.00001\n"
	"      || distance(foo_centroid.v, ref_centroid) > 0.00001) {\n"
	"    color = vec4(1.0, 0.0, 0.0, 1.0);\n"
	"  } else {\n"
	"    color = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"  }\n"
	"}\n";


static GLuint prog;
static GLuint fbo;
static GLuint rb;


void
piglit_init(int argc, char **argv)
{
	/* Create the shaders */
	prog = piglit_build_simple_program(vs_text, fs_text);

	/* Create the multisampled framebuffer */
	glGenFramebuffers(1, &fbo);
	glGenRenderbuffers(1, &rb);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER,
					 4 /* samples */,
					 GL_RGBA8 /* internalformat */,
					 piglit_width, piglit_height);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, rb);
	if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)
	    != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer incomplete\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	const float verts[3][4] = {
		{ -1.0, -1.0, 0.0, 1.0 },
		{ -0.9,  1.0, 0.0, 1.0 },
		{  1.0,  0.8, 0.0, 1.0 }
	};
	const float green[4] = { 0.0, 1.0, 0.0, 1.0 };
	bool pass = true;

	/* Set up to draw into the multisampled renderbuffer */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(prog);

	/* Draw a rectangle covering the entire buffer */
	piglit_draw_rect(-1, -1, 2, 2);

	/* Draw a triangle where some samples are <50% covered */
	glVertexAttribPointer(PIGLIT_ATTRIB_POS, 4, GL_FLOAT, GL_FALSE, 0,
			      verts);
	glEnableVertexAttribArray(PIGLIT_ATTRIB_POS);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	/* Blit to the main window to downsample the image */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, piglit_width, piglit_height,
			  0, 0, piglit_width, piglit_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Check that the image is all green */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      green) && pass;

	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
