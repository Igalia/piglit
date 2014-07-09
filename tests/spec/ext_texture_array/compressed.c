/*
 * Copyright © 2012 Intel Corporation
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
 * \file compressed.c
 *
 * Confirm that the functions glCompressedTexImage3D and
 * glCompressedTexSubImage3D work properly for 2D array textures.
 *
 * This test performs the following operations:
 *
 * - Create a 2D array texture with a width of 8 texture blocks, a
 *   height of 8 texture blocks, and a depth of 4.
 *
 * - If the test is operating in "teximage" mode, use a single call to
 *   glCompressedTexImage3D to upload a single array texture where
 *   each compressed block has a different grayscale value.
 *
 * - If the test is operating in "texsubimage" mode, use multiple
 *   calls to glCompressedTexSubImage3D to upload the texture in
 *   pieces.
 *
 * - Draw each layer of the texture to a separate region on the
 *   screen.
 *
 * - Verify that each portion of the drawn image matches the expected
 *   grayscale intensity.
 *
 * On GLES3, this test is performed using either ETC2 textures.  On
 * desktop GL, it is performed using S3TC textures.
 */

#include "piglit-util-gl.h"
#include "piglit-util-compressed-grays.h"

#ifdef PIGLIT_USE_OPENGL
#define GRAYSCALE_BLOCKS piglit_s3tc_grayscale_blocks
#define COMPRESSED_FORMAT GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#define BLOCK_WIDTH 4
#define BLOCK_HEIGHT 4
#define BLOCK_BYTES 8
#else // PIGLIT_USE_OPENGL_ES3
#define GRAYSCALE_BLOCKS piglit_etc1_grayscale_blocks
#define COMPRESSED_FORMAT GL_COMPRESSED_RGB8_ETC2
#define BLOCK_WIDTH 4
#define BLOCK_HEIGHT 4
#define BLOCK_BYTES 8
#endif

PIGLIT_GL_TEST_CONFIG_BEGIN

#ifdef PIGLIT_USE_OPENGL
	config.supports_gl_compat_version = 10;
#else // PIGLIT_USE_OPENGL_ES3
	config.supports_gl_es_version = 30;
#endif

	if (config.window_width < 4 * 8 * BLOCK_WIDTH)
		config.window_width = 4 * 8 * BLOCK_WIDTH;
	if (config.window_height < 8 * BLOCK_HEIGHT)
		config.window_height = 8 * BLOCK_HEIGHT;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_text[] =
#ifdef PIGLIT_USE_OPENGL
	"#version 120\n"
	"#define piglit_Vertex gl_Vertex\n"
	"#define piglit_MultiTexCoord0 gl_MultiTexCoord0\n"
	"#define piglit_in attribute\n"
	"#define piglit_out varying\n"
#else // PIGLIT_USE_OPENGL_ES3
	"#version 300 es\n"
	"#define piglit_in in\n"
	"#define piglit_out out\n"
	"piglit_in vec4 piglit_Vertex;\n"
	"piglit_in vec4 piglit_MultiTexCoord0;\n"
#endif
	"piglit_out vec3 texcoord;\n"
	"uniform mat4 proj;\n"
	"uniform int layer;\n"
	"void main()\n"
	"{\n"
	"  gl_Position = proj * piglit_Vertex;\n"
	"  texcoord = vec3(piglit_MultiTexCoord0.xy, float(layer));\n"
	"}\n";

static const char fs_text[] =
#ifdef PIGLIT_USE_OPENGL
	"#version 120\n"
	"#extension GL_EXT_texture_array : require\n"
	"#define piglit_FragColor gl_FragColor\n"
	"#define piglit_in varying\n"
	"#define piglit_texture2DArray texture2DArray\n"
#else // PIGLIT_USE_OPENGL_ES3
	"#version 300 es\n"
	"precision mediump float;\n"
	"precision mediump sampler2DArray;\n"
	"#define piglit_in in\n"
	"#define piglit_texture2DArray texture\n"
	"out vec4 piglit_FragColor;\n"
#endif
	"piglit_in vec3 texcoord;\n"
	"uniform sampler2DArray samp;\n"
	"void main()\n"
	"{\n"
	"  piglit_FragColor = piglit_texture2DArray(samp, texcoord);\n"
	"}\n";

static bool test_texsubimage;
static GLuint tex;
static GLuint prog;
static GLint proj_loc;
static GLint layer_loc;
static unsigned expected_gray_levels[8][8][4]; /* x, y, z */


