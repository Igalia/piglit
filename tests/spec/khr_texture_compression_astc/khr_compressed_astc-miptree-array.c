/*
 * Copyright 2015 Intel Corporation
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
 * \brief Test texturing from an ASTC miptree of a real image.
 *
 * This is an adaptation of the khr_texture_compression_astc-miptree test.
 *
 * This test draws miplevels of the compressed textures in a 2D array
 * according to the MIPLAYOUT_BELOW organization scheme. Each miplevel of
 * both images are compared for equality after each level is drawn.
 */

#include "piglit-util-gl.h"
#include "piglit_ktx.h"

#define num_levels 8
#define level0_width 160
#define level0_height 106

#define num_vertices 4

static GLuint prog;

static struct piglit_gl_test_config *piglit_config;

enum piglit_result
test_miptrees(void* odd);

PIGLIT_GL_TEST_CONFIG_BEGIN

	piglit_config = &config;
	config.supports_gl_compat_version = 11;
	config.supports_gl_es_version = 31;

	config.window_width = 2 * level0_width;
	config.window_height = level0_height + (level0_height >> 1);
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	static bool is_odd[2] = {true, false};

	config.subtests = (struct piglit_subtest[]) {
		{
			"5x5 Block Dim",
			"odd",
			test_miptrees,
			&is_odd[0]
		},
		{
			"12x12 Block Dim",
			"even",
			test_miptrees,
			&is_odd[1]
		},
		{NULL},
	};

PIGLIT_GL_TEST_CONFIG_END

/**
 * The \a filename is relative to the current test's source directory.
 *
 * A new texture is created and returned in \a tex_name.
 */
static void
load_texture(const char *dir1, const char *dir2,
	const char *filename, GLuint *tex_name)
{
	struct piglit_ktx *ktx;
	const struct piglit_ktx_info *info;
	char filepath[4096];
	bool ok = true;

	piglit_join_paths(filepath, sizeof(filepath), 7,
	                  piglit_source_dir(),
	                  "tests",
	                  "spec",
	                  "khr_texture_compression_astc",
	                  dir1,
	                  dir2,
	                  filename);

	ktx = piglit_ktx_read_file(filepath);
	if (ktx == NULL)
		piglit_report_result(PIGLIT_FAIL);


	info = piglit_ktx_get_info(ktx);
	assert(info->num_miplevels == num_levels);
	assert((info->target == GL_TEXTURE_2D_ARRAY) || (info->target == GL_TEXTURE_2D));
	assert(info->pixel_width == level0_width);
	assert(info->pixel_height== level0_height);

	*tex_name = 0;
	ok = piglit_ktx_load_texture(ktx, tex_name, NULL);
	if (!ok)
		piglit_report_result(PIGLIT_FAIL);

	piglit_ktx_destroy(ktx);
}

/** Compares the compresseed texture against the decompressed texture */
bool draw_compare_levels(GLint index,
			GLint level_pixel_size_loc, GLint pixel_offset_loc,
			GLuint compressed_tex)
{
	unsigned y = 0;
	unsigned x = 0;
	bool pass = true;
	int level = 0;

	glBindTexture(GL_TEXTURE_2D_ARRAY, compressed_tex);

	for (; level < num_levels; ++level) {
		int w = level0_width >> level;
		int h = level0_height >> level;
		glUniform2f(level_pixel_size_loc, (float) w, (float) h);

		/* Draw miplevel of compressed texture 1. */
		glUniform2f(pixel_offset_loc, x, y);
		glUniform1i(index, 0);
		glDrawArrays(GL_TRIANGLE_FAN, 0, num_vertices);

		/* Draw miplevel of compressed texture 2. */
		glUniform2f(pixel_offset_loc, level0_width + x, y);
		glUniform1i(index, 1);
		glDrawArrays(GL_TRIANGLE_FAN, 0, num_vertices);

		if (pass) {
			pass = piglit_probe_rects_equal(x, y,
						level0_width + x, y,
						 w, h, GL_RGBA);

			if (!pass)
				piglit_loge("Miplevel %d", level);
		}

		/* Update the next miplevel arrangement */
		if (level == 1)
			x += w;
		else
			y += h;
	}

	piglit_present_results();
	return pass;
}

