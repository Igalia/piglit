/*
 * Copyright (c) 2011 Red Hat Inc.
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

/*
 * Author:
 *    Dave Airlie
 */

/**
 * @file fbo-extended-blend.c
 *
 * Test GL_ARB_blend_func_extended.
 *
 * Note all closed drivers seem to only support 1 dual source draw target
 * so just have the initial test validate that
 */

#include "piglit-util.h"
#include "piglit-framework.h"

int piglit_width = 100;
int piglit_height = 100;
int piglit_window_mode = GLUT_RGB;

static const char *TestName = "fbo-extended-blend";

static GLint max_ds_buffers;
static GLuint fbo;

static GLenum srcFactors[] = {
	GL_ZERO,
	GL_SRC1_COLOR,
	GL_ONE_MINUS_SRC1_COLOR,
	GL_SRC1_ALPHA,
	GL_ONE_MINUS_SRC1_ALPHA,
	GL_SRC_ALPHA_SATURATE,
};

static GLenum dstFactors[] = {
	GL_ZERO,
	GL_SRC1_COLOR,
	GL_ONE_MINUS_SRC1_COLOR,
	GL_SRC1_ALPHA,
	GL_ONE_MINUS_SRC1_ALPHA,
	GL_SRC_ALPHA_SATURATE,
};

static GLenum operators[] = {
	GL_FUNC_ADD,
	GL_FUNC_SUBTRACT,
	GL_FUNC_REVERSE_SUBTRACT,
	GL_MIN,
	GL_MAX,
};

static void
check_error(int line)
{
	GLenum err = glGetError();
	if (err) {
		printf("%s: Unexpected error 0x%x at line %d\n",
		       TestName, err, line);
		piglit_report_result(PIGLIT_FAIL);
	}
}

static GLint uniform_src0, uniform_src1;

static void blend(const float *src, const float *src1, const float *dst,
		  GLenum blendsrc, GLenum blenddst, GLenum blendop)
{
	glUniform4fv(uniform_src0, 1, dst);
	piglit_draw_rect(0, 0, piglit_width, piglit_height);
	glEnable(GL_BLEND);
	glBlendEquation(blendop);
	glBlendFunc(blendsrc, blenddst);
	glUniform4fv(uniform_src0, 1, src);
	glUniform4fv(uniform_src1, 1, src1);
	piglit_draw_rect(0, 0, piglit_width, piglit_height);
	glDisable(GL_BLEND);
}

static void blend_expected(float *expected, const float *src, const float *src1, const float *dst, GLenum blendsrc, GLenum blenddst, GLenum blendop)
{
	float a;
	int i;
	float src_vals[4] = {0}, dst_vals[4] = {0};

	switch (blendsrc) {
	case GL_ZERO:
		for (i = 0; i < 4; i++)
			src_vals[i] = 0;
		break;
	case GL_SRC1_COLOR:
		for (i = 0; i < 4; i++)
			src_vals[i] = src[i] * src1[i];
		break;
	case GL_ONE_MINUS_SRC1_COLOR:
		for (i = 0; i < 4; i++)
			src_vals[i] = src[i] * (1.0 - src1[i]);
		break;
	case GL_SRC1_ALPHA:
		a = src1[3];
		for (i = 0; i < 4; i++)
			src_vals[i] = src[i] * a;
		break;
	case GL_ONE_MINUS_SRC1_ALPHA:
		a = src1[3];
		for (i = 0; i < 4; i++)
			src_vals[i] = src[i] * (1.0 - a);
		break;
	case GL_SRC_ALPHA_SATURATE:
		a = MIN2(src[3], (1 - dst[3]));
		for (i = 0; i < 3; i++)
			src_vals[i] = src[i] * a;
		src_vals[3] = src[3];
		break;
	}

	switch (blenddst) {
	case GL_ZERO:
		break;
	case GL_SRC1_COLOR:
		for (i = 0; i < 4; i++)
			dst_vals[i] = dst[i] * src1[i];
		break;
	case GL_ONE_MINUS_SRC1_COLOR:
		for (i = 0; i < 4; i++)
			dst_vals[i] = dst[i] * (1.0 - src1[i]);
		break;
	case GL_SRC1_ALPHA:
		a = src1[3];
		for (i = 0; i < 4; i++)
			dst_vals[i] = dst[i] * a;
		break;
	case GL_ONE_MINUS_SRC1_ALPHA:
		a = src1[3];
		for (i = 0; i < 4; i++)
			dst_vals[i] = dst[i] * (1.0 - a);
		break;
	case GL_SRC_ALPHA_SATURATE:
		a = MIN2(src[3], (1 - dst[3]));
		for (i = 0; i < 3; i++)
			dst_vals[i] = dst[i] * a;
		dst_vals[3] = dst[3];
		break;
	}

	switch (blendop) {
	case GL_FUNC_ADD:
		for (i = 0 ; i < 4; i++)
			expected[i] = src_vals[i] + dst_vals[i];
		break;
	case GL_FUNC_SUBTRACT:
		for (i = 0 ; i < 4; i++) {
			expected[i] = src_vals[i] - dst_vals[i];
			if (expected[i] < 0)
				expected[i] = 0;
		}
		break;
	case GL_FUNC_REVERSE_SUBTRACT:
		for (i = 0 ; i < 4; i++) {
			expected[i] = dst_vals[i] - src_vals[i];
			if (expected[i] < 0)
				expected[i] = 0;
		}
		break;
	case GL_MIN:
		for (i = 0 ; i < 4; i++) {
			expected[i] = MIN2(dst[i], src[i]);
		}
		break;
	case GL_MAX:
		for (i = 0 ; i < 4; i++) {
			expected[i] = MAX2(dst[i], src[i]);
		}
		break;
	}
}

