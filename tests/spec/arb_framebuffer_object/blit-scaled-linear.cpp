/*
 * Copyright © 2013 Intel Corporation
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

/** \file blit-scaled-linear.cpp
 *
 * This test verifies the accuracy of scaled blitting from a single sample
 * buffer with GL_LINEAR filter.It compares the output from following
 * rendering scenarios:
 * 1. Scaled blit using a framebuffer with texture/renderbuffer attachment.
 * 2. Scaled blit using glsl shader program.
 */

#include "piglit-test-pattern.h"
#include "piglit-fbo.h"
using namespace piglit_util_fbo;
using namespace piglit_util_test_pattern;
const int pattern_width = 258; const int pattern_height = 258;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;

	config.window_width = pattern_width * 2;
	config.window_height = pattern_height;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static TestPattern *test_pattern;
static unsigned prog, vao, vertex_buf;
const int srcX0 = 0, srcY0 = 0, dstX0 = 0, dstY0 = 0;
const int srcX1 = pattern_width / 2, srcY1 = pattern_height / 2;
static Fbo fbo_tex, fbo_rb;

void
compile_shader(void)
{
	static const char *vert =
		"#version 130\n"
		"uniform mat4 proj;\n"
		"in vec2 pos;\n"
		"in vec2 texCoord;\n"
		"out vec2 textureCoord;\n"
		"void main()\n"
		"{\n"
		"  gl_Position = proj * vec4(pos, 0.0, 1.0);\n"
		"  textureCoord = texCoord;\n"
		"}\n";
	/* Bilinear filtering of samples using shader program */
	static const char *frag =
		"#version 130\n"
		"#extension GL_ARB_texture_rectangle : enable\n"
		"in vec2 textureCoord;\n"
		"uniform sampler2DRect tex2d;\n"
		"uniform float xmax;\n"
		"uniform float ymax;\n"
		"out vec4 out_color;\n"
		"void main()\n"
		"{\n"
		"  vec2 f;\n"
		"  vec4 c0, c1, c2, c3;\n"
		"  vec2 tex_coord = textureCoord - vec2(0.5, 0.5);\n"
		"\n"
		"  tex_coord.xy = clamp(tex_coord.xy,\n"
		"                      vec2(0.0, 0.0),\n"
		"                      vec2 (xmax - 1.0, ymax - 1.0));\n"
		"\n"
		"  f.x = fract(tex_coord.x);\n"
		"  f.y = fract(tex_coord.y);\n"
		"\n"
		"  tex_coord.x = tex_coord.x - f.x;\n"
		"  tex_coord.y = tex_coord.y - f.y;\n"
		"\n"
		"    c0 = texture2DRect(tex2d, tex_coord.xy + vec2(0, 0));\n"
		"    c1 = texture2DRect(tex2d, tex_coord.xy + vec2(1, 0));\n"
		"    c2 = texture2DRect(tex2d, tex_coord.xy + vec2(0, 1));\n"
		"    c3 = texture2DRect(tex2d, tex_coord.xy + vec2(1, 1));\n"
		"\n"
		"  vec4 color_x1 =  mix(c0, c1, f.x);\n"
		"  vec4 color_x2 =  mix(c2, c3, f.x);\n"
		"\n"
		"  out_color = mix(color_x1, color_x2, f.y);\n"
		"}\n";
	/* Compile program */
	prog = glCreateProgram();
	GLint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	glAttachShader(prog, vs);
	piglit_check_gl_error(GL_NO_ERROR);
	GLint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	glAttachShader(prog, fs);
	glBindAttribLocation(prog, 0, "pos");
	glBindAttribLocation(prog, 1, "texCoord");
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up vertex array object */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Set up vertex input buffer */
	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, 4*sizeof(float),
			      (void *) 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_INT, GL_FALSE, 4*sizeof(float),
			      (void *) (2*sizeof(float)));

	/* Set up element input buffer to tessellate a quad into
	 * triangles
	 */
	unsigned int indices[6] = { 0, 1, 2, 0, 2, 3 };
	GLuint element_buf;
	glGenBuffers(1, &element_buf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
		     GL_STATIC_DRAW);
}

