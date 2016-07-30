/*
 * Copyright 2016 Intel Corporation
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
 * The files under compressed/3D/{hdr, ldrl, ldrs} contain full miptrees, in
 * the GL_*_ASTC_* formats, of a 3D texture of waffles and fruit [1]. The base
 * level size was shrunken to 160x106 pixels and used to create a 3D texture
 * with depth=8. The files under the decompressed/3D/{hdr, ldrl, ldrs} directory
 * contain the same miptree in GL_RGBA format. Each miplevel was obtained by
 * decompressing the corresponding ASTC texture with astcenc [2].
 *
 * This test draws miplevels of the compressed textures in a space-efficient
 * manner. It does the same when drawing the decompressed texture on the right.
 * Each miplevel of both images are compared for equality after being drawn.
 *
 * [1] The reference image is located at:
 *     http://people.freedesktop.org/~chadversary/permalink/2012-07-09/1574cff2-d091-4421-a3cf-b56c7943d060.jpg.
 * [2] astcenc is the reference ASTC compression tool, available at:
 *     http://malideveloper.arm.com/develop-for-mali/tools/software-tools/astc-evaluation-codec/.
 */

#include "piglit-util-gl.h"
#include "piglit_ktx.h"

#define NUM_LEVELS 3
#define LEVEL0_WIDTH 160
#define LEVEL0_HEIGHT 106
#define LEVEL0_DEPTH 8

#define NUM_VERTICES 4

static GLuint prog;

static struct piglit_gl_test_config *piglit_config;

enum test_type
{
	TEST_TYPE_HDR,
	TEST_TYPE_LDR,
	TEST_TYPE_SRGB,
};

enum piglit_result
test_miptrees(void* input_type);

static enum test_type ldr_test  = TEST_TYPE_LDR;
static enum test_type hdr_test  = TEST_TYPE_HDR;
static enum test_type srgb_test = TEST_TYPE_SRGB;
static const struct piglit_subtest subtests[] = {
	{
		"LDR Profile",
		"ldr",
		test_miptrees,
		&ldr_test,
	},
	{
		"HDR Profile",
		"hdr",
		test_miptrees,
		&hdr_test,
	},
	{
		"sRGB decode",
		"srgb",
		test_miptrees,
		&srgb_test,
	},
	{NULL},
};

PIGLIT_GL_TEST_CONFIG_BEGIN

	piglit_config = &config;
	config.supports_gl_compat_version = 11;
	config.supports_gl_es_version = 30;

	config.window_width = 2 * LEVEL0_WIDTH;
	config.window_height = LEVEL0_HEIGHT + (LEVEL0_HEIGHT >> 1);
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

	config.subtests = subtests;

PIGLIT_GL_TEST_CONFIG_END

static const float green[3] = {0.0, 1.0, 0.0};
static const float red[3] = {1.0, 0.0, 0.0};
static const float black[3] = {0.0, 0.0, 0.0};

/**
 * The \a filename is relative to the current test's source directory.
 *
 * A new texture is created and returned in \a tex_name.
 */
static void
load_texture(const char *dir1, const char *dir2,
	     const char *block_dim_str, GLuint *tex_name)
{
	struct piglit_ktx *ktx;
	const struct piglit_ktx_info *info;
	char filepath[4096];
	char cur_file[20];
	bool ok = true;

	/* Generate filename for compressed texture */
	snprintf(cur_file, sizeof(cur_file), "waffles-%s.ktx",
				block_dim_str);

	piglit_join_paths(filepath, sizeof(filepath), 7,
	                  piglit_source_dir(),
	                  "tests",
	                  "spec",
	                  "oes_texture_compression_astc",
	                  dir1,
	                  dir2,
	                  cur_file);

	ktx = piglit_ktx_read_file(filepath);
	if (ktx == NULL)
		piglit_report_result(PIGLIT_FAIL);

	info = piglit_ktx_get_info(ktx);
	assert(info->num_miplevels == NUM_LEVELS);
	assert(info->target == GL_TEXTURE_3D);
	assert(info->pixel_width == LEVEL0_WIDTH);
	assert(info->pixel_height == LEVEL0_HEIGHT);
	assert(info->pixel_depth == LEVEL0_DEPTH);

	*tex_name = 0;
	ok = piglit_ktx_load_texture(ktx, tex_name, NULL);
	if (!ok)
		piglit_report_result(PIGLIT_FAIL);

	piglit_ktx_destroy(ktx);
}

