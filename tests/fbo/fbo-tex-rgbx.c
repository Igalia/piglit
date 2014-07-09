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
 * \file fbo-tex-rgbx.c
 * \author Brian Paul
 *
 * Test texturing from an RGB texture after we've rendered some
 * non-1 alpha values into it.  When we sample from an RGB texture,
 * the alpha values should always be one.
 * Many (most?) GL implementations store RGB textures as RGBx so there
 * really is an alpha channel but it's supposed to be ignored when we
 * sample from it.
 */

#include "piglit-util-gl.h"

#define TEX_SIZE 256


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.window_width = TEX_SIZE;
	config.window_height = TEX_SIZE;
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END


static const char *fragShaderText =
	"uniform sampler2D tex; \n"
	"void main() \n"
	"{ \n"
	"   gl_FragColor = texture2D(tex, gl_TexCoord[0].xy); \n"
	"} \n";

static GLuint Program;


/* draw green quad with varying alpha values */
static void
draw_quad(void)
{
	static const float verts[4][2] = {
		{ -1.0f, -1.0f },
		{  1.0f, -1.0f },
		{  1.0f,  1.0f },
		{ -1.0f,  1.0f }
	};
	static const float colors[4][4] = {
		{ 0.0f, 1.0f, 0.0f, 0.1f },
		{ 0.0f, 1.0f, 0.0f, 0.2f },
		{ 0.0f, 1.0f, 0.0f, 0.8f },
		{ 0.0f, 1.0f, 0.0f, 0.9f }
	};
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glColorPointer(4, GL_FLOAT, 0, colors);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}


/**
 * Create texture with given internal format.  Render to it.  Then draw
 * a test quad using that texture.  Check that all alpha values are one.
 */
static bool
test_format(GLenum internalFormat)
{
	GLuint tex, fbo;
	GLenum status;
	bool pass = true;
	GLubyte *results;
	GLuint i;

	/* Create (RGB) tex and FBO to render into it */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, TEX_SIZE, TEX_SIZE, 0,
		     GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER,
			       GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, tex, 0);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "incomplete fbo (status 0x%x)\n", status);
		return true; /* this isn't necessarily a failure */
	}

	/* Draw into tex, constant green and varying alpha */
	glUseProgram(0);
	draw_quad();

	/* Now draw a textured quad in the window using the texture that we
	 * just rendered to.
	 */
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(Program);

	piglit_draw_rect_tex(-1.0f, -1.0f, 2.0f, 2.0f,
			     0.0f, 0.0f, 1.0f, 1.0f);

	/* Now read back the rendering and check that alpha is always one */
	results = malloc(TEX_SIZE * TEX_SIZE * 4);
	glReadPixels(0, 0, TEX_SIZE, TEX_SIZE,
		     GL_RGBA, GL_UNSIGNED_BYTE, results);

	for (i = 0; i < TEX_SIZE * TEX_SIZE; i++) {
		if (results[i*4+3] != 255) {
			fprintf(stderr,
				"Bad alpha value at texel [%d]: %d."
				"  Should be 255."
				"  Texture format %s\n",
				i, results[i*4+3],
				piglit_get_gl_enum_name(internalFormat));
			pass = false;
			break;
		}
	}

	free(results);

	piglit_present_results();

	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &tex);

	return pass;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_gl_version(20);

	Program = piglit_build_simple_program(NULL, fragShaderText);
	assert(Program);
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;

	pass = test_format(GL_RGB) && pass;
	pass = test_format(GL_RGB4) && pass;
	pass = test_format(GL_RGB5) && pass;
	pass = test_format(GL_RGB8) && pass;
	pass = test_format(GL_RGB10) && pass;
	pass = test_format(GL_RGB12) && pass;
	pass = test_format(GL_RGB16) && pass;

	if (piglit_is_extension_supported("GL_ARB_ES2_compatibility")) {
		pass = test_format(GL_RGB565) && pass;
	}

	if (piglit_is_extension_supported("GL_EXT_texture_sRGB")) {
		pass = test_format(GL_SRGB) && pass;
		pass = test_format(GL_SRGB8) && pass;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
