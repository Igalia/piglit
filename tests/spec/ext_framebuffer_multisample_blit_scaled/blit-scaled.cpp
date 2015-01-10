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

/** \file blit-scaled.cpp
 *
 * This test verifies the accuracy of scaled blitting from a multisampled
 * buffer to a single-sampled buffer by comparing the output from following
 * rendering scenarios:
 * 1. Scaled blit using EXT_multisample_framebuffer_blit_scaled.
 * 2. Scaled blit using glsl shader program.
 *
 * Note: This test is specific to Intel's implementation of extension
 * EXT_multisample_framebuffer_blit_scaled and may not produce expected
 * results on other hardware. Currently test passes with all of the scaling
 * factors between 0.1 to 2.5 on Intel's i965 drivers and NVIDIA's proprietary
 * linux drivers.
 */

#include "piglit-test-pattern.h"
#include "piglit-fbo.h"
using namespace piglit_util_fbo;
using namespace piglit_util_test_pattern;

const int pattern_width = 258; const int pattern_height = 258;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = pattern_width * 2;
	config.window_height = pattern_height;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static int samples;
static TestPattern *test_pattern;
static unsigned prog, vao, vertex_buf;
const float srcX0 = 6, srcY0 = 7, dstX0 = 0, dstY0 = 0;
const float srcX1 = pattern_width / 2, srcY1 = pattern_height / 2;
static Fbo multisampled_tex, multisampled_fbo, singlesampled_fbo;

