/*
 * Copyright © 2011 Intel Corporation
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
 * \file generatemipmap.c
 *
 * Tests that glGenerateMipmapEXT works correctly even when
 * GL_RASTERIZER_DISCARD and/or transform feedback is enabled.  This
 * is important to test because on some implementations,
 * glGenerateMipmapEXT works by temporarily reconfiguring the graphics
 * pipeline to generate the mipmap, and then restoring the old
 * configuration.  We need to ensure that GL_RASTERIZER_DISCARD and
 * transform feedback are appropriately disabled while the mipmap is
 * being generated, and then properly restored later.
 *
 * This test can be run in the following modes (specified by a command
 * line argument):
 *
 * - discard: in this mode the glGenerateMipmapEXT call is made while
 *   GL_RASTERIZER_DISCARD is enabled, and the test verifies that
 *   GL_RASTERIZER_DISCARD is still enabled after the call to
 *   glGenerateMipmapEXT completes.
 *
 * - buffer: in this mode the glGenerateMipmapEXT call is made while
 *   transform feedback is active, and the test verifies that no
 *   vertices were output to the transform feedback buffer while the
 *   glGenerateMipmapEXT call was in progress.  The verification is
 *   performed by checking that the contents of the transform feedback
 *   buffer were not overwritten.
 *
 * - prims_written: in this mode the glGenerateMipmapEXT call is made
 *   while transform feedback is active and while performing a
 *   GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN query, and the test
 *   verifies that the query reports that 0 primitives were written.
 *
 * - prims_generated: in this mode the glGenerateMipmapEXT call is
 *   made while performing a GL_PRIMITIVES_GENERATED query, and the
 *   test verifies that the query reports that 0 primitives were
 *   generated.
 *
 * This file is largely copy-and-pasted from
 * tests/fbo/fbo-generatemipmap.c.
 */

#include "piglit-util-gl.h"

#define TEX_WIDTH 256
#define TEX_HEIGHT 256

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 700;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const float red[] =   {1, 0, 0, 0};
static const float green[] = {0, 1, 0, 0.25};
static const float blue[] =  {0, 0, 1, 0.5};
static const float white[] = {1, 1, 1, 1};
static GLboolean discard = GL_FALSE;
static GLboolean buffer = GL_FALSE;
static GLboolean prims_written = GL_FALSE;
static GLboolean prims_generated = GL_FALSE;
static GLuint xfb_prog;
static GLuint xfb_buf;
static GLuint prims_written_query;
static GLuint prims_generated_query;

static const char *vstext =
	"varying float xfb_out;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_Position = gl_Vertex;\n"
	"  xfb_out = gl_Vertex.x;\n"
	"}\n";

static const char *xfb_varyings[] = { "xfb_out" };

