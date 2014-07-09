/*
 * Copyright (c) 2013 Intel Corporation
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
 */

/*
 * Test 2D depth array texture rendering with gl_Layer (AMD_vertex_shader_layer)
 *
 * This test uses layered rendering (gl_Layer) within the vertex shader.
 * Support for gl_Layer in VS is added by the AMD_vertex_shader_layer
 * extension.
 *
 * This test first renders to a depth array texture which is attached to
 * a framebuffer. The texture has 5 layers and 7 LODs.
 *
 * Once depths have been rendered to each array slice & LOD, the test
 * then verifies the depth value in each array slice & LOD.
 */

#include "piglit-util-gl.h"

#define PAD		5
#define SIZE		64
#define LAYERS		5
#define LOD		7
#define DRAW_COUNT	LAYERS * LOD
#define STRINGIFY(x)    #x
#define EXP_STRINGIFY(x) STRINGIFY(x)

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;
	config.supports_gl_compat_version = 31;

	config.window_width = (((SIZE+PAD)*LAYERS)+PAD);
	config.window_height = (((SIZE+PAD)*2)+PAD);
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static GLuint rectangle_vertices_bo;

/* VS and FS to fill the 2D array texture */
static const char *fill_tex_vs = 
 "#version 140\n"
 "#extension GL_AMD_vertex_shader_layer: enable\n"
 "uniform int drawing_level;\n"
 "in vec2 vertex;\n"
 "out vec3 color;\n"
 "int num_layers = " EXP_STRINGIFY(LAYERS) ";\n"
 "int draw_count = " EXP_STRINGIFY(DRAW_COUNT) ";\n"
 "float get_z()\n"
 "{\n"
 " return float((drawing_level * num_layers) + gl_InstanceID) / draw_count;\n"
  "}\n"
 "void main()\n"
 "{\n"
 " gl_Position = vec4(vertex, get_z(), 1.0);\n"
 " gl_Layer = gl_InstanceID;\n"
 "}\n";

static GLuint fill_tex_program;

static bool
render_tex_layers(GLuint tex)
{
	int lod;
	GLint drawing_level_loc, vertex_loc;
	GLenum status;
	int color_index = 0;
	int size;

	glUseProgram(fill_tex_program);

	drawing_level_loc = glGetUniformLocation(fill_tex_program, "drawing_level");

	glBindBuffer(GL_ARRAY_BUFFER, rectangle_vertices_bo);
	vertex_loc = glGetAttribLocation(fill_tex_program, "vertex");
	glVertexAttribPointer(vertex_loc, 2, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(vertex_loc);
	for (lod = 0; lod < LOD; lod++) {
		size = SIZE >> lod;
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex, lod);
		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			fprintf(stderr, "fbo incomplete (status = %s)\n",
			        piglit_get_gl_enum_name(status));
			return false;
		}

		/* Clear background to gray */
		glClearDepth(1.0);
		glClear(GL_DEPTH_BUFFER_BIT);

		glViewport(0, 0, size, size);
		glUniform1i(drawing_level_loc, lod);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, LAYERS);
		color_index += LAYERS;
	}
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
	glDisableVertexAttribArray(vertex_loc);

	return true;
}

static GLuint
build_texture(void)
{
	GLuint tex;
	int lod;
	int size;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
	for (lod = 0; lod < LOD; lod++) {
		size = SIZE >> lod;
		glTexImage3D(GL_TEXTURE_2D_ARRAY, lod, GL_DEPTH_COMPONENT,
		             size, size, LAYERS, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	render_tex_layers(tex);

	return tex;
}

/* Attach the texture layer/lod to the read framebuffer
 */
static void
set_up_read_framebuffer(GLuint tex, int level, int layer)
{
	GLenum status;

	glFramebufferTextureLayer(GL_READ_FRAMEBUFFER,
				  GL_DEPTH_ATTACHMENT,
				  tex, level, layer);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status == GL_FRAMEBUFFER_UNSUPPORTED && level == 0) {
		printf("This buffer combination is unsupported\n");
		piglit_report_result(PIGLIT_SKIP);
	} else if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FBO incomplete at miplevel %d\n", level);
		piglit_report_result(PIGLIT_FAIL);
	}
}

static GLboolean
test_texture(GLuint tex)
{
	int layer, lod;
	GLboolean retval = GL_TRUE;
	float expected;
	float draw_count = LAYERS * LOD;
	int dim = SIZE;

	for (lod = 0; lod < LOD; lod++) {
		for (layer = 0; layer < LAYERS; layer++) {
			GLboolean pass;
			set_up_read_framebuffer(tex, lod, layer);
			expected = ((float)(lod * LAYERS) + layer) / draw_count;
			expected = (expected / 2.0) + 0.5;
			pass = piglit_probe_rect_depth(0, 0, dim, dim, expected);
			retval = retval && pass;
		}
		dim >>= 1;
	}

	return retval;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass;
	GLuint vao, fbo;
	GLuint tex;
	static const GLfloat verts[4][2] = {
		{ 1.0, -1.0},
		{-1.0, -1.0},
		{ 1.0,  1.0},
		{-1.0,  1.0},
	};

	glEnable(GL_DEPTH_TEST);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &rectangle_vertices_bo);
	glBindBuffer(GL_ARRAY_BUFFER, rectangle_vertices_bo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	tex = build_texture();
	pass = test_texture(tex);
	glDeleteTextures(1, &tex);

	glDeleteBuffers(1, &rectangle_vertices_bo);
	glDeleteFramebuffers(1, &fbo);
	glDeleteVertexArrays(1, &vao);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	/* For glFramebufferTexture we need either GL 3.2 or
	 * GL_ARB_geometry_shader4.
	 */
	if (piglit_get_gl_version() < 32) {
		piglit_require_extension("GL_ARB_geometry_shader4");
	}

	piglit_require_extension("GL_AMD_vertex_shader_layer");

	fill_tex_program = piglit_build_simple_program(fill_tex_vs, NULL);
	piglit_check_gl_error(GL_NO_ERROR);
}
