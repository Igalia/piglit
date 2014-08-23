/*
 * Copyright © 2009 Intel Corporation
 * Copyright (c) 2010 VMware, Inc.
 * Copyright © 2011 Red Hat Inc.
 * Copyright © 2014 Advanced Micro Devices, Inc.
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
 * Authors:
 *     Dave Airlie
 *     Christoph Bumiller
 *     Marek Olšák <maraeo@gmail.com>
 */

/** @file fbo-depth-array.c
 *
 * Tests that drawing to or clearing each layer of a depth-stencil array
 * texture FBO and then drawing views of those individual layers
 * to the window system framebuffer succeeds.
 * based on fbo-array.c
 */

#include "piglit-util-gl.h"

/* GL3 requirement. */
#define MAX_DIM 8192
#define MAX_MEM (2048*1024)

enum {
	CLEAR,
	LAYERED_CLEAR,
	DRAW,
	FS_WRITES_VALUE,
};

static bool test_stencil;
static bool test_single_size;
static unsigned width = 32, height = 32, layers = 6;
static unsigned test;

static void parse_args(int argc, char **argv);

PIGLIT_GL_TEST_CONFIG_BEGIN

	piglit_gl_process_args(&argc, argv, &config);
	parse_args(argc, argv);

	config.supports_gl_compat_version = 33;
	config.supports_gl_core_version = 33;

	if (piglit_use_fbo && !test_single_size) {
		config.window_width = MAX_DIM;
		config.window_height = MAX_DIM;
	}
	else {
		config.window_width = (width + 2) * MIN2(layers, 3);
		config.window_height = (height + 2) * ((layers + 2) / 3);
	}
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const char *vs_text =
	"#version 330 \n"
	"layout(location = 0) in vec4 piglit_vertex; \n"
	"layout(location = 1) in vec4 piglit_texcoord; \n"
	"out vec4 texcoord; \n"
	"void main() { \n"
	"  gl_Position = piglit_vertex; \n"
	"  texcoord = piglit_texcoord; \n"
	"}\n";

static const char *frag_shader_empty_text =
	"#version 330 \n"
	"void main() {} \n";

static const char *fs_depth_output_text =
	"#version 330 \n"
	"uniform float z; \n"
	"void main() \n"
	"{ \n"
	"   gl_FragDepth = z; \n"
	"} \n";

static const char *fs_stencil_output_text =
	"#version 330 \n"
	"#extension GL_ARB_shader_stencil_export : require \n"
	"uniform int ref; \n"
	"void main() \n"
	"{ \n"
	"   gl_FragStencilRefARB = ref; \n"
	"} \n";


static const char *fs_texdepth_text =
	"#version 330 \n"
	"uniform sampler2DArray tex; \n"
	"uniform float z; \n"
	"in vec4 texcoord; \n"
	"void main() \n"
	"{ \n"
	"   gl_FragColor = texture(tex, vec3(texcoord.xy, z)).xxxx; \n"
	"} \n";

static const char *fs_texstencil_text =
	"#version 330 \n"
	"uniform usampler2DArray tex; \n"
	"uniform float z; \n"
	"in vec4 texcoord; \n"
	"void main() \n"
	"{ \n"
	"   gl_FragColor = vec4(float(texture(tex, vec3(texcoord.xy, z)))) / 255.0; \n"
	"} \n";

static GLuint program_fs_empty;
static GLuint program_depth_output;
static GLuint program_stencil_output;
static GLuint program_texdepth;
static GLuint program_texstencil;


static float
get_depth_value(unsigned layer)
{
	if (test == LAYERED_CLEAR)
		return 0.4; /* constant */
	else
		return (double)(layer+1) / (layers+1);
}

static unsigned
get_stencil_value(unsigned layer) {
	if (test == LAYERED_CLEAR)
		return 0x53;
	else
		return (layer+1) * 255 / (layers+1);
}

static float
get_stencil_value_float(unsigned layer)
{
	return get_stencil_value(layer) / 255.0;
}