static void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples>\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
compile_shader(void)
{
	static const char *vert =
		"#version 130\n"
		"in vec2 pos;\n"
		"in vec2 texCoord;\n"
		"out vec2 textureCoord;\n"
		"void main()\n"
		"{\n"
		"  gl_Position = vec4(pos, 0.0, 1.0);\n"
		"  textureCoord = texCoord;\n"
		"}\n";
	/* Bilinear filtering of samples using shader program */
	static const char *frag_template =
		"#version 130\n"
		"#extension GL_ARB_texture_multisample : require\n"
		"in vec2 textureCoord;\n"
		"uniform sampler2DMS texSampler;\n"
		"uniform float src_width;\n"
		"uniform float src_height;\n"
		"out vec4 out_color;\n"
		"void main()\n"
		"{\n"
		"%s"
		"  float x_f, y_f;\n"
		"  const float x_scale = 2.0f, x_scale_inv = 0.5f;\n"
		"  const float y_scale = %ff, y_scale_inv = %ff;\n"
		"  const float x_offset = 0.25f, y_offset = %ff;\n"
		"  vec2 s_0_coord, s_1_coord, s_2_coord, s_3_coord;\n"
		"  vec4 s_0_color, s_1_color, s_2_color, s_3_color;\n"
		"  vec4 x_0_color, x_1_color;\n"
		"\n"
		"  vec2 tex_coord = vec2(textureCoord.x - x_offset,\n"
		"                        textureCoord.y - y_offset);\n"
		"  tex_coord = vec2(x_scale * tex_coord.x, y_scale * tex_coord.y);\n"
		"\n"
		"  clamp(tex_coord.x, 0.0f, x_scale * src_width - 1.0f);\n"
		"  clamp(tex_coord.y, 0.0f, y_scale * src_height - 1.0f);\n"
		"\n"
		"  x_f = fract(tex_coord.x);\n"
		"  y_f = fract(tex_coord.y);\n"
		"\n"
		"  tex_coord.x = int(tex_coord.x) * x_scale_inv;\n"
		"  tex_coord.y = int(tex_coord.y) * y_scale_inv;\n"
		"\n"
		"\n"
		"  /* Compute the sample coordinates used for filtering. */\n"
		"  s_0_coord = tex_coord;\n"
		"  s_1_coord = tex_coord + vec2(x_scale_inv, 0.0f);\n"
		"  s_2_coord = tex_coord + vec2(0.0f, y_scale_inv);\n"
		"  s_3_coord = tex_coord + vec2(x_scale_inv, y_scale_inv);\n"
		"\n"
		"  /* Fetch sample color values. */\n"
		"%s"
		"  s_0_color = TEXEL_FETCH(s_0_coord)\n"
		"  s_1_color = TEXEL_FETCH(s_1_coord)\n"
		"  s_2_color = TEXEL_FETCH(s_2_coord)\n"
		"  s_3_color = TEXEL_FETCH(s_3_coord)\n"
		"#undef TEXEL_FETCH\n"
		"\n"
		"  /* Do bilinear filtering on sample colors. */\n"
		"  x_0_color =  mix(s_0_color, s_1_color, x_f);\n"
		"  x_1_color =  mix(s_2_color, s_3_color, x_f);\n"
		"  out_color = mix(x_0_color, x_1_color, y_f);\n"
		"}\n";

	char* frag, *texel_fetch_macro;
	const char*sample_number, *sample_map = "";
	const float y_scale = samples * 0.5;

	/* Below switch is used to setup the shader expression, which computes
	 * sample index and map it to to a sample number on Intel hardware.
	 * Sample index layout shows the numbering of slots in a rectangular
	 * grid of samples with in a pixel. Sample number layout shows the
	 * rectangular grid of samples roughly corresponding to the real sample
	 * locations with in a pixel. Sample number layout matches the sample
	 * index layout in case of 2X and 4x MSAA, but they are different in
	 * case of 8X MSAA.
	 *
	 * 2X MSAA sample index / number layout
	 *           ---------
	 *           | 0 | 1 |
	 *           ---------
	 *
	 * 4X MSAA sample index / number layout
	 *           ---------
	 *           | 0 | 1 |
	 *           ---------
	 *           | 2 | 3 |
	 *           ---------
	 *
	 * 8X MSAA sample index layout    8x MSAA sample number layout
	 *           ---------                      ---------
	 *           | 0 | 1 |                      | 5 | 2 |
	 *           ---------                      ---------
	 *           | 2 | 3 |                      | 4 | 6 |
	 *           ---------                      ---------
	 *           | 4 | 5 |                      | 0 | 3 |
	 *           ---------                      ---------
	 *           | 6 | 7 |                      | 7 | 1 |
	 *           ---------                      ---------
	 */
	switch(samples) {
	case 2:
		sample_number =  "int(2 * fract(coord.x))";
		break;
	case 4:
		sample_number =  "int(2 * fract(coord.x) + 4 * fract(coord.y))";
	break;
	case 8:
		sample_map = "  const int sample_map[8] = int[8](5 , 2, 4, 6, 0, 3, 7, 1);\n";
		sample_number =  "sample_map[int(2 * fract(coord.x) + 8 * fract(coord.y))]";
		break;
	default:
		printf("Unsupported sample count %d\n", samples);
		piglit_report_result(PIGLIT_SKIP);
	}

	asprintf(&texel_fetch_macro,
		 "#define TEXEL_FETCH(coord) texelFetch(texSampler, ivec2(coord), %s);\n",
		 sample_number);

	asprintf(&frag, frag_template, sample_map, y_scale, 1.0f / y_scale,
		 1.0f / samples, texel_fetch_macro);

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
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float),
			      (void *) 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float),
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
ms_blit_scaled_glsl(const Fbo *src_fbo)
{
	float vertex_data[4][4] = {
		{ -1, -1, srcX0, srcY0 },
		{ -1,  1, srcX0, srcY1 },
		{  1,  1, srcX1, srcY1 },
		{  1, -1, srcX1, srcY0 }};

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, src_fbo->color_tex[0]);
	glUseProgram(prog);
	glBindVertexArray(vao);

	/* Set up uniforms */
	glUseProgram(prog);
	glUniform1i(glGetUniformLocation(prog, "texSampler"), 0);
	glUniform1f(glGetUniformLocation(prog, "src_width"),
		    multisampled_fbo.config.width);
	glUniform1f(glGetUniformLocation(prog, "src_height"),
		    multisampled_fbo.config.height);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data,
		     GL_STREAM_DRAW);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *) 0);
}