static void
print_usage_and_exit(const char *prog_name)
{
	printf("Usage: %s <test_mode>\n"
	       "  where <test_mode> is one of the following:\n"
	       "    teximage: test glCompressedTexImage3D\n"
	       "    texsubimage: test glCompressedTexSubImage3D\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}


static void
compute_expected_gray_levels(unsigned width, unsigned height, unsigned depth,
			     unsigned xoffset, unsigned yoffset,
			     unsigned zoffset, unsigned gray_level)
{
	unsigned x, y, z;
	for (z = 0; z < depth; z++) {
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				expected_gray_levels
					[x + xoffset]
					[y + yoffset]
					[z + zoffset]
					= gray_level++;
			}
		}
	}
}


void
piglit_init(int argc, char **argv)
{
	/* Parse args */
	if (argc != 2)
		print_usage_and_exit(argv[0]);
	if (strcmp(argv[1], "teximage") == 0)
		test_texsubimage = false;
	else if (strcmp(argv[1], "texsubimage") == 0)
		test_texsubimage = true;
	else
		print_usage_and_exit(argv[0]);

	/* Make sure required GL features are present */
#ifdef PIGLIT_USE_OPENGL
	piglit_require_gl_version(21);
	piglit_require_extension("GL_ARB_texture_compression");
	piglit_require_extension("GL_EXT_texture_compression_s3tc");
	piglit_require_extension("GL_EXT_texture_array");
#endif

	/* We're using texture unit 0 for this entire test */
	glActiveTexture(GL_TEXTURE0);

	/* Create the texture */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* Upload the image */
	if (!test_texsubimage) {
		glCompressedTexImage3D(GL_TEXTURE_2D_ARRAY, 0,
				       COMPRESSED_FORMAT,
				       8 * BLOCK_WIDTH,
				       8 * BLOCK_HEIGHT,
				       4,
				       0 /* border */,
				       256 * BLOCK_BYTES,
				       GRAYSCALE_BLOCKS);
		compute_expected_gray_levels(8, 8, 4, 0, 0, 0, 0);
	} else {
		unsigned xoffset, yoffset, zoffset;
		unsigned gray_level = 0;
		glCompressedTexImage3D(GL_TEXTURE_2D_ARRAY, 0,
				       COMPRESSED_FORMAT,
				       8 * BLOCK_WIDTH,
				       8 * BLOCK_HEIGHT,
				       4,
				       0 /* border */,
				       8 * 8 * 4 * BLOCK_BYTES,
				       NULL);
		for (xoffset = 0; xoffset < 8; xoffset += 4) {
			for (yoffset = 0; yoffset < 8; yoffset += 4) {
				for (zoffset = 0; zoffset < 4; zoffset += 2) {
					glCompressedTexSubImage3D(
						GL_TEXTURE_2D_ARRAY, 0,
						xoffset * BLOCK_WIDTH,
						yoffset * BLOCK_HEIGHT,
						zoffset,
						4 * BLOCK_WIDTH,
						4 * BLOCK_HEIGHT,
						2,
						COMPRESSED_FORMAT,
						4 * 4 * 2 * BLOCK_BYTES,
						GRAYSCALE_BLOCKS[gray_level]);
					compute_expected_gray_levels(
						4, 4, 2,
						xoffset, yoffset, zoffset,
						gray_level);
					gray_level += 4 * 4 * 2;
				}
			}
		}
	}

	/* Create the shaders */
	prog = piglit_build_simple_program_unlinked(vs_text, fs_text);
	glBindAttribLocation(prog, PIGLIT_ATTRIB_POS, "piglit_Vertex");
	glBindAttribLocation(prog, PIGLIT_ATTRIB_TEX, "piglit_MultiTexCoord0");
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);
	proj_loc = glGetUniformLocation(prog, "proj");
	layer_loc = glGetUniformLocation(prog, "layer");
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}


bool
check_result(unsigned x, unsigned y, unsigned z)
{
	float f = expected_gray_levels[x][y][z] / 255.0;
	float expected[] = { f, f, f, 1.0 };
	return piglit_probe_rect_rgba((z * 8 + x) * BLOCK_WIDTH,
				      y * BLOCK_HEIGHT,
				      BLOCK_WIDTH,
				      BLOCK_HEIGHT,
				      expected);
}


enum piglit_result
piglit_display(void)
{
	unsigned x, y, z;
	bool pass = true;

	/* Draw each texture level */
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(prog);
	piglit_ortho_uniform(proj_loc, piglit_width, piglit_height);
	for (z = 0; z < 4; z++) {
		glUniform1i(layer_loc, z);
		piglit_draw_rect_tex(z * 8 * BLOCK_WIDTH, 0,
				     8 * BLOCK_WIDTH, 8 * BLOCK_HEIGHT,
				     0, 0, 1, 1);
	}
	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	/* Check results */
	for (z = 0; z < 4; z++) {
		for (y = 0; y < 8; y++) {
			for (x = 0; x < 8; x++) {
				pass = check_result(x, y, z) && pass;
			}
		}
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