static void
parse_args(int argc, char **argv)
{
	unsigned lwidth, lheight, llayers;
	int i;

	for (i = 1; i < argc; i++) {
		if (sscanf(argv[i], "%ux%ux%u", &lwidth, &lheight, &llayers) == 3 &&
		    lwidth && lheight && llayers) {
			width = lwidth;
			height = lheight;
			layers = llayers;
			test_single_size = true;
		}
		else if (!strcmp(argv[i], "depth-clear")) {
			test = CLEAR;
			puts("Testing glClear");
		}
		else if (!strcmp(argv[i], "depth-layered-clear")) {
			test = LAYERED_CLEAR;
			puts("Testing layered glClear");
		}
		else if (!strcmp(argv[i], "depth-draw")) {
			test = DRAW;
			puts("Testing drawing");
		}
		else if (!strcmp(argv[i], "fs-writes-depth")) {
			test = FS_WRITES_VALUE;
			puts("Testing gl_FragDepth");
		}
		else if (!strcmp(argv[i], "stencil-clear")) {
			test = CLEAR;
			test_stencil = true;
			puts("Testing stencil glClear");
		}
		else if (!strcmp(argv[i], "stencil-layered-clear")) {
			test = LAYERED_CLEAR;
			test_stencil = true;
			puts("Testing stencil layered glClear");
		}
		else if (!strcmp(argv[i], "stencil-draw")) {
			test = DRAW;
			test_stencil = true;
			puts("Testing stencil drawing");
		}
		else if (!strcmp(argv[i], "fs-writes-stencil")) {
			test = FS_WRITES_VALUE;
			test_stencil = true;
			puts("Testing gl_FragStencilRef");
		}
		else {
			puts("Invalid parameter.");
			piglit_report_result(PIGLIT_FAIL);
		}
	}
}

static GLuint
create_array_fbo(void)
{
	GLuint tex, fb, z, ref;
	GLenum status;
	int layer;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
	assert(glGetError() == 0);

	/* allocate empty array texture */
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH24_STENCIL8,
		     width, height, layers, 0,
		     GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	assert(glGetError() == 0);

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	/* draw something into each layer of the array texture */
	for (layer = 0; layer < layers; layer++) {
		if (test == LAYERED_CLEAR) {
			glFramebufferTexture(GL_FRAMEBUFFER,
					     test_stencil ? GL_STENCIL_ATTACHMENT :
							    GL_DEPTH_ATTACHMENT,
					     tex, 0);

			status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE) {
				fprintf(stderr, "FBO incomplete\n");
				goto done;
			}

			glViewport(0, 0, width, height);
			if (test_stencil) {
				glClearStencil(get_stencil_value(0));
				glClear(GL_STENCIL_BUFFER_BIT);
			}
			else {
				glClearDepth(get_depth_value(0));
				glClear(GL_DEPTH_BUFFER_BIT);
			}
			break;
		}
		glFramebufferTextureLayer(GL_FRAMEBUFFER,
					  test_stencil ? GL_STENCIL_ATTACHMENT :
						         GL_DEPTH_ATTACHMENT,
					  tex,
					  0,
					  layer);

		assert(glGetError() == 0);

		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			fprintf(stderr, "FBO incomplete\n");
			goto done;
		}

		glViewport(0, 0, width, height);

		switch (test) {
		case CLEAR:
			if (test_stencil) {
				glClearStencil(get_stencil_value(layer));
				glClear(GL_STENCIL_BUFFER_BIT);
			}
			else {
				glClearDepth(get_depth_value(layer));
				glClear(GL_DEPTH_BUFFER_BIT);
			}
			break;
		case DRAW:
			glUseProgram(program_fs_empty);
			if (test_stencil) {
				glEnable(GL_STENCIL_TEST);
				glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
				glStencilFunc(GL_ALWAYS, get_stencil_value(layer), 0xff);

				piglit_draw_rect(-1, -1, 2, 2);

				glDisable(GL_STENCIL_TEST);
			}
			else {
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_ALWAYS);

				piglit_draw_rect_z(get_depth_value(layer) * 2 - 1,
						   -1, -1, 2, 2);

				glDisable(GL_DEPTH_TEST);
			}
			glUseProgram(0);
			break;
		case FS_WRITES_VALUE:
			if (test_stencil) {
				glUseProgram(program_stencil_output);
				ref = glGetUniformLocation(program_stencil_output, "ref");
				glUniform1i(ref, get_stencil_value(layer));

				glEnable(GL_STENCIL_TEST);
				glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
				glStencilFunc(GL_ALWAYS, 0, 0xff);

				piglit_draw_rect(-1, -1, 2, 2);

				glDisable(GL_STENCIL_TEST);
			}
			else {
				glUseProgram(program_depth_output);
				z = glGetUniformLocation(program_depth_output, "z");
				glUniform1f(z, get_depth_value(layer));

				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_ALWAYS);

				piglit_draw_rect(-1, -1, 2, 2);

				glDisable(GL_DEPTH_TEST);
			}
			glUseProgram(0);
			break;
		}
	}

