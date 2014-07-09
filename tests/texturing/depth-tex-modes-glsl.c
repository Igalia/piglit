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

/*
 * GLSL version of depth-tex-modes. Draws depth textures as LUMINANCE,
 * INTENSITY, and ALPHA with programmable shaders.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 400;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint tex[3];
static GLint prog;
static GLint fs;
static GLint vs;


static GLfloat vertices[12] = {150.0, 125.0, 0.0,
				150.0, 175.0, 0.0,
				100.0, 125.0, 0.0,
				100.0, 175.0, 0.0};

static GLfloat texCoords[8] = {1.0, 0.0,
				1.0, 1.0,
				0.0, 0.0,
				0.0, 1.0};

static GLuint elements[4] = {0, 1, 2, 3};

static const char *vertShaderText =
	"attribute vec2 textureCoords;\n"
	"varying vec2 texCoords;\n"
	"void main()\n"
	"{ \n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"	texCoords = textureCoords;\n"
	"} \n";

static const char *fragShaderText =
	"uniform sampler2D depthTex2d;\n"
	"uniform int colorOrAlpha;\n"
	"varying vec2 texCoords;\n"
	"void main()\n"
	"{ \n"
	"	vec4 color = vec4(1.0, 0.0, 1.0, 1.0);\n"
	"       vec4 depth = texture2D(depthTex2d, texCoords);\n"
	"	if (colorOrAlpha == 0)\n"
	"		gl_FragColor = vec4(color.xyz*depth.xyz, 1.0);\n"
	"       else\n"
	" 	       gl_FragColor = vec4(color.xyz*depth.w, 1.0);\n"
	"} \n";

static void compileLinkProg(void);
static void loadTex(void);

void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(20);

	if (!piglit_automatic)
		printf(" Left to Right: LUMINANCE, INTENSITY, ALPHA\n"
		       "Lower row: Combined with color\n"
		       "Upper row: combined with alpha\n");

	loadTex();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 400, 0, 300, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	glClearColor(0.2, 0.2, 0.2, 1.0);

	compileLinkProg();
}

static void
compileLinkProg(void)
{
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vertShaderText);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fragShaderText);

	prog = piglit_link_simple_program(vs, fs);
	glBindAttribLocation(prog, 1, "textureCoords");
	glUseProgram(prog);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat),
				vertices);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat),
				texCoords);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

static void
loadTex(void)
{
	#define height 2
	#define width 2
	int i, j;

	GLfloat texDepthData[width][height];
	for (i=0; i < width; ++i) {
		for (j=0; j < height; ++j) {
			if ((i+j) & 1) {
				texDepthData[i][j] = 1.0;
			}
			else {
				texDepthData[i][j] = 0.0;
			}
		}
	}

	//depth texture 0 using LUMINANCE
	glGenTextures(3, tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, texDepthData);


	//depth texture 1 using INTENSITY
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, texDepthData);


	//depth texture 2 using ALPHA
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_ALPHA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, texDepthData);

	#undef height
	#undef width
}

enum piglit_result
piglit_display(void)
{
	GLint loc1, loc2;

	GLboolean pass = GL_TRUE;

	GLfloat pink[3] = {1.0, 0.0, 1.0};
	GLfloat black[3] = {0.0, 0.0, 0.0};

	GLenum err;

	loc1 = glGetUniformLocation(prog, "depthTex2d");
	loc2 = glGetUniformLocation(prog, "colorOrAlpha");

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);


	glUniform1i(loc1, 0);
	glUniform1i(loc2, 0);
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, elements);

	glPushMatrix();
	glTranslatef(75.0, 0.0, 0.0);

	glUniform1i(loc1, 1);
	glUniform1i(loc2, 0);
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, elements);

	glTranslatef(75.0, 0.0, 0.0);

	glUniform1i(loc1, 2);
	glUniform1i(loc2, 0);
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, elements);

	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.0, 75.0, 0.0);


	glUniform1i(loc1, 0);
	glUniform1i(loc2, 1);
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, elements);

	glTranslatef(75.0, 0.0, 0.0);

	glUniform1i(loc1, 1);
	glUniform1i(loc2, 1);
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, elements);

	glTranslatef(75.0, 0.0, 0.0);

	glUniform1i(loc1, 2);
	glUniform1i(loc2, 1);
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, elements);

	glPopMatrix();

	pass = piglit_probe_pixel_rgb(110, 135, black);
	pass = pass && piglit_probe_pixel_rgb(140, 135, pink);
	pass = pass && piglit_probe_pixel_rgb(185, 135, black);
	pass = pass && piglit_probe_pixel_rgb(215, 135, pink);
	pass = pass && piglit_probe_pixel_rgb(260, 135, black);
	pass = pass && piglit_probe_pixel_rgb(290, 135, black);

	pass = pass && piglit_probe_pixel_rgb(110, 210, pink);
	pass = pass && piglit_probe_pixel_rgb(140, 210, pink);
	pass = pass && piglit_probe_pixel_rgb(185, 210, black);
	pass = pass && piglit_probe_pixel_rgb(215, 210, pink);
	pass = pass && piglit_probe_pixel_rgb(260, 210, black);
	pass = pass && piglit_probe_pixel_rgb(290, 210, pink);

	err = glGetError();
	switch (err)
	{
	case GL_INVALID_ENUM:
		printf("GL_INVALID_ENUM\n");
		break;
	case GL_INVALID_VALUE:
		printf("GL_INVALID_VALUE\n");
		break;
	case GL_INVALID_OPERATION:
		printf("GL_INVALID_OPERATION\n");
		break;
	case GL_STACK_OVERFLOW:
		printf("GL_STACK_OVERLFOW\n");
		break;
	case GL_STACK_UNDERFLOW:
		printf("GL_STACK_UNDERFLOW\n");
		break;
	case GL_OUT_OF_MEMORY:
		printf("GL_OUT_OF_MEMORY\n");
		break;

	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;

}
