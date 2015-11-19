/*
 * Copyright (c) 2015 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** \file simple-fs.c
 * Simple fragment shader test for textureSamplesIdenticalEXT.
 *
 * Render a simple image.  Scan the image.  At each texel, render green if
 * textureSamplesIdenticalEXT returns false.  If textureSamplesIdenticalEXT
 * returns true, examine each sample.  If the samples are all the same color,
 * render blue.  Render red otherwise.  The test passes if there are zero red
 * pixels and non-zero green pixels.
 *
 * Much code borrowed from tests/spec/arb_texture_multisample/texelfetch.c.
 *
 * \note
 * This is a pretty weak test.  A stronger test would read back the original
 * multisampled image and verify the sample-indenticalness using that.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

    config.supports_gl_compat_version = 30;
    config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define MAX_SAMPLES 32

static const char *vs_src_draw =
	"#version 130\n"
	"\n"
	"in vec4 piglit_vertex;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"}\n"
	;

static const char *fs_src_draw =
	"#version 130\n"
	"\n"
	"out vec4 frag_color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"   frag_color = vec4(0.9, 0.8, 0, 1);\n"
	"}\n"
	;

static const char *vs_src_readback =
	"#version 130\n"
	"\n"
	"in vec4 piglit_vertex;\n"
	"in vec2 piglit_texcoord;\n"
	"\n"
	"out vec2 coord_fs;\n"
	"\n"
	"void main() {\n"
	"    gl_Position = piglit_vertex;\n"
	"    coord_fs = piglit_texcoord;\n"
	"}\n"
	;

static const char *fs_src_readback =
	"#version 130\n"
	"#extension GL_ARB_texture_multisample: require\n"
	"#extension GL_EXT_shader_samples_identical: require\n"
	"\n"
	"uniform sampler2DMS tex;\n"
	"uniform int num_samples;\n"
	"\n"
	"in vec2 coord_fs;\n"
	"out vec4 frag_color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\n"
	"    if (textureSamplesIdenticalEXT(tex, ivec2(coord_fs))) {\n"
	"        frag_color = vec4(0.0, 0.0, 1.0, 1.0);\n"
	"\n"
	"        /* Verify that all the samples have the same color. */\n"
	"        vec4 base = texelFetch(tex, ivec2(coord_fs), 0);\n"
	"        for (int i = 1; i < num_samples; i++) {\n"
	"            vec4 s = texelFetch(tex, ivec2(coord_fs), i);\n"
	"\n"
	"            if (s != base)\n"
	"                frag_color = vec4(1.0, 0.0, 0.0, 1.0);\n"
	"        }\n"
	"    } else {\n"
	"        frag_color = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"    }\n"
	"}\n"
	;

static GLuint tex;
static GLuint fbo;
static GLuint readback_prog, draw_prog;

enum piglit_result
piglit_display(void)
{
	static const GLfloat quad_verts[4][4] = {
		{  0.8,  0.1, 0, 1 },
		{  0.1,  1.0, 0, 1 },
		{ -0.1, -1.0, 0, 1 },
		{ -0.8, -0.1, 0, 1 },
	};
	static const float green[] = { 0.0, 1.0, 0.0, 1.0 };
	static const float blue[]  = { 0.0, 0.0, 1.0, 1.0 };
	unsigned i;
	GLfloat image[32 * 32 * 4];
	unsigned blue_pixels = 0;
	unsigned green_pixels = 0;
	bool pass = true;

	glViewport(0, 0, 32, 32);

	/* Draw triangle into MSAA texture */
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(draw_prog);
	piglit_draw_rect_from_arrays(quad_verts, NULL, false);

	/* Scan the previous render.  Draw green if the samples are all
	 * (verifiably) the same, blue if there may be differences, and red if
	 * there were differences but textureSamplesIdentical said there were
	 * not.
	 */
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.25, 0.25, 0.25, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(readback_prog);

	piglit_draw_rect_tex(-1, -1, 2, 2, 0, 0, 31, 31);

	glReadPixels(0, 0, 32, 32, GL_RGBA, GL_FLOAT, image);

	for (i = 0; i < 32 * 32; i++) {
		if (memcmp(&image[i * 4], blue, sizeof(blue)) == 0) {
			blue_pixels++;
		} else if (memcmp(&image[i * 4], green, sizeof(green)) == 0) {
			green_pixels++;
		} else {
			fprintf(stderr,
				"Bad pixel color @ %u: { %f, %f, %f }\n",
				i,
				image[(i * 4) + 0],
				image[(i * 4) + 1],
				image[(i * 4) + 2]);
			pass = false;
		}
	}

	printf("Blue pixels:  %u\n", blue_pixels);
	printf("Green pixels: %u\n", green_pixels);
	piglit_present_results();

	pass = (green_pixels > 0) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	GLint num_samples_uniform;
	int num_samples = 0;
	GLint max_samples;

	piglit_require_extension("GL_ARB_texture_multisample");
	piglit_require_extension("GL_EXT_shader_samples_identical");

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <sample_count>\n", argv[0]);
		piglit_report_result(PIGLIT_SKIP);
	}

	num_samples = strtoul(argv[1], NULL, 0);
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	assert(num_samples <= MAX_SAMPLES);

	/* create MSAA tex and fbo */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, num_samples,
				GL_RGBA8,
				32, 32, GL_TRUE);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D_MULTISAMPLE,
			       tex, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		piglit_report_result(PIGLIT_FAIL);

	/* create sample readback shader */
	readback_prog = piglit_build_simple_program(vs_src_readback,
						    fs_src_readback);
	glUseProgram(readback_prog);

	num_samples_uniform = glGetUniformLocation(readback_prog,
						   "num_samples");
	glUniform1i(num_samples_uniform, num_samples);

	/* create triangle drawing shader */
	draw_prog = piglit_build_simple_program(vs_src_draw, fs_src_draw);
	glUseProgram(draw_prog);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glEnable(GL_MULTISAMPLE);
}