void
piglit_init(int argc, char **argv)
{
	if (argc != 2)
		print_usage_and_exit(argv[0]);

	/* 1st arg: num_samples */
	char *endptr = NULL;
	int num_samples = strtol(argv[1], &endptr, 0);
	if (endptr != argv[1] + strlen(argv[1]))
		print_usage_and_exit(argv[0]);

	piglit_require_gl_version(21);
	piglit_require_extension("GL_ARB_vertex_array_object");
	piglit_require_extension("GL_EXT_framebuffer_multisample_blit_scaled");

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples == 0 || num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	singlesampled_fbo.setup(FboConfig(0,
					  2 * pattern_width,
					  pattern_height));
	/* Create two multisample FBOs with same dimensions and sample count
	 * but different color attachment types.
	 */
	FboConfig msConfig(num_samples, pattern_width, pattern_height);
	multisampled_fbo.setup(msConfig);
	msConfig.num_tex_attachments = 1;
	msConfig.num_rb_attachments = 0; /* default value is 1 */
	multisampled_tex.setup(msConfig);

	/* Implementation might not create a buffer with requested sample
	 * count. So, query the actual sample count of buffer.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampled_tex.handle);
	glGetIntegerv(GL_SAMPLES, &samples);

	test_pattern = new Triangles();
	test_pattern->compile();

	compile_shader();
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}
}

bool test_ms_blit_scaled(const Fbo& ms_fbo)
{
	GLfloat scale;
	bool pass = true, result = true;

	/* Draw the test pattern into the framebuffer with texture
	 * attachment.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampled_tex.handle);
	glViewport(0, 0, srcX1, srcY1);
	glClear(GL_COLOR_BUFFER_BIT);
	test_pattern->draw(TestPattern::no_projection);

	if(ms_fbo.config.num_tex_attachments == 0) {
		/* Blit the framebuffer with multisample texture attachment
		 * into the framebuffer with multisample renderbuffer attachment.
		 */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampled_tex.handle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampled_fbo.handle);
		glClear(GL_COLOR_BUFFER_BIT);
		glBlitFramebuffer(0, 0,
				  multisampled_tex.config.width,
				  multisampled_tex.config.height,
				  0, 0,
				  multisampled_tex.config.width,
				  multisampled_tex.config.height,
				  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

        for(scale = 0.1; scale < 2.5f; scale += 0.1) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glClear(GL_COLOR_BUFFER_BIT);

		/* Do scaled resolve of multisampled_fbo to left half of
		 * singlesampled_fbo.
		 */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, ms_fbo.handle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, singlesampled_fbo.handle);
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
				  GL_SCALED_RESOLVE_FASTEST_EXT);
		glDisable(GL_SCISSOR_TEST);

		/* Use multisampled texture to draw in to right half of scaled
		 * single-sampled buffer using shader program.
		 */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampled_tex.handle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, singlesampled_fbo.handle);
		glViewport(pattern_width + dstX0, dstY0, srcX1 * scale, srcY1 * scale);
		ms_blit_scaled_glsl(&multisampled_tex);

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
                glBindFramebuffer(GL_READ_FRAMEBUFFER, singlesampled_fbo.handle);
                result = piglit_probe_rect_halves_equal_rgba(0, 0,
                                                           piglit_width,
                                                           piglit_height);
		pass = result && pass;

		glBindFramebuffer(GL_READ_FRAMEBUFFER, singlesampled_fbo.handle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glBlitFramebuffer(0, 0, 2 * pattern_width, piglit_height,
				  0, 0, 2 * pattern_width, piglit_height,
				  GL_COLOR_BUFFER_BIT, GL_NEAREST);
		piglit_present_results();
		printf("MS attachment = %12s, scale = %f, result = %s\n",
		       ms_fbo.config.num_tex_attachments > 0 ?
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
	printf("Left Image: multisample scaled blit using extension.\n"
	       "Right Image: multisample scaled blit using shader program.\n");
	pass = test_ms_blit_scaled(multisampled_tex)
               && pass;
	pass = test_ms_blit_scaled(multisampled_fbo)
               && pass;
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
