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

/** \file
 *
 * Verify that the "centroid" qualifier is properly respected when
 * used inside an interface block.
 *
 * This test operates by comparing varyings declared inside an
 * interface block with varyings declared outside an interface block.
 * We assume that the "centroid" qualifier works properly when
 * declared outside interface blocks, because that is tested by other
 * piglit tests.
 *
 * The test may be run in different modes in order to test:
 * - an unnamed interface block
 * - a named interface block (non-array)
 * - an interface block array
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END


/**
 * Vertex data.  This triangle is deliberately chosen to be at a
 * skewed angle so that some of its boundary pixels will be <50%
 * covered (and therefore will have a different value for
 * centroid-interpolated inputs).
 */
const float vertex_data[3][4] = {
	{ -1.0, -1.0, 0.0, 1.0 },
	{ -0.9,  1.0, 0.0, 1.0 },
	{  1.0,  0.8, 0.0, 1.0 }
};


const char *vs_text_unnamed =
	"#version 150\n"
	"in vec4 piglit_vertex;\n"
	"centroid out float centroid_var;\n"
	"out float unqualified_var;\n"
	"out Blk {\n"
	"  centroid float ifc_centroid_var;\n"
	"  float ifc_unqualified_var;\n"
	"};\n"
	"void main()\n"
	"{\n"
	"  gl_Position = piglit_vertex;\n"
	"  float var = float(gl_VertexID);\n"
	"  centroid_var = var;\n"
	"  unqualified_var = var;\n"
	"  ifc_centroid_var = var;\n"
	"  ifc_unqualified_var = var;\n"
	"}\n";

const char *fs_text_unnamed =
	"#version 150\n"
	"centroid in float centroid_var;\n"
	"in float unqualified_var;\n"
	"in Blk {\n"
	"  centroid float ifc_centroid_var;\n"
	"  float ifc_unqualified_var;\n"
	"};\n"
	"void main()\n"
	"{\n"
	"  bool ok = true;\n"
	"  if (centroid_var != ifc_centroid_var) ok = false;\n"
	"  if (unqualified_var != ifc_unqualified_var) ok = false;\n"
	"  gl_FragColor = ok ? vec4(0.0, 1.0, 0.0, 1.0)\n"
	"                    : vec4(1.0, 0.0, 0.0, 1.0);\n"
	"}\n";

const char *vs_text_named =
	"#version 150\n"
	"in vec4 piglit_vertex;\n"
	"centroid out float centroid_var;\n"
	"out float unqualified_var;\n"
	"out Blk {\n"
	"  centroid float centroid_var;\n"
	"  float unqualified_var;\n"
	"} ifc;\n"
	"void main()\n"
	"{\n"
	"  gl_Position = piglit_vertex;\n"
	"  float var = float(gl_VertexID);\n"
	"  centroid_var = var;\n"
	"  unqualified_var = var;\n"
	"  ifc.centroid_var = var;\n"
	"  ifc.unqualified_var = var;\n"
	"}\n";

const char *fs_text_named =
	"#version 150\n"
	"centroid in float centroid_var;\n"
	"in float unqualified_var;\n"
	"in Blk {\n"
	"  centroid float centroid_var;\n"
	"  float unqualified_var;\n"
	"} ifc;\n"
	"void main()\n"
	"{\n"
	"  bool ok = true;\n"
	"  if (centroid_var != ifc.centroid_var) ok = false;\n"
	"  if (unqualified_var != ifc.unqualified_var) ok = false;\n"
	"  gl_FragColor = ok ? vec4(0.0, 1.0, 0.0, 1.0)\n"
	"                    : vec4(1.0, 0.0, 0.0, 1.0);\n"
	"}\n";

const char *vs_text_array =
	"#version 150\n"
	"in vec4 piglit_vertex;\n"
	"centroid out float centroid_var;\n"
	"out float unqualified_var;\n"
	"out Blk {\n"
	"  centroid float centroid_var;\n"
	"  float unqualified_var;\n"
	"} ifc[2];\n"
	"void main()\n"
	"{\n"
	"  gl_Position = piglit_vertex;\n"
	"  float var = float(gl_VertexID);\n"
	"  centroid_var = var;\n"
	"  unqualified_var = var;\n"
	"  for (int i = 0; i < 2; i++) {\n"
	"    ifc[i].centroid_var = var;\n"
	"    ifc[i].unqualified_var = var;\n"
	"  }\n"
	"}\n";

const char *fs_text_array =
	"#version 150\n"
	"centroid in float centroid_var;\n"
	"in float unqualified_var;\n"
	"in Blk {\n"
	"  centroid float centroid_var;\n"
	"  float unqualified_var;\n"
	"} ifc[2];\n"
	"void main()\n"
	"{\n"
	"  bool ok = true;\n"
	"  for (int i = 0; i < 2; i++) {\n"
	"    if (centroid_var != ifc[i].centroid_var) ok = false;\n"
	"    if (unqualified_var != ifc[i].unqualified_var) ok = false;\n"
	"  }\n"
	"  gl_FragColor = ok ? vec4(0.0, 1.0, 0.0, 1.0)\n"
	"                    : vec4(1.0, 0.0, 0.0, 1.0);\n"
	"}\n";


static GLuint prog;
static GLuint fbo;


static void
print_usage_and_exit(const char *prog_name)
{
	printf("Usage: %s <subtest>\n"
	       "  where <subtest> is one of:\n"
	       "    unnamed: use an unnamed interface block\n"
	       "    named: use a named interface block (non-array)\n"
	       "    array: use an interface block array\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}


void
piglit_init(int argc, char **argv)
{
	GLuint rb;
	GLuint vao;
	GLuint array_buf;

	/* Parse params */
	if (argc != 2)
		print_usage_and_exit(argv[0]);
	if (strcmp(argv[1], "unnamed") == 0) {
		prog = piglit_build_simple_program(vs_text_unnamed,
						   fs_text_unnamed);
	} else if (strcmp(argv[1], "named") == 0) {
		prog = piglit_build_simple_program(vs_text_named,
						   fs_text_named);
	} else if (strcmp(argv[1], "array") == 0) {
		prog = piglit_build_simple_program(vs_text_array,
						   fs_text_array);
	} else {
		print_usage_and_exit(argv[0]);
	}

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

	/* Set up vertex inputs */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &array_buf);
	glBindBuffer(GL_ARRAY_BUFFER, array_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), &vertex_data,
		     GL_STATIC_DRAW);
	glVertexAttribPointer(PIGLIT_ATTRIB_POS, 4, GL_FLOAT, GL_FALSE, 0,
			      NULL);
	glEnableVertexAttribArray(PIGLIT_ATTRIB_POS);
}


enum piglit_result
piglit_display(void)
{
	const float green[4] = { 0.0, 1.0, 0.0, 1.0 };
	bool pass = true;

	/* Set up to draw into the multisampled renderbuffer */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(prog);

	/* Draw a rectangle covering the entire buffer */
	piglit_draw_rect(-1, -1, 2, 2);

	/* Draw a triangle where some samples are <50% covered */
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
