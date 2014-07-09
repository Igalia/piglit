/*
 * Copyright © 2013 VMware, Inc.
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
 * Check for a glScissorIndexed() bug found in MacOS AMD driver.
 * Passes with NVIDIA's Linux driver.
 *
 * Brian Paul and others at VMware
 * 15 Nov 2013
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;
	config.supports_gl_compat_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define WIDTH 32
#define HEIGHT 32

static bool
test(void)
{
	static const char *vsSrc =
		"#version 150\n"
		"in vec4 Attr0;"
		"void main(void) {"
		"   gl_Position = Attr0;"
		"}";
	static const char *fsSrc =
		"#version 150\n"
		"out vec4 fragColor0;"
		"void main(void) {"
		"   fragColor0 = vec4(1, 0, 0, 1);"
		"}";
	static const float vertArray[] = {
		1.0f, -1.0f, 0.0f, 1.0f,
		1.0f,  1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 1.0f,
	};
	const GLuint white = 0xffffffff;
	const GLuint red = 0xff0000ff;
	const GLuint magenta = 0xff00ffff;
	const unsigned int numPixels = WIDTH * HEIGHT;
	GLuint texData[WIDTH * HEIGHT];
	GLuint i, tex, fbo, prog, vertexArray, vertexBuf;

	for (i = 0; i < numPixels; ++i) {
		texData[i] = white;
	}

	/* Create a white 2D texture. */
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, WIDTH, HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, texData);

	/* Create FBO with texture color attachment */
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, tex, 0);
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

	/* Create shader program */
	prog = piglit_build_simple_program(vsSrc, fsSrc);
	glBindFragDataLocation(prog, 0, "fragColor0");
	glLinkProgram(prog);
	glUseProgram(prog);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	/* Setup vertex attributes. */
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);
	glGenBuffers(1, &vertexBuf);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertArray), vertArray,
		     GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	glClearColor(1, 0, 1, 1);  /* magenta */
	glEnable(GL_SCISSOR_TEST);
	glViewportIndexedf(0, 0, 0, WIDTH, HEIGHT);
	glScissorIndexed(0, 16, 16, 16, 16);

	/* This scissor rect should be ignored/unused but with the buggy
	 * driver, it does effect subsequent clearing and drawing.
	 */
	glScissorIndexed(1, 0, 0, 0, 0);

	/* Should clear upper-right 16x16 to magenta while leaving
	 * the rest white.
	 */
	glClear(GL_COLOR_BUFFER_BIT);

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

	if (texData[0] != white) {
		printf("At pixel (0,0) expected 0x%x but found 0x%x\n",
		       white, texData[0]);
		return false;
	}

	if (texData[numPixels - 1] != magenta) {
		printf("At pixel (%d,%d) expected 0x%x but found 0x%x\n",
		       WIDTH - 1, HEIGHT - 1, magenta, texData[numPixels - 1]);
		return false;
	}

	/* Draw red quad (fragment shader always emits red).
	 * With scissor, upper-right 16x16 should be red, leaving the rest
	 * white.
	 */
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	/* read color buffer */
	memset(texData, 0, sizeof(texData));
	glReadPixels(0, 0, WIDTH, HEIGHT,
		     GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, texData);

	if (texData[0] != white) {
		printf("At pixel (0,0) expected 0x%x but found 0x%x\n",
		       white, texData[0]);
		return false;
	}

	if (texData[numPixels - 1] != red) {
		printf("At pixel (%d,%d) expected 0x%x but found 0x%x\n",
		       WIDTH - 1, HEIGHT - 1, magenta, texData[numPixels - 1]);
		return false;
	}

	return true;
}


enum piglit_result
piglit_display(void)
{
	/* should never get here */
	return PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	bool pass;

	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_ARB_viewport_array");

	pass = test();
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