/** Compares the compressed texture against the decompressed texture */
bool draw_compare_levels(bool check_error,
			 GLint level_pixel_size_loc, GLint pixel_offset_loc,
			 GLint slice_loc, GLint depth_loc, GLint slice,
			 GLuint compressed_tex, GLuint decompressed_tex)
{
	/* Fully-saturated magenta */
	static const float error_color[4] = {1.0, 0.0, 1.0, 1.0};

	unsigned y = 0;
	unsigned x = 0;
	bool result = true;
	int level = 0;

	for (; level < NUM_LEVELS; ++level) {
		int w = LEVEL0_WIDTH >> level;
		int h = LEVEL0_HEIGHT >> level;
		int d = LEVEL0_DEPTH >> level;
		bool pass = true;
		glUniform2f(level_pixel_size_loc, (float) w, (float) h);
		glUniform1f(slice_loc, slice);
		glUniform1f(depth_loc, d);

		/* Draw miplevel of compressed texture. */
		glBindTexture(GL_TEXTURE_3D, compressed_tex);
		glUniform2f(pixel_offset_loc, x, y);
		glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_VERTICES);

		/* Check the textures (or error-colors) for equivalence. */
		if (check_error) {
			pass = piglit_probe_rect_rgba(x, y, w, h,
						      error_color);
		} else {
			/* Draw miplevel of decompressed texture. */
			glBindTexture(GL_TEXTURE_3D, decompressed_tex);
			glUniform2f(pixel_offset_loc, LEVEL0_WIDTH + x, y);
			glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_VERTICES);

			pass = piglit_probe_rects_equal(x, y,
						LEVEL0_WIDTH + x, y,
						w, h, GL_RGBA);
		}

		if (!pass) {
			piglit_loge("Slice: %d, Miplevel: %d",
				    slice, level);
			result = false;
		}

		/* Update the next miplevel arrangement */
		if (level == 1)
			x += w;
		else
			y += h;
	}

	/* Delete bound textures */
	glDeleteTextures(1, &compressed_tex);
	glDeleteTextures(1, &decompressed_tex);

	piglit_present_results();
	return result;
}

enum piglit_result
test_miptrees(void* input_type)
{
	GLint slice_loc, depth_loc, pixel_offset_loc, level_pixel_size_loc;
	const enum test_type subtest = *(enum test_type*) input_type;
	const bool is_srgb_test = subtest == TEST_TYPE_SRGB;
	const bool is_hdr_test  = subtest == TEST_TYPE_HDR;

	static const char * tests[3] = {"hdr", "ldrl", "ldrs"};
	static const char * block_dim_str[] = {
		"3x3x3",
		"4x3x3",
		"4x4x3",
		"4x4x4",
		"5x4x4",
		"5x5x4",
		"5x5x5",
		"6x5x5",
		"6x6x5",
		"6x6x6"
	};

	pixel_offset_loc = glGetUniformLocation(prog, "pixel_offset");
	level_pixel_size_loc = glGetUniformLocation(prog, "level_pixel_size");
	slice_loc = glGetUniformLocation(prog, "slice");
	depth_loc = glGetUniformLocation(prog, "depth");

	/*  Check for error color if an LDR-only sys reading an HDR
	 *  texture. No need to draw a reference mipmap in this case.
	 */
	const bool has_hdr = piglit_is_extension_supported(
		"GL_KHR_texture_compression_astc_hdr");
	const bool check_error = is_hdr_test && !has_hdr;
	int block_dims = 0, slice;;

	if (is_srgb_test)
		/* Loosen up the tolerence for sRGB tests. This will allow testing
		 * sRGB formats which have known precision issues in void extent
		 * blocks. See khronos bug#11294 for details.
		 */
		piglit_set_tolerance_for_bits(7, 7, 7, 7);
	else
		piglit_set_tolerance_for_bits(8, 8, 8, 8);

	for ( ; block_dims < ARRAY_SIZE(block_dim_str); ++block_dims) {
		for (slice = 0; slice < LEVEL0_DEPTH; slice++) {

			/* Texture objects. */
			GLuint tex_compressed = 0;
			GLuint tex_decompressed = 0;

			/* Load texture for current submode and block size */
			load_texture("compressed/3D", tests[subtest],
				     block_dim_str[block_dims],
				     &tex_compressed);
			if (!check_error) {
				load_texture("decompressed/3D", tests[subtest],
					     block_dim_str[block_dims],
					     &tex_decompressed);
			}

			/* Draw and compare each level of the two textures */
			glClear(GL_COLOR_BUFFER_BIT);
			if (!draw_compare_levels(check_error,
						 level_pixel_size_loc,
						 pixel_offset_loc,
						 slice_loc, depth_loc,
						 slice,
						 tex_compressed,
						 tex_decompressed)) {
				piglit_loge("Mode: %s Block: %s.",
					    tests[subtest],
					    block_dim_str[block_dims]);
				return PIGLIT_FAIL;
			}
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
		"uniform highp sampler3D tex;\n"
		"uniform float slice;\n"
		"uniform float depth;\n"
		"in vec2 tex_coord;\n"
		"out vec4 fragment_color;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    vec4 t = texture(tex, vec3(tex_coord.x, tex_coord.y,\n"
		"                     slice / (depth - 1.0)));\n"
		"    fragment_color = vec4(t.rgb, 1.0);\n"
		"}\n";

	/* Vertices to draw a square triangle strip. */
	static const GLfloat vertices[2 * NUM_VERTICES] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1,
	};

	GLint vertex_loc;
	GLuint vertex_buf;
	GLuint vao;

	piglit_require_extension("GL_OES_texture_compression_astc");

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
