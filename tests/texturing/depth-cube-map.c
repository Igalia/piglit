/*
 * Copyright © 2011 Intel Corporation
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

/*
 * file depth-cube-map.c
 * Test to verify cubemap depth texture support in GL version >= 3.0
 *
 * author: Anuj Phogat
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 400;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint tex;
static GLint prog;
static GLint fs;
static GLint vs;

static GLfloat vertices[12] =  {150.0, 125.0, 0.0,
				150.0, 175.0, 0.0,
				100.0, 125.0, 0.0,
				100.0, 175.0, 0.0};

static GLuint elements[4] = {0, 1, 2, 3};

static const char *vertShaderText =
	"attribute vec3 textureCoords;\n"
	"void main()\n"
	"{\n"
	" gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	" gl_TexCoord[0] = gl_MultiTexCoord0;\n"
	"}\n";

static const char *fragShaderText =
	"uniform samplerCube depthcubeTex;\n"
	"void main()\n"
	"{\n"
	" vec4 depthcolor  = textureCube(depthcubeTex, gl_TexCoord[0].xyz);\n"
	" gl_FragColor = vec4(depthcolor.xyz, 1.0);\n"
	"}\n";

static void
shaderSetup(void)
{
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vertShaderText);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fragShaderText);
	prog = piglit_link_simple_program(vs, fs);
	glUseProgram(prog);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}


static void
loadTex(void)
{
	#define height 2
	#define width 2
	int i, j;

	GLfloat texDepthDataPosX[width][height];
	GLfloat texDepthDataNegX[width][height];
	GLfloat texDepthDataPosY[width][height];
	GLfloat texDepthDataNegY[width][height];
	GLfloat texDepthDataPosZ[width][height];
	GLfloat texDepthDataNegZ[width][height];

	/* Set the cubemap texture data */
	for (i=0; i < height; ++i) {
		for (j = 0 ; j < width; ++j) {
			texDepthDataPosX[i][j] = 0.0;
			texDepthDataNegX[i][j] = 0.2;
			texDepthDataPosY[i][j] = 0.35;
			texDepthDataNegY[i][j] = 0.50;
			texDepthDataPosZ[i][j] = 0.75;
			texDepthDataNegZ[i][j] = 1.0;
		}
	}

	/* Render the cube depth texture using LUMINANCE */
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

	glTexParameteri(GL_TEXTURE_CUBE_MAP,
			GL_GENERATE_MIPMAP,
			GL_FALSE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_MIN_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_WRAP_S,
			GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_WRAP_T,
			GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_WRAP_R,
			GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP,
			GL_DEPTH_TEXTURE_MODE,
			GL_LUMINANCE);
	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_COMPARE_MODE,
			GL_NONE);

	/* Set a different color texture to each face of cubemap */
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_DEPTH_COMPONENT,
		     width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
		     texDepthDataPosX);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_DEPTH_COMPONENT,
		     width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
		     texDepthDataNegX);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_DEPTH_COMPONENT,
		     width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
		     texDepthDataPosY);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_DEPTH_COMPONENT,
		     width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
		     texDepthDataNegY);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_DEPTH_COMPONENT,
		     width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
		     texDepthDataPosZ);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_DEPTH_COMPONENT,
		     width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
		     texDepthDataNegZ);

	#undef height
	#undef width
}

void
piglit_init(int argc, char **argv)
{
	/* Check if EXT_gpu_shader4 is supported */
	if (!piglit_is_extension_supported("GL_EXT_gpu_shader4"))
		/* If EXT_gpu_shader4 is not supported GL version must be 3.0 */
		piglit_require_gl_version(30);
	loadTex();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, piglit_width, 0, piglit_height, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glClearColor(0.1, 0.1, 0.1, 1.0);
	shaderSetup();
}

enum piglit_result
piglit_display(void)
{
	GLint loc1;
	GLboolean pass = GL_TRUE;

	GLfloat ColorPosX[3] = {0.0, 0.0, 0.0};
	GLfloat ColorNegX[3] = {0.2, 0.2, 0.2};
	GLfloat ColorPosY[3] = {0.35, 0.35, 0.35};
	GLfloat ColorNegY[3] = {0.5, 0.5, 0.5};
	GLfloat ColorPosZ[3] = {0.75, 0.75, 0.75};
	GLfloat ColorNegZ[3] = {1.0, 1.0, 1.0};

	loc1 = glGetUniformLocation(prog, "depthcubeTex");
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

	/* Apply each face of cubemap as a texture to a polygon */
	glUniform1i(loc1, 0);
	glTexCoordPointer(3, GL_FLOAT, 0, cube_face_texcoords[0]); /* +X */
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, elements);

	glPushMatrix();
	glTranslatef(75.0, 0.0, 0.0);
	glTexCoordPointer(3, GL_FLOAT, 0, cube_face_texcoords[2]); /* +Y */
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, elements);

	glTranslatef(75.0, 0.0, 0.0);
	glTexCoordPointer(3, GL_FLOAT, 0, cube_face_texcoords[4]); /* +Z */
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, elements);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, 75.0, 0.0);
	glTexCoordPointer(3, GL_FLOAT, 0, cube_face_texcoords[1]); /* -X */
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, elements);

	glTranslatef(75.0, 0.0, 0.0);
	glTexCoordPointer(3, GL_FLOAT, 0, cube_face_texcoords[3]); /* -Y */
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, elements);

	glTranslatef(75.0, 0.0, 0.0);
	glTexCoordPointer(3, GL_FLOAT, 0, cube_face_texcoords[5]); /* -Z */
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, elements);
	glPopMatrix();

	/* Test the pixel color of polygons against the expected output */
	pass = piglit_probe_pixel_rgb(110, 135, ColorPosX);
	pass = piglit_probe_pixel_rgb(185, 135, ColorPosY) && pass;
	pass = piglit_probe_pixel_rgb(260, 135, ColorPosZ) && pass;
	pass = piglit_probe_pixel_rgb(110, 210, ColorNegX) && pass;
	pass = piglit_probe_pixel_rgb(185, 210, ColorNegY) && pass;
	pass = piglit_probe_pixel_rgb(260, 210, ColorNegZ) && pass;

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