static int
create_texture(void)
{
	GLuint tex;
	float *ptr;

	/* Generate mipmap level 0 */
	tex = piglit_rgbw_texture(GL_RGBA, TEX_WIDTH, TEX_WIDTH, GL_FALSE,
				  GL_TRUE, GL_FLOAT);
	glBindTexture(GL_TEXTURE_2D, tex);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Go into appropriate transform feedback or discard state */
	if (discard)
		glEnable(GL_RASTERIZER_DISCARD);
	if (buffer || prims_written) {
		float buffer[4096];
		buffer[0] = 12345.0;
		glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfb_buf);
		glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(buffer),
			     buffer, GL_STREAM_READ);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf);
		glUseProgram(xfb_prog);
		glBeginTransformFeedback(GL_POINTS);
	}
	if (prims_written) {
		glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN,
			     prims_written_query);
	}
	if (prims_generated) {
		glBeginQuery(GL_PRIMITIVES_GENERATED, prims_generated_query);
	}

	/* Ask the implementation to generate the remaining mipmap levels. */
	glGenerateMipmapEXT(GL_TEXTURE_2D);
	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);

	/* Check state */
	if (discard) {
		if (!glIsEnabled(GL_RASTERIZER_DISCARD)) {
			printf("GL_RASTERIZER_DISCARD state not restored "
			       "after glGenerateMipmapEXT\n");
			piglit_report_result(PIGLIT_FAIL);
		}
		glDisable(GL_RASTERIZER_DISCARD);
	}
	if (buffer || prims_written) {
		glEndTransformFeedback();
		glUseProgram(0);
	}
	if (buffer) {
		ptr = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT,
				  GL_READ_ONLY);
		if (ptr[0] != 12345.0) {
			printf("Transform feedback buffer was overwritten "
			       "during glGenerateMipmapEXT\n");
			piglit_report_result(PIGLIT_FAIL);
		}
	}
	if (prims_written) {
		GLuint result;
		glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
		glGetQueryObjectuiv(prims_written_query,
				    GL_QUERY_RESULT, &result);
		if (result != 0) {
			printf("PRIMITIVES_WRITTEN counter was incremented "
			       "during glGenerateMipmapEXT\n");
			piglit_report_result(PIGLIT_FAIL);
		}
	}
	if (prims_generated) {
		GLuint result;
		glEndQuery(GL_PRIMITIVES_GENERATED);
		glGetQueryObjectuiv(prims_generated_query,
				    GL_QUERY_RESULT, &result);
		if (result != 0) {
			printf("PRIMITIVES_GENERATED counter was incremented "
			       "during glGenerateMipmapEXT\n");
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	return tex;
}

static void
draw_mipmap(int x, int y, int dim)
{
	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	piglit_draw_rect_tex(x, y, dim, dim,
			     0, 0, 1, 1);
	glDisable(GL_TEXTURE_2D);
}

static GLboolean
test_mipmap_drawing(int start_x, int start_y, int dim)
{
	GLboolean pass = GL_TRUE;
	pass = pass && piglit_probe_rect_rgb(
			start_x, start_y, dim/2, dim/2, red);
	pass = pass && piglit_probe_rect_rgb(
			start_x + dim/2, start_y, dim/2, dim/2, green);
	pass = pass && piglit_probe_rect_rgb(
			start_x, start_y + dim/2, dim/2, dim/2, blue);
	pass = pass && piglit_probe_rect_rgb(
			start_x + dim/2, start_y + dim/2, dim/2, dim/2, white);

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int dim;
	GLuint tex;
	int x;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	tex = create_texture();

	x = 1;
	for (dim = TEX_WIDTH; dim > 1; dim /= 2) {
		draw_mipmap(x, 1, dim);
		x += dim + 1;
	}

	x = 1;
	for (dim = TEX_WIDTH; dim > 1; dim /= 2) {
		pass &= test_mipmap_drawing(x, 1, dim);
		x += dim + 1;
	}

	glDeleteTextures(1, &tex);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <mode>\n"
	       "  where <mode> is one of:\n"
	       "    discard\n"
	       "    buffer\n"
	       "    prims_written\n"
	       "    prims_generated\n", prog_name);
	exit(1);
}

void piglit_init(int argc, char **argv)
{
	if (argc != 2)
		print_usage_and_exit(argv[0]);
	if (strcmp(argv[1], "discard") == 0)
		discard = GL_TRUE;
	else if (strcmp(argv[1], "buffer") == 0)
		buffer = GL_TRUE;
	else if (strcmp(argv[1], "prims_written") == 0)
		prims_written = GL_TRUE;
	else if (strcmp(argv[1], "prims_generated") == 0)
		prims_generated = GL_TRUE;
	else
		print_usage_and_exit(argv[0]);

	piglit_require_transform_feedback();

	if (buffer || prims_written) {
		GLuint vs;
		piglit_require_GLSL();
		vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
		xfb_prog = glCreateProgram();
		glAttachShader(xfb_prog, vs);
		glTransformFeedbackVaryings(xfb_prog, 1, xfb_varyings,
					    GL_INTERLEAVED_ATTRIBS);
		glLinkProgram(xfb_prog);
		if (!piglit_link_check_status(xfb_prog)) {
			piglit_report_result(PIGLIT_FAIL);
		}
		glGenBuffers(1, &xfb_buf);
	}
	if (prims_written) {
		glGenQueries(1, &prims_written_query);
	}
	if (prims_generated) {
		glGenQueries(1, &prims_generated_query);
	}
}
