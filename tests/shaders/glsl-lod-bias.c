/*
 * Copyright © 2009 Intel Corporation
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

// author: Ben Holmes

/* This test utilizes a texture sampling function in GLSL that specifies a
 * LOD bias.  Create a texture with a 4x4 checkerboard pattern.  Draw that
 * texture with all of the positive LOD biases that will result in a mipmap
 * level greater than or equal to 4x4 (single texel tiles) being used.  Verify
 * that all the image are the same.
 */

#include "piglit-util-gl.h"

/* Pick the number of LODs to examine and the size of the texture so that the
 * smallest LOD is the one where each of the 4x4 tiles in the checkerboard
 * texture is 1x1.
 */
#define TEST_COLS 5
#define BOX_SIZE  (1 << (TEST_COLS + 1))

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = (BOX_SIZE+2)*TEST_COLS+1;
	config.window_height = (BOX_SIZE+1)+1;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint tex[1];
static GLint prog;
static GLint vs;
static GLint fs;
static GLint tex_loc;
static GLint bias_loc;


static void loadTex(void);
static void compileLinkProg(void);

static const char *vertShaderText =
	"varying vec2 texCoords;\n"
	"void main()\n"
	"{ \n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"	texCoords = gl_MultiTexCoord0.st;\n"
	"} \n";

static const char *fragShaderText =
	"uniform sampler2D tex2d;\n"
	"uniform float lodBias;\n"
	"varying vec2 texCoords;\n"
	"void main()\n"
	"{ \n"
	"	gl_FragColor = texture2D(tex2d, texCoords, lodBias);\n"
	"} \n";


static const float clear_color[4] = {0.6, 0.6, 0.6, 1.0};
static const float green[4]       = {0.0, 1.0, 0.0, 1.0};
static const float pink[4]        = {1.0, 0.0, 1.0, 0.0}; /* Note: 0.0 alpha */

void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(20);

	loadTex();
	compileLinkProg();

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	glEnable(GL_TEXTURE_2D);
	glClearColor(clear_color[0], clear_color[1],
		     clear_color[2], clear_color[3]);
}

static void
compileLinkProg(void)
{
	GLint stat;

	vs = glCreateShader(GL_VERTEX_SHADER);
	fs = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vs, 1, (const GLchar **) &vertShaderText, NULL);
	glShaderSource(fs, 1, (const GLchar **) &fragShaderText, NULL);

	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &stat);
	if (!stat) {
                printf("error compiling vertex shader!\n");
                exit(1);
        }

	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &stat);
	if (!stat) {
		printf("error compiling fragment shader!\n");
		exit(1);
	}


	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);
	glUseProgram(prog);

	tex_loc = glGetUniformLocation(prog, "tex2d");
	bias_loc = glGetUniformLocation(prog, "lodBias");

	glUniform1i(tex_loc, 0);
}

static void
loadTex(void)
{
	#define height BOX_SIZE
	#define width BOX_SIZE
	int i, j;

	GLfloat texData[width][height][4];
	for (i=0; i < width; ++i) {
		for (j=0; j < height; ++j) {
			if ((i ^ j) & (BOX_SIZE / 4)) {
				memcpy(texData[i][j], pink, sizeof(pink));
			}
			else {
				memcpy(texData[i][j], green, sizeof(green));
			}
		}
	}

	glGenTextures(1, tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
			GL_RGBA, GL_FLOAT, texData);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	#undef height
	#undef width
}


enum piglit_result
piglit_display(void)
{
	const int tile_size = BOX_SIZE / 4;
	GLboolean pass = GL_TRUE;
	unsigned i;

	glClear(GL_COLOR_BUFFER_BIT);

	for (i = 0; i < TEST_COLS; i++) { 
		const int x = 1 + ((BOX_SIZE + 2) * i);

		glUniform1f(bias_loc, (float) i);

		/* Draw the rectangle the same size as the texture.  This
		 * guarantees that the unbiased LOD will be 0.0.
		 */
		piglit_draw_rect_tex((float) x, 1.0,
				     BOX_SIZE, BOX_SIZE,
				     0.0, 0.0, 1.0, 1.0);

		/* The middle of the lower left tile should be green, and the
		 * middle of the tile next to it should be the clear color.
		 */
		if (!piglit_probe_pixel_rgb(x + (3 * tile_size / 2), 2, clear_color)
		    || !piglit_probe_pixel_rgb(x + (tile_size / 2), 2, green)) {
			pass = GL_FALSE;
		}
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
