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
 * With Nvidia OpenGL drivers, the texelFetch() GLSL shader function
 * cannot return the correct data in the TextureView if we set the texture
 * parameter GL_TEXTURE_MAX_LEVEL for the TextureView.bug.
 *
 * Known to be
 *      -- Present in : Nvidia GTX 650, driver - 319.32
 *      -- Fixed in   : driver 319.59
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;
	config.supports_gl_compat_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define WIDTH 32
#define HEIGHT 32
#define LEVELS 6
#define COLOR_RED        0xFF0000FF
#define COLOR_GREEN      0x00FF00FF
#define COLOR_BLUE       0x0000FFFF
#define COLOR_CYAN       0x00FFFFFF
#define COLOR_MAGENTA    0xFF00FFFF
#define COLOR_YELLOW     0xFFFF00FF
#define COLOR_GRAY       0x7F7F7FFF
#define CLEAR_COLOR      0x000033FF
#define NUM_VERTICES     4
#define ATTR_SIZE        4
#define VIEW_LEVEL       3

static GLuint prog;

static bool
test_max_level(void)
{
	static const float vertArray[] = {
		1.0f, -1.0f, 0.0f, 1.0f,
		1.0f,  1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 1.0f,
	};
	GLint attrLoc = -1, samplerLoc = -1;
	const GLuint white = 0xffffffff;
	const unsigned int colors[LEVELS] = {COLOR_RED, COLOR_GREEN,
					     COLOR_BLUE, COLOR_CYAN,
					     COLOR_CYAN, COLOR_MAGENTA};
	const unsigned int numPixels = WIDTH * HEIGHT;
	GLuint texData[WIDTH * HEIGHT];
	GLuint i, texFbo, tex, view, fbo, vertexArray, vertexBuf, l;

	for (i = 0; i < numPixels; ++i) {
		texData[i] = white;
	}

	/* Create 2D textures */
	glGenTextures(1, &texFbo);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texFbo);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, WIDTH, HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, texData);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexStorage2D(GL_TEXTURE_2D, LEVELS, GL_RGBA8, WIDTH, HEIGHT);

	for (l = 0; l < LEVELS; ++l) {
		for (i = 0; i < numPixels >> (l * 2); ++i) {
			texData[i] = colors[l];
		}

		glPixelStorei(GL_UNPACK_ROW_LENGTH, WIDTH >> l);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexSubImage2D(GL_TEXTURE_2D, l, 0, 0,
				WIDTH >> l, HEIGHT >> l,
				GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
				texData);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, LEVELS - 1);

	/*
	 * Create a texture view that is the (VIEW_LEVEL) mipmap level
	 * of the original texture.
	 */
	glGenTextures(1, &view);
	glTextureView(view, GL_TEXTURE_2D, tex, GL_RGBA8,
		      VIEW_LEVEL, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, view);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	/* Setup the sampler */
	samplerLoc = glGetUniformLocation(prog, "s");
	glUniform1i(samplerLoc, 0);    // GL_TEXTURE0

	/* Setup vertex attributes. */
	attrLoc = glGetAttribLocation(prog, "Attr0");
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);
	glGenBuffers(1, &vertexBuf);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertArray), vertArray,
		     GL_STATIC_DRAW);
	glEnableVertexAttribArray(attrLoc);
	glVertexAttribPointer(attrLoc, ATTR_SIZE, GL_FLOAT, GL_FALSE,
			      ATTR_SIZE * sizeof(float), 0);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	/* Setup the FBO. */
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
	glDrawArrays(GL_TRIANGLE_STRIP, 0, NUM_VERTICES);

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

	if (texData[0] != colors[VIEW_LEVEL]) {
		printf("At pixel (0,0) expected 0x%x but found 0x%x\n",
		       colors[VIEW_LEVEL], texData[0]);
		/* clean up */
		glDeleteTextures(1, &tex);
		glDeleteTextures(1, &texFbo);
		glDeleteTextures(1, &view);
		glDeleteFramebuffers(1, &fbo);
		return false;
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	glDeleteTextures(1, &tex);
	glDeleteTextures(1, &texFbo);
	glDeleteTextures(1, &view);
	glDeleteFramebuffers(1, &fbo);
	return true;
}


enum piglit_result
piglit_display(void)
{
	bool pass = test_max_level();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


static void
setup_shaders(void)
{
	static const char *vsSrc =
		"#version 150\n"
		"in vec4 Attr0;"
		"void main(void) {"
		"   gl_Position = Attr0;"
		"}";
	static const char *fsSrc =
		"#version 150\n"
		"uniform sampler2D s;"
		"out vec4 fragColor0;"
		"void main(void) {"
		"   fragColor0 = texelFetch(s, ivec2(0, 0), 0);"
		"}";

	/* Create shaders and the program */
	prog = piglit_build_simple_program(vsSrc, fsSrc);
	glBindFragDataLocation(prog, 0, "fragColor0");
	glLinkProgram(prog);
	glUseProgram(prog);
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_ARB_texture_view");

	setup_shaders();
}
