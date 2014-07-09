/*
 * Copyright 2014 VMware, Inc.
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
 * Test OpenGL 3.0 rendering to integer texture formats.
 * Brian Paul
 * 19 June 2014
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END


static const char *vertShaderText =
	"#version 130 \n"
	"uniform ivec4 int_in; \n"
	"flat out ivec4 int_val; \n"
	" \n"
	"void main() \n"
	"{ \n"
	"   gl_Position = gl_Vertex; \n"
	"   int_val = int_in; \n"
	"} \n";

static const char *fragShaderText =
	"#version 130 \n"
	"uniform ivec4 int_bias;\n"
	"flat in ivec4 int_val;\n"
	"out ivec4 int_result;\n"
	"void main()\n"
	"{ \n"
	"   int_result = int_val + int_bias; \n"
	"} \n";


static const GLint bias[4] = {1, 2, 3, 4};

static GLuint Prog;
static GLint IntInUniform, IntBiasUniform;
static GLint TexSize = 200;


static void
sum(int *s, const int *a, const int *b)
{
	int i;
	for (i = 0; i < 4; i++)
		s[i] = a[i] + b[i];
}


static bool
probe_int(int x, int y, const int *color, GLenum intFormat)
{
	int actual[4], expected[4];

	sum(expected, color, bias);

	switch (intFormat) {
	case GL_RGBA32I:
	case GL_RGBA32UI:
	case GL_RGBA16I:
	case GL_RGBA16UI:
	case GL_RGBA8I:
	case GL_RGBA8UI:
		break;
	case GL_RGB32I:
	case GL_RGB32UI:
	case GL_RGB16I:
	case GL_RGB16UI:
	case GL_RGB8I:
	case GL_RGB8UI:
		expected[3] = 1;
		break;
	case GL_RG32I:
	case GL_RG32UI:
	case GL_RG16I:
	case GL_RG16UI:
	case GL_RG8I:
	case GL_RG8UI:
		expected[2] = 0;
		expected[3] = 1;
		break;
	case GL_R32I:
	case GL_R32UI:
	case GL_R16I:
	case GL_R16UI:
	case GL_R8I:
	case GL_R8UI:
		expected[1] = 0;
		expected[2] = 0;
		expected[3] = 1;
		break;
	default:
		assert(!"Unexpected format in probe_int()");
	}

	/* need to clamp for 8-bit formats */
	if (intFormat == GL_RGBA8I ||
	    intFormat == GL_RGB8I ||
	    intFormat == GL_RG8I ||
	    intFormat == GL_R8I) {
		expected[0] = CLAMP(expected[0], -128, 127);
		expected[1] = CLAMP(expected[1], -128, 127);
		expected[2] = CLAMP(expected[2], -128, 127);
		expected[3] = CLAMP(expected[3], -128, 127);
	}
	else if (intFormat == GL_RGBA8UI ||
		 intFormat == GL_RGB8UI ||
		 intFormat == GL_RG8UI ||
		 intFormat == GL_R8UI) {
		expected[0] = CLAMP(expected[0], 0, 255);
		expected[1] = CLAMP(expected[1], 0, 255);
		expected[2] = CLAMP(expected[2], 0, 255);
		expected[3] = CLAMP(expected[3], 0, 255);
	}

	glReadPixels(x, y, 1, 1, GL_RGBA_INTEGER, GL_INT, actual);

	if (actual[0] != expected[0] ||
	    actual[1] != expected[1] ||
	    actual[2] != expected[2] ||
	    actual[3] != expected[3]) {
		printf("Failure at pixel (%d, %d):\n", x, y);
		printf("Format: %s\n", piglit_get_gl_enum_name(intFormat));
		printf("Expected: %d, %d, %d, %d\n",
		       expected[0], expected[1], expected[2], expected[3]);
		printf("Found: %d, %d, %d, %d\n",
		       actual[0], actual[1], actual[2], actual[3]);
		return false;
	}

	return true;
}


static bool
setup_fbo(GLenum intFormat)
{
	GLuint tex, fbo;
	GLenum status;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, intFormat, TexSize, TexSize, 0,
		     GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, NULL);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER,
			       GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, tex, 0);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		fprintf(stderr, "Failed to create integer FBO.\n");
		piglit_report_result(PIGLIT_FAIL);
		return false;
	}

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("Incomplete fbo for format %s (status %s)\n",
		       piglit_get_gl_enum_name(intFormat),
		       piglit_get_gl_enum_name(status));
		return false;
	}

	return true;
}



static bool
test_format(GLenum intFormat)
{
	static const GLint red[4] = {1000, 0, 0, 0};
	static const GLint green[4] = {2000, 0, 0, 0};
	static const GLint blue[4] = {0, 0, 3000, 0};
	static const GLint alpha[4] = {0, 0, 0, 4000};
	int x0 = TexSize / 4;
	int y0 = TexSize / 4;
	int x1 = TexSize * 3 / 4;
	int y1 = TexSize * 3 / 4;
	bool pass = true;
	enum piglit_result result;

	if (!setup_fbo(intFormat)) {
		result = PIGLIT_SKIP;
		goto end;
	}

	/* Draw different value into each texture quadrant */
	glUniform4iv(IntInUniform, 1, red);
	piglit_draw_rect(-1, -1, 1, 1);

	glUniform4iv(IntInUniform, 1, green);
	piglit_draw_rect(0, -1, 1, 1);

	glUniform4iv(IntInUniform, 1, blue);
	piglit_draw_rect(-1, 0, 1, 1);

	glUniform4iv(IntInUniform, 1, alpha);
	piglit_draw_rect(0, 0, 1, 1);

	pass = probe_int(x0, y0, red, intFormat) && pass;
	pass = probe_int(x1, y0, green, intFormat) && pass;
	pass = probe_int(x0, y1, blue, intFormat) && pass;
	pass = probe_int(x1, y1, alpha, intFormat) && pass;

	result = pass ? PIGLIT_PASS : PIGLIT_FAIL;

end:
	piglit_report_subtest_result(result,
				     "Format %s",
				     piglit_get_gl_enum_name(intFormat));
	return pass;
}


enum piglit_result
piglit_display(void)
{
	static const GLenum formats[] = {
		GL_RGBA32I,
		GL_RGB32I,
		GL_RG32I,
		GL_R32I,
		GL_RGBA16I,
		GL_RGB16I,
		GL_RG16I,
		GL_R16I,
		GL_RGBA8I,
		GL_RGB8I,
		GL_RG8I,
		GL_R8I,

		GL_RGBA32UI,
		GL_RGB32UI,
		GL_RG32UI,
		GL_R32UI,
		GL_RGBA16UI,
		GL_RGB16UI,
		GL_RG16UI,
		GL_R16UI,
		GL_RGBA8UI,
		GL_RGB8UI,
		GL_RG8UI,
		GL_R8UI,
	};
	bool pass = true;
	int i;

	for (i = 0; i < ARRAY_SIZE(formats); i++) {
		pass = test_format(formats[i]) && pass;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(30);

	Prog = piglit_build_simple_program(vertShaderText, fragShaderText);
	if (!Prog) {
		printf("Failed to compile/link program\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glBindFragDataLocation(Prog, 0, "int_result");
	glLinkProgram(Prog);
	glUseProgram(Prog);

	IntInUniform = glGetUniformLocation(Prog, "int_in");
	IntBiasUniform = glGetUniformLocation(Prog, "int_bias");

	glUniform4iv(IntBiasUniform, 1, bias);
}
