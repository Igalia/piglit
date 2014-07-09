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
 * \brief Test texturing from an ETC2 miptree of a real image.
 *
 * This test uses two data files for each etc2 format:
 * The file waffles-compressed-etc2-xxxx-64x32-mipmap.ktx contains a full
 * miptree in compressed ETC2 format of a 2D texture of waffles and fruit [1].
 * The base level size is 64x32 pixels.
 * The file waffles-decompressed-xxxx-64x32-mipmap.ktx contains a parallel
 * miptree in corresponding decompressed format. Each of its images was
 * obtained by decompressing the corresponding ETC2 image with etcpack [2].
 *
 * This test draws each miplevel i of the ETC2 texture such that the image's
 * lower left corner is at (x=0, y=sum(height of miplevel j for j=0 to i-1)),
 * and it draws each miplevel of the RGB texture to the right of its
 * corresponding ETC2 image. Then it compares that the images are identical.
 *
 * [1] The reference image is located at:
 * http://people.freedesktop.org/~chadversary/permalink/2012-07-09/1574cff2-d091-4421-a3cf-b56c7943d060.jpg.
 * [2] etcpack version 2.60 is the reference ETC2 compression tool, available at:
 * http://devtools.ericsson.com/etc.
 */

#include "piglit-util-gl.h"
#include "piglit_ktx.h"

static const int num_levels = 7;
static const int level0_width = 64;
static const int level0_height = 32;
static const int num_vertices = 4;
static const int window_width = 128;
static const int window_height = 64;

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
	                  "gles-3.0",
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
print_usage_and_exit(char *prog_name)
{
        printf("Usage: %s <format> %s\n"
               "  where <format> is one of:\n"
	       "    rgb8\n"
	       "    srgb8\n"
	       "    rgba8\n"
	       "    srgb8-alpha8\n"
	       "    r11\n"
	       "    rg11\n"
	       "    rgb8-punchthrough-alpha1\n"
	       "    srgb8-punchthrough-alpha1\n"
#if defined(PIGLIT_USE_OPENGL)
	       "  <profile> is one of:\n"
	       "    compat\n"
	       "    core\n", prog_name, "<profile>");
#elif defined(PIGLIT_USE_OPENGL_ES3)
	       ,prog_name, "");
#endif
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	static const char *compressed_filename;
	static const char *decompressed_filename;
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
		"precision mediump float;\n"
		"\n"
		"uniform sampler2D tex;\n"
		"varying vec2 tex_coord;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    gl_FragColor = texture2D(tex, tex_coord);\n"
		"}\n";

	/* Draw a square triangle fan. */
	const GLfloat vertices[8] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1,
	};

	GLint vertex_loc;
	GLuint vertex_buf;
	GLuint vao;

	if (!piglit_is_gles())
		piglit_require_extension("GL_ARB_ES3_compatibility");

	if (strcmp(argv[1], "rgb8") == 0) {
		compressed_filename =
			"waffles-compressed-etc2-rgb8-64x32-miptree.ktx";
		decompressed_filename =
			"waffles-decompressed-etc2-rgb8-64x32-miptree.ktx";
	} else if (strcmp(argv[1], "srgb8") == 0){
		compressed_filename =
			"waffles-compressed-etc2-srgb8-64x32-miptree.ktx";
		decompressed_filename =
			"waffles-decompressed-etc2-srgb8-64x32-miptree.ktx";
	} else if (strcmp(argv[1], "rgba8") == 0){
		compressed_filename =
			"waffles-compressed-etc2-rgba8-64x32-miptree.ktx";
		decompressed_filename =
			"waffles-decompressed-etc2-rgba8-64x32-miptree.ktx";
	} else if (strcmp(argv[1], "srgb8-alpha8") == 0){
		compressed_filename =
			"waffles-compressed-etc2-srgb8-alpha8-64x32-miptree.ktx";
		decompressed_filename =
			"waffles-decompressed-etc2-srgb8-alpha8-64x32-miptree.ktx";
	} else if (strcmp(argv[1], "r11") == 0){
		/* waffles-decompressed-etc2-r11-64x32-miptree.ktx contains
		 * per pixel RGBA data. But, glTexImage2D() in OpenGL ES 3.0
		 * doesn't allow internalFormat = GL_R8 with format= GL_RGBA.
		 * To workaround this issue use internalFormat = GL_RGBA and
		 * mask all the color channels except Red.
		 */
		glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
		compressed_filename =
			"waffles-compressed-etc2-r11-64x32-miptree.ktx";
		decompressed_filename =
			"waffles-decompressed-etc2-r11-64x32-miptree.ktx";
	} else if (strcmp(argv[1], "rg11") == 0){
		compressed_filename =
			"waffles-compressed-etc2-rg11-64x32-miptree.ktx";
		decompressed_filename =
			"waffles-decompressed-etc2-rg11-64x32-miptree.ktx";
	} else if (strcmp(argv[1], "rgb8-punchthrough-alpha1") == 0){
		compressed_filename =
			"waffles-compressed-etc2-rgb8-pt-alpha1-64x32-miptree.ktx";
		decompressed_filename =
			"waffles-decompressed-etc2-rgb8-pt-alpha1-64x32-miptree.ktx";
	} else if (strcmp(argv[1], "srgb8-punchthrough-alpha1") == 0){
		compressed_filename =
			"waffles-compressed-etc2-srgb8-pt-alpha1-64x32-miptree.ktx";
		decompressed_filename =
			"waffles-decompressed-etc2-srgb8-pt-alpha1-64x32-miptree.ktx";
	} else {
	        printf("Invalid format\n");
		print_usage_and_exit(argv[0]);
        }

	if((strcmp(argv[1], "rgba8") == 0) |
	   (strcmp(argv[1], "srgb8-alpha8") == 0) |
	   (strcmp(argv[1], "rgb8-punchthrough-alpha1") == 0) |
	   (strcmp(argv[1], "srgb8-punchthrough-alpha1") == 0))
	{
		/* Enable blending */
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	load_texture(compressed_filename, &compressed_tex);
	load_texture(decompressed_filename, &decompressed_tex);

	glClearColor(0.3, 0.5, 1.0, 1.0);

	prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);

	vertex_loc = glGetAttribLocation(prog, "vertex");
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
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

	glViewport(0, 0, window_width, window_height);

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

PIGLIT_GL_TEST_CONFIG_BEGIN
	bool test_compat = true;

#if defined(PIGLIT_USE_OPENGL)
	if (argc < 3)
		print_usage_and_exit(argv[0]);

	test_compat = strcmp(argv[2], "compat") == 0;

	if (!test_compat && strcmp(argv[2], "core") != 0)
		print_usage_and_exit(argv[0]);

#elif defined(PIGLIT_USE_OPENGL_ES3)
	if (argc < 2)
		print_usage_and_exit(argv[0]);
#endif

	if (test_compat)
		config.supports_gl_compat_version = 10;
	else
		config.supports_gl_core_version = 31;

	config.supports_gl_es_version = 30;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END
