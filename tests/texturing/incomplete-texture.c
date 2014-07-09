/*
 * Copyright © 2011 VMware, Inc.
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
 * Test what texture color is received when sampling from a missing/incomplete
 * texture, with fixed-function, ARB_vp, and GLSL.
 *
 * Brian Paul
 * Oct 2011
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define TEST_FIXED_FUNC 1
#define TEST_ARB_FP     2
#define TEST_GLSL       4
#define TEST_ALL        7


static unsigned Tests = 0;


static void
get_rect_bounds(int pos, int *x, int *y, int *w, int *h)
{
	*x = pos * (piglit_width / 3) + 5;
	*y = 5;
	*w = piglit_width / 3 - 10;
	*h = piglit_height - 10;
}


static void
draw_rect(int pos)
{
	int x, y, w, h;
	get_rect_bounds(pos, &x, &y, &w, &h);
	piglit_draw_rect_tex(x, y, w, h, 0, 0, 1, 1);
}


static GLboolean
probe_pos(int pos, const GLfloat expected[4])
{
	int x, y, w, h;
	get_rect_bounds(pos, &x, &y, &w, &h);
	return piglit_probe_rect_rgb(x, y, w, h, expected);
}


/**
 * For fixed function, if the texture is incomplete, it's as if that texture
 * unit was disabled.
 */
GLboolean
test_fixed_function(void)
{
	static const GLfloat expected[4] = { 0, 1, 0, 1.0 };
	int pos = 0;
	GLboolean p;

	glColor4fv(expected);
	glEnable(GL_TEXTURE_2D);
	draw_rect(pos);
	glDisable(GL_TEXTURE_2D);

	p = probe_pos(pos, expected);
	if (!p)
		printf("  Testing fixed-function\n");
	return p;
}


/**
 * The GL_ARB_fragment_shader spec, issue 23 says:
 *   (23) What is the result of a sample from an incomplete texture? 
 *   The definition of texture completeness can be found in section 3.8.9 
 *   of the core GL spec. 
 *
 *   RESOLVED: The result of a sample from an incomplete texture is the 
 *   constant vector (0,0,0,1).
 *
 * In this test we swizzle RGBA->ABGR so we don't need to worry if the
 * framebuffer has an alpha channel.
 */
GLboolean
test_arb_fp(void)
{
	static const char *fragProgramText =
		"!!ARBfp1.0\n"
		"TEMP t1;\n"
		"TEX t1, fragment.texcoord[0], texture[0], 2D;\n"
		"MOV result.color, t1.abgr;\n"
		"END\n";
	static const GLfloat expected[4] = { 1.0, 0.0, 0.0, 0.0 };
	int pos = 1;
	GLboolean p;
	GLuint prog;

	glGenProgramsARB(1, &prog);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, prog);
	glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,
			   GL_PROGRAM_FORMAT_ASCII_ARB,
			   strlen(fragProgramText),
			   (const GLubyte *) fragProgramText);

	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	glColor3f(0, 1, 0);
	draw_rect(1);

	glDisable(GL_FRAGMENT_PROGRAM_ARB);
	glDeleteProgramsARB(1, &prog);

	p = probe_pos(pos, expected);
	if (!p)
		printf("  Testing ARB fragment program\n");
	return p;
}


/**
 * Section 3.11.2 of the GL 2.1 spec says:
 *    If a fragment shader uses a sampler whose associated texture object is
 *    not complete, as defined in section 3.8.10, the texture image unit will
 *    return (R, G, B, A) = (0, 0, 0, 1).
 *
 * In this test we swizzle RGBA->ABGR so we don't need to worry if the
 * framebuffer has an alpha channel.
 */
GLboolean
test_glsl(void)
{
	static const char *fragShaderText =
		"uniform sampler2D tex;\n"
		"void main()\n"
		"{\n"
		"   gl_FragColor = texture2D(tex, gl_TexCoord[0].xy).abgr;\n"
		"}\n";
	static const GLfloat expected[4] = { 1.0, 0.0, 0.0, 0.0 };
	int pos = 2;
	GLboolean p;
	GLuint frag, prog;
	GLint tex;

	frag = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fragShaderText);
	prog = piglit_link_simple_program(0, frag);

	glUseProgram(prog);
	tex = glGetUniformLocation(prog, "tex");
	glUniform1i(tex, 0);

	glColor3f(0, 1, 0);
	draw_rect(2);

	glUseProgram(0);
	glDeleteShader(frag);
	glDeleteProgram(prog);

	p = probe_pos(pos, expected);
	if (!p)
		printf("  Testing GLSL\n");
	return p;
}


enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClear(GL_COLOR_BUFFER_BIT);

	if (Tests & TEST_FIXED_FUNC) {
		pass = test_fixed_function() && pass;
	}

	if (Tests & TEST_ARB_FP) {
		piglit_require_extension("GL_ARB_fragment_program");
		pass = test_arb_fp() && pass;
	}

	if (Tests & TEST_GLSL) {
		piglit_require_GLSL_version(110);
		pass = test_glsl() && pass;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


static void
setup_texture(void)
{
#define TW 64
#define TH 64
	GLubyte img[TH][TW][4];
	GLuint t;
	int i, j, w, h;

	/* solid red texture */
	for (i = 0; i < TH; i++) {
		for (j = 0; j < TW; j++) {
			img[i][j][0] = 0xff;
			img[i][j][1] = 0x0;
			img[i][j][2] = 0x0;
			img[i][j][3] = 0x0;
		}
	}

	/* make a texture with the last mipmap level omitted so
	 * that it's incomplete
	 */
	glGenTextures(1, &t);
	glBindTexture(GL_TEXTURE_2D, t);
	w = TW;
	h = TH;
	for (i = 0; w > 1 && h > 1; i++) {
		glTexImage2D(GL_TEXTURE_2D, i, GL_RGB, w, h, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, img);
		w /= 2;
		h /= 2;
	}

	if (0) {
		/* Enable this to force the texture to be complete */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glClearColor(0.5, 0.5, 0.5, 0.0);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}


void
piglit_init(int argc, char **argv)
{
	int i;

	Tests = 0;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "fixed") == 0) {
			Tests |= TEST_FIXED_FUNC;
		}
		else if (strcmp(argv[i], "arb_fp") == 0) {
			Tests |= TEST_ARB_FP;
		}
		else if (strcmp(argv[i], "glsl") == 0) {
			Tests |= TEST_GLSL;
		}
		else if (strcmp(argv[i], "all") == 0) {
			Tests |= TEST_ALL;
		}
		else {
			printf("Usage:\n");
			printf("  incomplete-texture fixed | arb_fp | glsl | all\n");
			piglit_report_result(PIGLIT_SKIP);
		}
	}

	setup_texture();
}