enum piglit_result
test_miptrees(void* odd)
{
	int subtest =  0;
	int block_dims = *(bool*)odd;

	/* Texture objects. */
	GLuint compressed_tex;

	static const char * tests[3] = {"hdr", "ldrs", "ldrl"};
	static const char * block_dim_str[2] = {"12x12", "5x5"};


	GLint pixel_offset_loc = glGetUniformLocation(prog, "pixel_offset");
	GLint level_pixel_size_loc = glGetUniformLocation(prog,
							"level_pixel_size");
	GLint index = glGetUniformLocation(prog, "index");

	/* Generate filename for compressed texture */
	char cur_file[50];
	snprintf(cur_file, sizeof(cur_file), "array/waffles-%s.ktx",
	       block_dim_str[block_dims]);

	/* Test each submode */
	for (; subtest < ARRAY_SIZE(tests); ++subtest) {

		/* Load texture for current submode and block size */
		load_texture("compressed", tests[subtest], cur_file,
		    &compressed_tex);

		/* Draw and compare each level of the two textures */
		glClear(GL_COLOR_BUFFER_BIT);
		if (!draw_compare_levels(index, level_pixel_size_loc,
		    pixel_offset_loc, compressed_tex)) {
			piglit_loge("Mode %s Block %s.", tests[subtest],
			            block_dim_str[block_dims]);
			return PIGLIT_FAIL;
		}
	}
	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	static const char vs_source[] =
		"#version 300 es\n"
		"\n"
		"uniform vec2 window_pixel_size;\n"
		"uniform vec2 level_pixel_size;\n"
		"uniform vec2 pixel_offset;\n"
		"\n"
		"// vertex is some corner of the unit square [0,1]^2 \n"
		"in vec2 vertex;\n"
		"out vec2 tex_coord;\n"
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

	static const char fs_source[] =
		"#version 300 es\n"
		"precision highp float;\n"
		"\n"
		"uniform highp sampler2DArray tex;\n"
		"uniform int index;\n"
		"in vec2 tex_coord;\n"
		"out vec4 fragment_color;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    vec4 t = texture(tex, vec3(tex_coord.x, tex_coord.y, index));\n"
		"    fragment_color = vec4(t.rgb, 1.0);\n"
		"}\n";

	/* Vertices to draw a square triangle strip. */
	static const GLfloat vertices[2 * num_vertices] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1,
	};

	GLint vertex_loc;
	GLuint vertex_buf;
	GLuint vao;

	piglit_require_extension("GL_KHR_texture_compression_astc_ldr");

	if (!piglit_is_gles())
		piglit_require_extension("GL_ARB_ES3_compatibility");

	glClearColor(0.9098, 0.8314, 0.7843, 1.0);
	glViewport(0, 0, piglit_width, piglit_height);

	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	prog = piglit_build_simple_program(vs_source, fs_source);
	glReleaseShaderCompiler();
	glUseProgram(prog);

	vertex_loc = glGetAttribLocation(prog, "vertex");
	glEnableVertexAttribArray(vertex_loc);
	glVertexAttribPointer(vertex_loc, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
	             GL_STATIC_DRAW);

	glUniform1i(glGetUniformLocation(prog, "tex"), 0);
	glUniform2f(glGetUniformLocation(prog, "window_pixel_size"),
	            piglit_width, piglit_height);
}

enum piglit_result
piglit_display(void)
{
	return piglit_run_selected_subtests(piglit_config->subtests,
				      piglit_config->selected_subtests,
				      piglit_config->num_selected_subtests,
				      PIGLIT_SKIP);
}
