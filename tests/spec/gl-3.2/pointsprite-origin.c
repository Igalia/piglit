/*
 * Copyright © 2014 VMware, Inc.
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

/**
 * With Mac AMD GL OpenGL drivers, the texture coordinate v (in Y-direction)
 * is flipped for point sprites.
 *
 * Known to be
 *      -- Present in : ATI HD 6770M on Mac OS X 10.8.4
 *      -- Fixed in   : Mac OS 10.9
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;
	config.supports_gl_compat_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define WIDTH 32
#define HEIGHT 32
#define COLOR_GRAY       0x7F7F7FFF
#define CLEAR_COLOR      0x000033FF

static GLuint prog;

static bool
test_pointsprite_origin(void)
{
	const unsigned int numPixels = WIDTH * HEIGHT;
	GLuint texData[WIDTH * HEIGHT];
	GLuint i, texFbo, fbo, vao;
	const float pointSize = WIDTH;
	const unsigned char expectedChannelVal = 0.5f / WIDTH * 255.0f + 0.5f;
	const unsigned int  expectedTexelColor = expectedChannelVal << 24 |
						 expectedChannelVal << 16 |
						 0x0000FFFF;

	for (i = 0; i < numPixels; ++i) {
		texData[i] = COLOR_GRAY;
	}

	/* Create vertex arrays */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Create 2D textures */
	glGenTextures(1, &texFbo);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texFbo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WIDTH, HEIGHT, 0,
		     GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, texData);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	/* Setup the FBO */
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, texFbo, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
	    GL_FRAMEBUFFER_COMPLETE) {
		printf("incomplete framebuffer at line %d\n", __LINE__);
		return false;
	}

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
	    GL_FRAMEBUFFER_COMPLETE) {
		printf("incomplete framebuffer at line %d\n", __LINE__);
		return false;
	}

	/* Clear and draw */
	glViewport(0, 0, WIDTH, HEIGHT);
	glClearColor(((CLEAR_COLOR >> 24) & 0xFF) / 255.0f,
		     ((CLEAR_COLOR >> 16) & 0xFF) / 255.0f,
		     ((CLEAR_COLOR >> 8)  & 0xFF) / 255.0f,
		     ((CLEAR_COLOR) & 0xFF) / 255.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(pointSize);
	glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
	glDrawArrays(GL_POINTS, 0, 1);

	/* Read back */
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
	    GL_FRAMEBUFFER_COMPLETE) {
		printf("incomplete framebuffer at line %d\n", __LINE__);
		return false;
	}

	/* read color buffer */
	glPixelStorei(GL_PACK_ROW_LENGTH, WIDTH);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	memset(texData, 0, sizeof(texData));
	glReadPixels(0, 0, WIDTH, HEIGHT,
		     GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, texData);

	if (texData[0] != expectedTexelColor) {
		printf("At pixel (0,0) expected 0x%x but found 0x%x\n",
			expectedTexelColor, texData[0]);
		/* clean up */
		glDeleteTextures(1, &texFbo);
		glDeleteFramebuffers(1, &fbo);
		return false;
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	glDeleteTextures(1, &texFbo);
	glDeleteFramebuffers(1, &fbo);
	return true;
}


static void
setup_shaders(void)
{
	static const char *vsSrc =
		"#version 150\n"
		"void main(void) {"
		"   gl_Position = vec4(0, 0, 0, 1);"
		"}";
	static const char *fsSrc =
		"#version 150\n"
		"out vec4 fragColor0;"
		"void main(void) {"
		"   fragColor0.xy = gl_PointCoord.xy;"
		"   fragColor0.zw = vec2(1, 1);"
		"}";

	/* Create shaders and the program */
	prog = piglit_build_simple_program(vsSrc, fsSrc);
	glBindFragDataLocation(prog, 0, "fragColor0");
	glLinkProgram(prog);
	glUseProgram(prog);
}


enum piglit_result
piglit_display(void)
{
	bool pass = test_pointsprite_origin();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	setup_shaders();
}