done:
	glDeleteFramebuffers(1, &fb);
	assert(glGetError() == 0);
	return tex;
}

/* Draw a textured quad, sampling only the given layer of the
 * array texture.
 */
static void
draw_layer(int x, int y, int depth)
{
	GLfloat depth_coord = (GLfloat)depth;
	GLuint prog = test_stencil ? program_texstencil : program_texdepth;
	GLint loc, z;

	glUseProgram(prog);
	loc = glGetUniformLocation(prog, "tex");
	z = glGetUniformLocation(prog, "z");
	glUniform1i(loc, 0); /* texture unit p */
	glUniform1f(z, depth_coord);

	glViewport(0, 0, piglit_width, piglit_height);
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	if (test_stencil)
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_DEPTH_STENCIL_TEXTURE_MODE,
				GL_STENCIL_INDEX);

	piglit_draw_rect_tex((double)x / piglit_width * 2 - 1,
			     (double)y / piglit_height * 2 - 1,
			     (double)width / piglit_width * 2,
			     (double)height / piglit_height * 2,
			     0, 0, 1, 1);
	glUseProgram(0);
	assert(glGetError() == 0);
}

static GLboolean test_layer_drawing(int start_x, int start_y, float expected)
{
	return piglit_probe_rect_r_ubyte(start_x, start_y, width, height,
					 expected * 255.0);
}

static bool
test_once(void)
{
	bool pass = true;
	int layer;
	GLuint tex;

	printf("Testing %ix%ix%i\n", width, height, layers);

	glClearColor(0.2, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	tex = create_array_fbo();

	for (layer = 0; layer < layers; layer++) {
		int x,y;

		if (piglit_use_fbo && !test_single_size) {
			x = 0;
			y = 0;
		}
		else {
			x = 1 + (layer % 3) * (width + 1);
			y = 1 + (layer / 3) * (height + 1);
		}
		draw_layer(x, y, layer);

		pass &= test_layer_drawing(x, y,
					   test_stencil ?
					   get_stencil_value_float(layer) :
					   get_depth_value(layer));

		if (piglit_use_fbo && !test_single_size && layer < layers-1) {
			glClearColor(0.2, 0.1, 0.1, 1.0);
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}

	glDeleteTextures(1, &tex);
	assert(glGetError() == 0);
	return pass;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	int i,j;

	if (piglit_use_fbo && !test_single_size) {
		static int dims[] = {1, 4, 16, 64, 256, 1024, 4096, MAX_DIM};

		for (i = 0; i < ARRAY_SIZE(dims); i++) {
			for (j = 0; j < ARRAY_SIZE(dims); j++) {
				width = dims[i];
				height = dims[j];

				/* Don't allocate too much texture memory. */
				if (width*height > MAX_MEM) {
					if (width > height && height/2 > dims[j-1])
						height /= 2;
					else if (height > width && width/2 > dims[i-1])
						width /= 2;

					if (width*height > MAX_MEM)
						continue;
				}

				pass = test_once() && pass;
			}
		}
	}
	else {
		pass = test_once() && pass;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	if (piglit_get_gl_version() < 33) {
		piglit_report_result(PIGLIT_SKIP);
	}

	if (test_stencil) {
		piglit_require_extension("GL_ARB_stencil_texturing");
		if (test == FS_WRITES_VALUE)
			piglit_require_extension("GL_ARB_shader_stencil_export");
	}

	if (test_stencil) {
		program_texstencil = piglit_build_simple_program(vs_text,
						fs_texstencil_text);

		if (test == FS_WRITES_VALUE)
			program_stencil_output = piglit_build_simple_program(vs_text,
							fs_stencil_output_text);
	}
	else {
		program_texdepth = piglit_build_simple_program(vs_text,
						fs_texdepth_text);

		if (test == FS_WRITES_VALUE)
			program_depth_output = piglit_build_simple_program(vs_text,
							fs_depth_output_text);
	}

	program_fs_empty = piglit_build_simple_program(vs_text,
					frag_shader_empty_text);
	assert(glGetError() == 0);
}
