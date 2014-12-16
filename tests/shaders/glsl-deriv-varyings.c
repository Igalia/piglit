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
 * This test uses the built-in glsl derivative functions (dFdx and dFdy)
 * on varying values.
 */

#include "piglit-util-gl.h"

static void compileLinkProg(void);
static void loadTex(void);

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 600;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint tex[1];
static GLint prog1;
static GLint vs1;
static GLint fs1;
static GLint prog2;
static GLint fs2;
static GLint prog3;
static GLint fs3;
static GLint prog4;
static GLint fs4;
static GLint prog5;
static GLint fs5;


static GLfloat verts[12] = {175.0, 125.0, 0.0,
			    175.0, 225.0, 0.0,
			    125.0, 125.0, 0.0,
			    125.0, 225.0, 0.0};

static GLfloat texCoords[8] = {1.0, 0.0,
				1.0, 1.0,
				0.0, 0.0,
				0.0, 1.0};


static const char *vertShaderText =
	"attribute vec2 textureCoords;\n"
	"varying vec2 texCoords;\n"
	"void main()\n"
	"{ \n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"	texCoords = textureCoords;\n"
	"} \n";

static const char *fragShaderText =
	"uniform sampler2D tex2d;\n"
	"varying vec2 texCoords;\n"
	"void main()\n"
	"{ \n"
	"	gl_FragColor = texture2D(tex2d, texCoords);\n"
	"} \n";

static const char *fragShaderText2 =
	"uniform sampler2D tex2d;\n"
	"varying vec2 texCoords;\n"
	"void main()\n"
	"{ \n"
	"	gl_FragColor = vec4(dFdx(texCoords.x) * 40.0,\n"
	"			    dFdy(texCoords.y) * 40.0,\n"
	"			    0.0, 1.0);\n"
	"} \n";

static const char *fragShaderText3 =
	"uniform sampler2D tex2d;\n"
	"varying vec2 texCoords;\n"
	"void main()\n"
	"{ \n"
	"	gl_FragColor = vec4(dFdx(-texCoords.x) * -40.0,\n"
	"			    dFdy(-texCoords.y) * -40.0,\n"
	"			    0.0, 1.0);\n"
	"} \n";

static const char *fragShaderText4 =
	"uniform sampler2D tex2d;\n"
	"varying vec2 texCoords;\n"
	"void main()\n"
	"{ \n"
	"	gl_FragColor = vec4(dFdx(abs(-texCoords.x)) * 40.0,\n"
	"			    dFdy(abs(-texCoords.y)) * 40.0,\n"
	"			    0.0, 1.0);\n"
	"} \n";

static const char *fragShaderText5 =
	"uniform sampler2D tex2d;\n"
	"varying vec2 texCoords;\n"
	"void main()\n"
	"{ \n"
	"	gl_FragColor = vec4(dFdx(-abs(texCoords.x)) * -40.0,\n"
	"			    dFdy(-abs(texCoords.y)) * -40.0,\n"
	"			    0.0, 1.0);\n"
	"} \n";



void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(20);

	compileLinkProg();

	loadTex();

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glEnable(GL_TEXTURE_2D);
	glClearColor(0.6, 0.6, 0.6, 1.0);
}