void
blit_scaled_linear_glsl(const Fbo *src_fbo, GLint samples)
{
	const float proj[4][4] = {
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 }};

	int vertex_data[4][4] = {
		{ -1, -1, srcX0, srcY0 },
		{ -1,  1, srcX0, srcY1 },
		{  1,  1, srcX1, srcY1 },
		{  1, -1, srcX1, srcY0 }};

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, src_fbo->color_tex);
	glUseProgram(prog);
	glBindVertexArray(vao);

	/* Set up uniforms */
	glUseProgram(prog);
	glUniform1i(glGetUniformLocation(prog, "tex2d"), 0);
	glUniform1f(glGetUniformLocation(prog, "xmin"), 0);
	glUniform1f(glGetUniformLocation(prog, "ymin"), 0);
	glUniform1f(glGetUniformLocation(prog, "xmax"),
		    fbo_rb.config.width);
	glUniform1f(glGetUniformLocation(prog, "ymax"),
		    fbo_rb.config.height);
	glUniformMatrix4fv(glGetUniformLocation(prog, "proj"), 1,
                           GL_TRUE, &proj[0][0]);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data,
		     GL_STREAM_DRAW);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *) 0);
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(21);
	piglit_require_extension("GL_ARB_vertex_array_object");
	piglit_require_GLSL_version(130);

	/* Create two singlesample FBOs with same format and dimensions but
	 * different color attachment types.
	 */
	FboConfig Config(0, pattern_width / 2, pattern_height / 2);
	Config.attach_texture = true;
	fbo_tex.setup(Config);
	Config.attach_texture = false;
	fbo_rb.setup(Config);

	test_pattern = new Triangles();
	test_pattern->compile();

	compile_shader();
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}
}

bool test_blit_scaled_linear(Fbo fbo_test)
{
	GLfloat scale;
	GLint samples;
	bool pass = true, result = true;

	/* Draw the test pattern into the framebuffer with texture
	 * attachment.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_tex.handle);
	glViewport(0, 0, srcX1, srcY1);
	glGetIntegerv(GL_SAMPLES, &samples);
	glClear(GL_COLOR_BUFFER_BIT);
	test_pattern->draw(TestPattern::no_projection);

	if(!fbo_test.config.attach_texture) {
		/* Blit the framebuffer with texture attachment into the
		 * framebuffer with renderbuffer attachment.
		 */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_tex.handle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_rb.handle);
		glClear(GL_COLOR_BUFFER_BIT);
		glBlitFramebuffer(0, 0,
				  fbo_tex.config.width,
				  fbo_tex.config.height,
				  0, 0,
				  fbo_rb.config.width,
				  fbo_rb.config.height,
				  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

        for(scale = 0.1; scale < 2.5f; scale += 0.1) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glClear(GL_COLOR_BUFFER_BIT);

		/* Do scaled blit of fbo_rb to left half of piglit_winsys_fbo
                 * with GL_LINEAR filter.
		 */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_test.handle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
                glClearColor(0.0, 1.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
                glClearColor(0.0, 0.0, 0.0, 0.0);
		glEnable(GL_SCISSOR_TEST);
		glScissor(0, 0, pattern_width, pattern_height);
		glBlitFramebuffer(srcX0, srcY0,
				  srcX1, srcY1,
				  dstX0, dstY0,
				  dstX0 + srcX1 * scale, dstY0 + srcY1 * scale,
				  GL_COLOR_BUFFER_BIT,
				  GL_LINEAR);
		glDisable(GL_SCISSOR_TEST);

		/* Use fbo with texture attachment to blit in to right half of
		 * piglit_winsys_fbo using a glsl shader program for linear
		 * filtering.
		 */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_tex.handle);
		glViewport(pattern_width + dstX0, dstY0, srcX1 * scale, srcY1 * scale);
		blit_scaled_linear_glsl(&fbo_tex, samples);

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
		glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
		result = piglit_probe_rect_halves_equal_rgba(0, 0,
							     piglit_width,
							     piglit_height);
		pass = result && pass;
		piglit_present_results();
		printf("Attachment = %12s, scale = %f, result = %s\n",
		       fbo_test.config.attach_texture ?
		       "TEXTURE" :
		       "RENDERBUFFER",
		       scale, result ? "pass" : "fail");
	}
	return pass;
}

enum piglit_result
piglit_display()
{
	bool pass = true;
	printf("Left Image: Linear scaled blit using glBlitFramebuffer.\n"
	       "Right Image: Linear scaled blit using glsl.\n");
	pass = test_blit_scaled_linear(fbo_tex)
               && pass;
	pass = test_blit_scaled_linear(fbo_rb)
               && pass;
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
