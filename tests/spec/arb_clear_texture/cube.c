/*
 * Copyright (c) 2014 Intel Corporation
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
 */

/** @file cube.c
 *
 * A test of using glClearTexSubImage to clear faces of a cube
 * texture. Each face is cleared to a separate color and then all of
 * the faces are rendered and probed.
 */

#include "piglit-util-gl.h"

struct vertex {
	float pos;
	float tex_coord[3];
};

struct face {
	/* Color used for this face */
	float color[3];
	/* Texture coordinates needed to access this face */
	float tex_coord[3];

	GLenum target;
};

static const struct face
faces[] = {
	{ { 0.0f, 0.0f, 1.0f }, { +1.0f, 0.0f, 0.0f },
	  GL_TEXTURE_CUBE_MAP_POSITIVE_X },
	{ { 0.0f, 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f },
	  GL_TEXTURE_CUBE_MAP_NEGATIVE_X },
	{ { 0.0f, 1.0f, 1.0f }, { 0.0f, +1.0f, 0.0f },
	  GL_TEXTURE_CUBE_MAP_POSITIVE_Y },
	{ { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f },
	  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y },
	{ { 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, +1.0f },
	  GL_TEXTURE_CUBE_MAP_POSITIVE_Z },
	{ { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f },
	  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z },
};

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;
	config.supports_gl_es_version = 20;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint
create_texture(void)
{
	GLuint tex;
	int i;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (i = 0; i < 6; i++) {
		glTexImage2D(faces[i].target,
			     0, /* level */
			     GL_RGB,
			     1, 1, /* width/height */
			     0, /* border */
			     GL_RGB,
			     GL_UNSIGNED_BYTE,
			     NULL);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return tex;
}

static void
clear_texture(GLuint tex)
{
	int i;

	for (i = 0; i < 6; i++) {
		glClearTexSubImage(tex,
				   0, /* level */
				   0, 0, i, /* x/y/z */
				   1, 1, 1, /* width/height/depth */
				   GL_RGB,
				   GL_FLOAT,
				   faces[i].color);
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}

static void
init_program(void)
{
	GLuint prog;
	GLuint uniform;

	static const char vs_source[] =
		"attribute float piglit_vertex;\n"
		"attribute vec3 piglit_texcoord;\n"
		"uniform vec2 fb_size;\n"
		"varying vec3 tex_coord;\n"
		"\n"
		"void main()\n"
		"{\n"
		"        gl_Position = vec4(vec2(piglit_vertex, 0.5) * 2.0 /\n"
		"                           fb_size - 1.0,\n"
		"                           0.0, 1.0);\n"
		"        tex_coord = piglit_texcoord;\n"
		"}\n";
	static const char fs_source[] =
		"uniform samplerCube tex;\n"
		"varying vec3 tex_coord;\n"
		"\n"
		"void main()\n"
		"{\n"
		"        gl_FragColor = textureCube(tex, tex_coord);\n"
		"}\n";

	prog = piglit_build_simple_program(vs_source, fs_source);

	glUseProgram(prog);

	uniform = glGetUniformLocation(prog, "tex");
	glUniform1i(uniform, 0);

	uniform = glGetUniformLocation(prog, "fb_size");
	glUniform2f(uniform, piglit_width, piglit_height);
}

void
piglit_init(int argc, char **argv)
{
	/* glClearTexture is either in the GL_ARB_clear_texture
	 * extension or in core in GL 4.4
	 */
	if (piglit_get_gl_version() < 44 &&
	    !piglit_is_extension_supported("GL_ARB_clear_texture")) {
		printf("OpenGL 4.4 or GL_ARB_clear_texture is required.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	init_program();
}

static void
draw_faces(void)
{
	struct vertex vertices[6];
	int i;

	for (i = 0; i < 6; i++) {
		vertices[i].pos = i + 0.5f;
		memcpy(vertices[i].tex_coord,
		       faces[i].tex_coord,
		       sizeof faces[i].tex_coord);
	}

	glEnableVertexAttribArray(PIGLIT_ATTRIB_POS);
	glVertexAttribPointer(PIGLIT_ATTRIB_POS,
			      1, /* size */
			      GL_FLOAT,
			      GL_FALSE, /* normalized */
			      sizeof vertices[0],
			      &vertices[0].pos);
	glEnableVertexAttribArray(PIGLIT_ATTRIB_TEX);
	glVertexAttribPointer(PIGLIT_ATTRIB_TEX,
			      3, /* size */
			      GL_FLOAT,
			      GL_FALSE, /* normalized */
			      sizeof vertices[0],
			      vertices[0].tex_coord);

	glDrawArrays(GL_POINTS, 0, 6);

	glDisableVertexAttribArray(PIGLIT_ATTRIB_POS);
	glDisableVertexAttribArray(PIGLIT_ATTRIB_TEX);
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	GLuint tex;
	int i;

	tex = create_texture();

	clear_texture(tex);

	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

	draw_faces();

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glDeleteTextures(1, &tex);

	for (i = 0; i < 6; i++)
		pass &= piglit_probe_pixel_rgb(i, 0, faces[i].color);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
