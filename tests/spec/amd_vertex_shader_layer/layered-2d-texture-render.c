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
 * Test 2D array texture rendering with gl_Layer (AMD_vertex_shader_layer)
 *
 * This test uses layered rendering (gl_Layer) within the vertex shader.
 * Support for gl_Layer in VS is added by the AMD_vertex_shader_layer
 * extension.
 *
 * This test first draws to a color array texture which is attached to
 * a framebuffer. The texture has 5 layers and 7 LODs.
 *
 * Once colors have been rendered to each array slice & LOD, the test
 * then uses the texture to draw on the system framebuffer and verifies
 * that the expected colors appear.
 */

#include "piglit-util-gl.h"

#define PAD		5
#define SIZE		64
#define LAYERS		5
#define LOD		7

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
 "uniform int color_bias;\n"
 "in vec2 vertex;\n"
 "out vec3 color;\n"
 "vec3 get_color(int num)\n"
 "{\n"
 " vec3 result = vec3(0.0);\n"
 " if ((num & 4) != 0) result.r = 1.0;"
 " if ((num & 2) != 0) result.g = 1.0;"
 " if ((num & 1) != 0) result.b = 1.0;"
 " return result;\n"
 "}\n"
 "void main()\n"
 "{\n"
 " gl_Position = vec4(vertex, vec2(0.0, 1.0));\n"
 " gl_Layer = gl_InstanceID;\n"
 " color = get_color(color_bias + gl_InstanceID);\n"
 "}\n";

static const char *fill_tex_fs = 
 "#version 140\n"
 "in vec3 color;\n"
 "void main()\n"
 "{\n"
 " gl_FragColor = vec4(color, 1.0);\n"
 "}\n";

/* VS and FS to use and test the 2D array texture */
static const char *use_tex_vs = 
 "#version 130\n"
 "in vec2 vertex;\n"
 "out vec2 coord;\n"
 "void main()\n"
 "{\n"
 " gl_Position = vec4(vertex, vec2(0.0, 1.0));\n"
 " coord = (vertex * 0.5) + 0.5;\n"
 "}\n";

static const char *use_tex_fs =
 "#version 130\n"
 "uniform sampler2DArray tex; \n"
 "uniform int layer;\n"
 "uniform int lod;\n"
 "in vec2 coord;\n"
 "void main()\n"
 "{\n"
 " gl_FragColor = textureLod(tex, vec3(coord, float(layer)), lod);\n"
 "}\n";

static GLuint fill_tex_program;
static GLuint use_tex_program;

static int get_x(int layer)
{
	return ((SIZE + PAD) * layer) + PAD;
}

static int get_y(int layer, int lod)
{
	int size = SIZE >> lod;
	return PAD + (((1 << lod) - 1) * 2 * size);
}

static const GLfloat *get_color(int num)
{
	int color_index;

	static const GLfloat colors[][3] = {
		{0.0, 0.0, 0.0},
		{0.0, 0.0, 1.0},
		{0.0, 1.0, 0.0},
		{0.0, 1.0, 1.0},
		{1.0, 0.0, 0.0},
		{1.0, 0.0, 1.0},
		{1.0, 1.0, 0.0},
		{1.0, 1.0, 1.0},
	};

	color_index = num % ARRAY_SIZE(colors);
	return colors[color_index];
}

static bool
render_tex_layers(GLuint tex)
{
	int lod;
	GLint color_bias_loc;
	GLint vertex_loc;
	GLenum status;
	int color_index = 0;
	int size;

	glUseProgram(fill_tex_program);

	color_bias_loc = glGetUniformLocation(fill_tex_program, "color_bias");

	glBindBuffer(GL_ARRAY_BUFFER, rectangle_vertices_bo);
	vertex_loc = glGetAttribLocation(fill_tex_program, "vertex");
	glVertexAttribPointer(vertex_loc, 2, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(vertex_loc);
	for (lod = 0; lod < LOD; lod++) {
		size = SIZE >> lod;
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, lod);
		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			fprintf(stderr, "fbo incomplete (status = %s)\n",
			        piglit_get_gl_enum_name(status));
			return false;
		}
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glViewport(0, 0, size, size);
		glUniform1i(color_bias_loc, color_index);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, LAYERS);
		color_index += LAYERS;
	}
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
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
		glTexImage3D(GL_TEXTURE_2D_ARRAY, lod, GL_RGBA,
		             size, size, LAYERS, 0, GL_RGBA, GL_FLOAT, NULL);
	}

	render_tex_layers(tex);

	return tex;
}

static void
draw_box(GLuint tex, int layer, int lod)
{
	GLint layer_loc, lod_loc, vertex_loc;
	int x = get_x(layer);
	int y = get_y(layer, lod);
	int size = SIZE >> lod;

	layer_loc = glGetUniformLocation(use_tex_program, "layer");
	lod_loc = glGetUniformLocation(use_tex_program, "lod");

	glBindBuffer(GL_ARRAY_BUFFER, rectangle_vertices_bo);

	vertex_loc = glGetAttribLocation(use_tex_program, "vertex");
	glVertexAttribPointer(vertex_loc, 2, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(vertex_loc);

	glBindTexture(GL_TEXTURE_2D_ARRAY, tex);

	glViewport(x, y, size, size);
	glUniform1i(layer_loc, layer);
	glUniform1i(lod_loc, lod);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(vertex_loc);
}

static GLboolean
test_results(int layer, int lod)
{
	int x = get_x(layer);
	int y = get_y(layer, lod);
	const GLfloat *expected_color3f = get_color((lod * LAYERS) + layer);
	GLboolean pass;
	int size = SIZE >> lod;

	pass = piglit_probe_rect_rgb(x, y, size, size, expected_color3f);

	if (!pass) {
		printf("2D array failed at size %d, layer %d\n",
		       size, layer);
	}

	return pass;
}

static GLboolean
test_texture(GLuint tex)
{
	int layer, lod;
	GLint tex_loc;
	GLboolean pass = GL_TRUE;
	glUseProgram(use_tex_program);
	glActiveTexture(GL_TEXTURE0);
	tex_loc = glGetUniformLocation(use_tex_program, "tex");
	glUniform1i(tex_loc, 0);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);

	for (lod = 0; lod < LOD; lod++) {
		for (layer = 0; layer < LAYERS; layer++) {
			draw_box(tex, layer, lod);
		}
	}

	for (lod = 0; lod < LOD; lod++) {
		for (layer = 0; layer < LAYERS; layer++) {
			pass = test_results(layer, lod) && pass;
		}
	}

	glUseProgram(0);
	return pass;
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

	/* Clear background to gray */
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

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

	piglit_present_results();

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

	fill_tex_program = piglit_build_simple_program(fill_tex_vs, fill_tex_fs);
	piglit_check_gl_error(GL_NO_ERROR);

	use_tex_program = piglit_build_simple_program(use_tex_vs, use_tex_fs);
	piglit_check_gl_error(GL_NO_ERROR);
}
