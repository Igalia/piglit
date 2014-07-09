/*
 * Copyright 2012 Intel Corporation
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
 * \file
 * \brief Test texturing from an ETC1 miptree of a real image.
 *
 * This test uses two data files. The file waffles-compressed-etc1-64x32.ktx
 * contains a full miptree in GL_ETC1_RGB8_OES format of a 2D texture of
 * waffles and fruit [1].  The base level size is 64x32 pixels. The file
 * waffles-decompressed-rgb-64x32.ktx contains a parallel miptree in GL_RGB
 * format. Each of its RGB images was obtained by decompressing the corresponding
 * ETC1 image with etcpack [2].
 *
 * This test draws each miplevel i of the ETC1 texture such that the image's
 * lower left corner is at (x=0, y=sum(height of miplevel j for j=0 to i-1)),
 * and it draws each miplevel of the RGB texture to the right of its
 * corresponding ETC1 image. Then it compares that the images are identical.
 *
 * [1] The reference image is located at http://people.freedesktop.org/~chadversary/permalink/2012-07-09/1574cff2-d091-4421-a3cf-b56c7943d060.jpg.
 * [2] etcpack is the reference ETC1 compression tool, available at http://devtools.ericsson.com/etc.
 */

#include "piglit-util-gl.h"
#include "piglit_ktx.h"

#define num_levels 7
#define level0_width 64
#define level0_height 32

#define num_vertices 4

static const int window_width = 2 * level0_width;
static const int window_height = 2 * level0_height;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 20;

	config.window_width = window_width;
	config.window_height = window_height;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END


static GLuint prog;

/* Texture objects. */
static GLuint compressed_tex;
static GLuint decompressed_tex;

/**
 * The \a filename is relative to the current test's source directory.
 *
 * A new texture is created and returned in \a tex_name.
 */
static void
load_texture(const char *filename, GLuint *tex_name)
{
	struct piglit_ktx *ktx;
	const struct piglit_ktx_info *info;
	char filepath[4096];
	bool ok = true;

	piglit_join_paths(filepath, sizeof(filepath), 5,
	                  piglit_source_dir(),
	                  "tests",
	                  "spec",
	                  "oes_compressed_etc1_rgb8_texture",
	                  filename);

	ktx = piglit_ktx_read_file(filepath);
	if (ktx == NULL)
		piglit_report_result(PIGLIT_FAIL);

	info = piglit_ktx_get_info(ktx);
	assert(info->num_miplevels == num_levels);
	assert(info->target == GL_TEXTURE_2D);
	assert(info->pixel_width == level0_width);
	assert(info->pixel_height== level0_height);

	*tex_name = 0;
	ok = piglit_ktx_load_texture(ktx, tex_name, NULL);
	if (!ok)
		piglit_report_result(PIGLIT_FAIL);

	piglit_ktx_destroy(ktx);
}

void
piglit_init(int argc, char **argv)
{
	static const char compressed_filename[] = "waffles-compressed-etc1-64x32-miptree.ktx";
	static const char decompressed_filename[] = "waffles-decompressed-rgb-64x32-miptree.ktx";

	const char vs_source[] =
		"#version 100\n"
		"\n"
		"uniform vec2 window_pixel_size;\n"
		"uniform vec2 level_pixel_size;\n"
		"uniform vec2 pixel_offset;\n"
		"\n"
		"// vertex is some corner of the unit square [0,1]^2 \n"
		"attribute vec2 vertex;\n"
		"varying vec2 tex_coord;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    vec2 pos = vertex;\n"
		"    pos *= level_pixel_size;\n"
		"    pos += pixel_offset;\n"
		"    pos /= 0.5 * window_pixel_size;\n"
		"    pos -= vec2(1, 1);\n"
		"    gl_Position = vec4(pos.xy, 0.0, 1.0);\n"
		"\n"
		"    tex_coord = vertex;\n"
		"}\n";

	const char fs_source[] =
		"#version 100\n"
		"precision highp float;\n"
		"\n"
		"uniform sampler2D tex;\n"
		"varying vec2 tex_coord;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    vec4 t = texture2D(tex, tex_coord);\n"
		"    gl_FragColor = vec4(t.rgb, 1.0);\n"
		"}\n";

	/* Draw a square triangle strip. */
	const GLfloat vertices[2 * num_vertices] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1,
	};

	GLint vertex_loc;
	GLuint vertex_buf;

	piglit_require_extension("GL_OES_compressed_ETC1_RGB8_texture");

	load_texture(compressed_filename, &compressed_tex);
	load_texture(decompressed_filename, &decompressed_tex);

	glClearColor(1.0, 0.0, 0.0, 1.0);
	glViewport(0, 0, window_width, window_height);

	prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);

	vertex_loc = glGetAttribLocation(prog, "vertex");
	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glEnableVertexAttribArray(vertex_loc);
	glVertexAttribPointer(vertex_loc, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
	             GL_STATIC_DRAW);

	glUniform1i(glGetUniformLocation(prog, "tex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glUniform2f(glGetUniformLocation(prog, "window_pixel_size"),
	            window_width, window_height);
}

static void
minify(int *x)
{
	assert(*x > 0);

	if (*x > 1)
		*x >>= 1;
}

enum piglit_result
piglit_display(void)
{
	GLint pixel_offset_loc = glGetUniformLocation(prog, "pixel_offset");
	GLint level_pixel_size_loc = glGetUniformLocation(prog, "level_pixel_size");

	int level = 0;
	int level_width = level0_width;
	int level_height = level0_height;
	int y_offset = 0;

	bool pass = true;

	glClear(GL_COLOR_BUFFER_BIT);

	for (level = 0; level < num_levels; ++level) {
		glUniform2f(level_pixel_size_loc,
		            (float) level_width,
		            (float) level_height);

		/* Draw miplevel of compressed texture. */
		glBindTexture(GL_TEXTURE_2D, compressed_tex);
		glUniform2f(pixel_offset_loc, 0, y_offset);
		glDrawArrays(GL_TRIANGLE_FAN, 0, num_vertices);

		/* Draw miplevel of decompressed texture. */
		glBindTexture(GL_TEXTURE_2D, decompressed_tex);
		glUniform2f(pixel_offset_loc, level0_width, y_offset);
		glDrawArrays(GL_TRIANGLE_FAN, 0, num_vertices);

		y_offset += level_height;
		minify(&level_width);
		minify(&level_height);
	}

	pass = piglit_probe_rect_halves_equal_rgba(0, 0, window_width, window_height);
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