static const char *vs_text =
	"#version 130\n"
	"void main() {\n"
	"        gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"        gl_FrontColor = gl_Color;\n"
	"}\n"
	;

static const char *fs_text =
	"#version 130\n"
	"uniform vec4 src0;\n"
	"uniform vec4 src1;\n"
	"out vec4 col0;\n"
	"out vec4 col1;\n"
	"void main() {\n"
	"        col0 = src0;\n"
	"        col1 = src1;\n"
	"}\n"
	;

static void
create_fbo(void)
{
	GLuint rb[32];
	int i;

	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

	glGenRenderbuffersEXT(max_ds_buffers, rb);
	check_error(__LINE__);

	for (i = 0; i < max_ds_buffers; i++) {
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rb[i]);
		check_error(__LINE__);

		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
					     GL_COLOR_ATTACHMENT0 + i,
					     GL_RENDERBUFFER_EXT,
					     rb[i]);
		check_error(__LINE__);

		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA,
					 piglit_width, piglit_height);
		check_error(__LINE__);
	}
}

static enum piglit_result
test(void)
{
	static const GLfloat dest_color[4] = { 0.75, 0.25, 0.25, 0.5 };
	static const GLfloat test_color[4] = { 1.0, 0.25, 0.75, 0.25 };
	static const GLfloat test_color1[4] = { 0.5, 0.5, 0.5, 0.5 };
	GLfloat expected[4];
	GLuint prog;
	GLuint vs;
	GLuint fs;
	int i, j, k, o;

	if (max_ds_buffers > 1) {
		printf("Test only supports 1 dual source blending color buffer\n");
		max_ds_buffers = 1;
	}

	prog = glCreateProgram();
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);

	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	piglit_check_gl_error(GL_NO_ERROR);

	glBindFragDataLocationIndexed(prog, 0, 0, "col0");
	glBindFragDataLocationIndexed(prog, 0, 1, "col1");

	create_fbo();

	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

	glLinkProgram(prog);
	glUseProgram(prog);

	uniform_src0 = glGetUniformLocation(prog, "src0");
	uniform_src1 = glGetUniformLocation(prog, "src1");

	/* Setup blend modes and compute expected result color.
	 * We only test two simple blending modes.  A more elaborate
	 * test would exercise a much wider variety of modes.
	 */

	for (o = 0; o < ARRAY_SIZE(operators); o++) {
		for (i = 0; i < ARRAY_SIZE(srcFactors); i++) {
			for (j = 0; j < ARRAY_SIZE(dstFactors); j++) {
				blend_expected(expected, test_color, test_color1,
					       dest_color, srcFactors[i],
					       dstFactors[j], operators[o]);

				blend(test_color, test_color1, dest_color,
				      srcFactors[i], dstFactors[j],
				      operators[o]);
				for (k = 0; k < max_ds_buffers; k++) {
					glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + k);
					check_error(__LINE__);

					if (!piglit_probe_pixel_rgba(5, 5, expected)) {
						printf("For src/dst %d %d %d\n", i, j, o);
						return PIGLIT_FAIL;
					}
				}
			}
		}
	}
	return PIGLIT_PASS;
}

enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char**argv)
{
	enum piglit_result result;
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	piglit_require_extension("GL_ARB_blend_func_extended");

	glGetIntegerv(GL_MAX_DUAL_SOURCE_DRAW_BUFFERS, &max_ds_buffers);

	result = test();
	piglit_report_result(result);
}