static void
compileLinkProg(void)
{
	GLint stat;

	vs1 = glCreateShader(GL_VERTEX_SHADER);
	fs1 = glCreateShader(GL_FRAGMENT_SHADER);

	fs2 = glCreateShader(GL_FRAGMENT_SHADER);

	fs3 = glCreateShader(GL_FRAGMENT_SHADER);

	fs4 = glCreateShader(GL_FRAGMENT_SHADER);

	fs5 = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vs1, 1, (const GLchar **) &vertShaderText, NULL);
	glShaderSource(fs1, 1, (const GLchar **) &fragShaderText, NULL);
	glShaderSource(fs2, 1, (const GLchar **) &fragShaderText2, NULL);
	glShaderSource(fs3, 1, (const GLchar **) &fragShaderText3, NULL);
	glShaderSource(fs4, 1, (const GLchar **) &fragShaderText4, NULL);
	glShaderSource(fs5, 1, (const GLchar **) &fragShaderText5, NULL);

	glCompileShader(vs1);
	glGetShaderiv(vs1, GL_COMPILE_STATUS, &stat);
	if (!stat) {
                printf("error compiling vertex shader1!\n");
                exit(1);
        }

	glCompileShader(fs1);
	glGetShaderiv(fs1, GL_COMPILE_STATUS, &stat);
	if (!stat) {
		printf("error compiling fragment shader1!\n");
		exit(1);
	}

	glCompileShader(fs2);
	glGetShaderiv(fs2, GL_COMPILE_STATUS, &stat);
	if (!stat) {
		printf("error compiling fragment shader2!\n");
		exit(1);
	}

	glCompileShader(fs3);
	glGetShaderiv(fs3, GL_COMPILE_STATUS, &stat);
	if (!stat) {
		printf("error compiling fragment shader3!\n");
		exit(1);
	}

	glCompileShader(fs4);
	glGetShaderiv(fs4, GL_COMPILE_STATUS, &stat);
	if (!stat) {
		printf("error compiling fragment shader4!\n");
		exit(1);
	}

	glCompileShader(fs5);
	glGetShaderiv(fs5, GL_COMPILE_STATUS, &stat);
	if (!stat) {
		printf("error compiling fragment shader5!\n");
		exit(1);
	}


	prog1 = glCreateProgram();
	glAttachShader(prog1, vs1);
	glAttachShader(prog1, fs1);
	glBindAttribLocation(prog1, 1, "textureCoords");
	glLinkProgram(prog1);
	glUseProgram(prog1);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat),
                                verts);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat),
				texCoords);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);


	prog2 = glCreateProgram();
	glAttachShader(prog2, vs1);
	glAttachShader(prog2, fs2);
	glBindAttribLocation(prog2, 1, "textureCoords");
	glLinkProgram(prog2);
	glUseProgram(prog2);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat),
                               verts);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat),
				texCoords);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);


	prog3 = glCreateProgram();
	glAttachShader(prog3, vs1);
	glAttachShader(prog3, fs3);
	glBindAttribLocation(prog3, 1, "textureCoords");
	glLinkProgram(prog3);
	glUseProgram(prog3);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat),
                               verts);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat),
				texCoords);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);


	prog4 = glCreateProgram();
	glAttachShader(prog4, vs1);
	glAttachShader(prog4, fs4);
	glBindAttribLocation(prog4, 1, "textureCoords");
	glLinkProgram(prog4);
	glUseProgram(prog4);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat),
                               verts);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat),
				texCoords);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);


	prog5 = glCreateProgram();
	glAttachShader(prog5, vs1);
	glAttachShader(prog5, fs5);
	glBindAttribLocation(prog5, 1, "textureCoords");
	glLinkProgram(prog5);
	glUseProgram(prog5);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat),
                               verts);
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

	GLfloat texData[width][height][4];
	for (i=0; i < width; ++i) {
		for (j=0; j < height; ++j) {
			if ((i+j) & 1) {
				texData[i][j][0] = 1;
				texData[i][j][1] = 0;
				texData[i][j][2] = 1;
				texData[i][j][3] = 0;
			}
			else {
				texData[i][j][0] = 0;
				texData[i][j][1] = 1;
				texData[i][j][2] = 0;
				texData[i][j][3] = 1;
			}
		}
	}

	glGenTextures(1, tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
			GL_RGBA, GL_FLOAT, texData);

	#undef height
	#undef width
}


enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;

	float deriv[3] = {0.8, 0.4, 0.0};
	float green[3] = {0, 1, 0};

	glClear(GL_COLOR_BUFFER_BIT);

	glPushMatrix();

	glUseProgram(prog1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glTranslatef(75.0, 0.0, 0.0);

	glUseProgram(prog2);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glTranslatef(75.0, 0.0, 0.0);

	glUseProgram(prog3);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glTranslatef(75.0, 0.0, 0.0);

	glUseProgram(prog4);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glTranslatef(75.0, 0.0, 0.0);

	glUseProgram(prog5);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glPopMatrix();

	pass = pass && piglit_probe_pixel_rgb(132, 125, green);
	pass = pass && piglit_probe_pixel_rgb(205, 125, deriv);
	pass = pass && piglit_probe_pixel_rgb(280, 125, deriv);
	pass = pass && piglit_probe_pixel_rgb(355, 125, deriv);
	pass = pass && piglit_probe_pixel_rgb(430, 125, deriv);

	glFinish();
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

