/*
 * Copyright © 2012 Intel Corporation
 * Copyright © 2020 Advanced Micro Devices, Inc.
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

#include "piglit-fbo.h"
using namespace piglit_util_fbo;

/**
 * \file nv_alpha_to_coverage_dither_control.cpp
 *
 * This file is inspired by the following file: -
 * tests\spec\ext_framebuffer_multisample\polygon-stipple.cpp
 *
 * This test case verifies the dithering functionality in the
 * context of alpha to coverage.
 *
 * This test operates  by clearing a multisample texture to black and
 * then drawing a white square onto it with alpha-to-coverage enabled, alpha
 * equal to 0.5 and dithering enabled/disabled. This texture is then passed to a
 * fragment shader which reads each sample of each fragment and determines its
 * partial derivative with respect to the adjacent fragments. If any of the
 * partial derivatives within a fragment is non-zero, thereby implying a
 * difference with the adjacent fragments, the fragment shader emits a red pixel
 * as output which is written to the window framebuffer at the pixel position
 * corresponding to that fragment. If the partial derivatives are all zero
 * thereby implying no difference with the adjacent fragment, a green pixel
 * is written.
 * The right half of the window framebuffer is always drawn with dithering
 * disabled, while the left half is drawn with dithering enabled or disabled
 * depending on the value of the "dither" parameter. Thus when the "visualize"
 * and "dither" parameters are set to 1(they are by default), the right half is
 * expected to be green and the left half is expected to be red. Only if the
 * number of samples is 0 are they both green.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 44;

	config.window_width = 512;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

const int pattern_width = 256; const int pattern_height = 256;

static Fbo ms_fbo, resolve_fbo;
static GLint num_samples;
static GLint enable_dither;
static GLint visualize;
static GLbitfield buffer_to_test;
static GLuint vertexbuffer;
static GLuint quadbuffer;
static GLuint elementbuffer;

static const float bg_color[4] =
	{0.0, 0.0, 0.0, 1.0};

GLfloat alphaValue = 0.5;

static const GLfloat g_vertex_buffer_data[] = {
	-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, alphaValue,
	-1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f, alphaValue,
	 1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f, alphaValue,
	-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, alphaValue,
	 1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f, alphaValue,
	 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, alphaValue
};

static const GLfloat g_quad_buffer_data[] = {
	-1.0f, -1.0f, 0.0f,
	-1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f, 0.0f,
	 1.0f, -1.0f, 0.0f,
};

GLint indices[] = {0,1,2,0,2,3};


static GLint msprog;
static GLint mstossprog;

static const char *msvert =
	"#version 440\n"
        "layout(location = 0) in vec3 vertexPosition;\n"
        "layout(location = 1) in vec4 vertexColor;\n"
        "uniform float alphaValue;\n"
        "out vec4 fragmentColor;\n"
	"void main()\n"
	"{\n"
	"  gl_Position = vec4(vertexPosition, 1.0);\n"
        "  fragmentColor = vec4(vertexColor.rgb,alphaValue);\n"
	"}\n";

static const char *msfrag =
	"#version 440\n"
        "layout(location = 0) out vec4 color;\n"
        "in vec4 fragmentColor;\n"
        "void main()\n"
        "{\n"
	"   color = fragmentColor;\n"
        "}\n";

static const char *mstossvert =
	"#version 440\n"
        "layout(location = 0) in vec3 vertexPosition;\n"
        "out vec2 UV;\n"
        "void main(){\n"
	"  gl_Position = vec4(vertexPosition, 1.0);\n"
	"  UV = vertexPosition.xy;\n"
	"}\n";

static const char *mstossfrag =
	"#version 440\n"
        "out vec4 fragmentColor;\n"
        "in vec2 UV;\n"
        "// Sampler variable\n"
        "uniform sampler2DMS multiSampleSampler;\n"
        "// Control if output is absolute value or is based on dFdx/y values\n"
        "uniform bool isAbsolute;\n"
        "bool isNotDifferent;\n"
        "uniform int numSamples;\n"
        "vec4 sampleColor;\n"
        "int i;\n"
        "void main()\n"
        "{\n"
	"        isNotDifferent = true;\n"
	"        // Using texel space coordinates\n"
	"        fragmentColor  = texelFetch( multiSampleSampler, ivec2(gl_FragCoord.xy), 0 ).rgba;\n"
	"        if ((dFdx(fragmentColor.rgba) != vec4(0.0,0.0,0.0,0.0)) || (dFdy(fragmentColor.rgba) != vec4(0.0,0.0,0.0,0.0)))\n"
	"           isNotDifferent = false;\n"
	"        for (i = 1; i < numSamples; i++) {\n"
	"          sampleColor  = texelFetch( multiSampleSampler, ivec2(gl_FragCoord.xy), i ).rgba;\n"
	"          if ((dFdx(sampleColor.rgba) != vec4(0.0,0.0,0.0,0.0)) || (dFdy(sampleColor.rgba) != vec4(0.0,0.0,0.0,0.0)))\n"
	"             isNotDifferent = false;\n"
	"          fragmentColor  += sampleColor;\n"
	"        }\n"
	"        if (numSamples != 0)\n"
	"           fragmentColor /= numSamples;\n"
	"        if (!isAbsolute) {\n"
	"          if (!isNotDifferent)\n"
	"             fragmentColor = vec4(1,0,0,1);\n"
	"          else\n"
	"             fragmentColor = vec4(0,1,0,1);\n"
	"        }\n"
	"        else\n"
	"          fragmentColor = fragmentColor;\n"
	"}\n";

void
shader_compile()
{
	/* Compile ms program */
	GLint msvs = piglit_compile_shader_text(GL_VERTEX_SHADER, msvert);
	GLint msfs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, msfrag);
	msprog = piglit_link_simple_program(msvs, msfs);

	if (!piglit_link_check_status(msprog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Compile mstoss program */
	GLint mstossvs = piglit_compile_shader_text(GL_VERTEX_SHADER, mstossvert);
	GLint mstossfs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, mstossfrag);
	mstossprog = piglit_link_simple_program(mstossvs, mstossfs);

	if (!piglit_link_check_status(msprog)) {
		piglit_report_result(PIGLIT_FAIL);
	}
}

void
draw_pattern(GLint mode)
{
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        if(mode == 1) {
          glAlphaToCoverageDitherControlNV(GL_ALPHA_TO_COVERAGE_DITHER_ENABLE_NV);
        }
        else {
          glAlphaToCoverageDitherControlNV(GL_ALPHA_TO_COVERAGE_DITHER_DISABLE_NV);
        }

        glUseProgram(msprog);
	GLfloat alphaValue_loc = glGetUniformLocation(msprog, "alphaValue");
	glUniform1f(alphaValue_loc, alphaValue);

	/* 1st attribute buffer : vertices */
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		(sizeof(GL_FLOAT) * 7),
		(void*)0
	);

	/* 2nd attribute buffer : colors */
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		1,
		4,
		GL_FLOAT,
		GL_FALSE,
		(sizeof(GL_FLOAT) * 7),
		(void*)(sizeof(GL_FLOAT) * 3)
	);

	glClearColor(bg_color[0], bg_color[1],
		     bg_color[2], bg_color[3]);
	glClear(buffer_to_test);

	/* Draw the quad */
        glDrawArrays(GL_TRIANGLES, 0, 2*3);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	/* Bind to resolve_fbo */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolve_fbo.handle);
	resolve_fbo.set_viewport();

	/* Use mstossprog shader */
        glUseProgram(mstossprog);
	/* Activate Texture Unit 0 */
	glActiveTexture(GL_TEXTURE0);
	/* Bind the mstexture Texture Unit 0; already done */

	GLuint texID = glGetUniformLocation(mstossprog, "multiSampleSampler");
	/* Set our sampler to use Texture Unit 0 */
	glUniform1i(texID, 0);

	GLint numSamples = glGetUniformLocation(mstossprog, "numSamples");
	glUniform1i(numSamples, num_samples);

	GLboolean isAbsolute = glGetUniformLocation(mstossprog, "isAbsolute");
        if(!visualize)
	   glUniform1i(isAbsolute, 1);
        else
	   glUniform1i(isAbsolute, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, quadbuffer);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		(sizeof(GL_FLOAT) * 3),
		(void*)0
	);

	GLint indices[] = {0,1,2,0,2,3};
	/* Generate and bind buffer for the indices */
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	/* Draw the quad */
	glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, NULL);

	glDisableVertexAttribArray(0);
}

bool
test_dither_control()
{
	bool result = true;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_fbo.handle);
	ms_fbo.set_viewport();
	draw_pattern(enable_dither);

	/* Blit resolve_fbo to the left half of window system framebuffer.
	 * This is the test image.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, resolve_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  buffer_to_test, GL_NEAREST);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	/* Check that the left half is red and the right half is green */
	if(enable_dither & visualize & (num_samples > 0)) {
		const float expected_right[4] = {0.0, 1.0, 0.0, 1.0};
		if (!piglit_probe_rect_rgba(pattern_width, 0, pattern_width, pattern_height, expected_right))
			result = false;
		const float expected_left[4] = {1.0, 0.0, 0.0, 1.0};
		if (!piglit_probe_rect_rgba(0, 0, pattern_width, pattern_height, expected_left))
			result = false;
	}
	/* Check that both the halves are green */
	else if(enable_dither & visualize & (num_samples = 0)) {
		const float expected[4] = {0.0, 1.0, 0.0, 1.0};
		if (!piglit_probe_rect_rgba(pattern_width, 0, pattern_width, pattern_height, expected))
			result = false;
		if (!piglit_probe_rect_rgba(0, 0, pattern_width, pattern_height, expected))
			result = false;
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	result = piglit_check_gl_error(GL_NO_ERROR) && result;
	return result;
}

void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples> enable_dither(1|0) visualize(1|0)\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	char *endptr = NULL;

        if (argc == 2) {
		enable_dither = 1;
		visualize = 1;
		num_samples = strtol(argv[1], &endptr, 0);
		if (endptr != argv[1] + strlen(argv[1]))
			print_usage_and_exit(argv[0]);
	}
	else if (argc < 4)
		print_usage_and_exit(argv[0]);
	else {
		num_samples = strtol(argv[1], &endptr, 0);
		if (endptr != argv[1] + strlen(argv[1]))
			print_usage_and_exit(argv[0]);
		enable_dither = strtol(argv[2], &endptr, 0);
                if (endptr != argv[2] + strlen(argv[2]))
			print_usage_and_exit(argv[0]);
		if ((enable_dither > 1) | (enable_dither < 0))
			print_usage_and_exit(argv[0]);
		visualize = strtol(argv[3], &endptr, 0);
                if (endptr != argv[3] + strlen(argv[3]))
			print_usage_and_exit(argv[0]);
		if ((visualize > 1) | (visualize < 0))
			print_usage_and_exit(argv[0]);
	}

	piglit_require_gl_version(44);
	piglit_require_extension("GL_NV_alpha_to_coverage_dither_control");

	piglit_ortho_projection(pattern_width, pattern_height, GL_TRUE);

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	glGenBuffers(1, &quadbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quadbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_buffer_data), g_quad_buffer_data, GL_STATIC_DRAW);

	/* Generate and bind buffer for the indices */
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	FboConfig temp_config = FboConfig(num_samples, pattern_width, pattern_height);
        temp_config.num_tex_attachments = 1;
	ms_fbo.setup(temp_config);
	resolve_fbo.setup(FboConfig(0, pattern_width, pattern_height));

	buffer_to_test = GL_COLOR_BUFFER_BIT;
	shader_compile();
}

enum piglit_result
piglit_display()
{
	bool pass = true;
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw test pattern to the ms fbo
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_fbo.handle);
	ms_fbo.set_viewport();
	draw_pattern(0);

	/* Blit resolve_fbo to the right half of window system framebuffer. This
	 * is a reference image.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, resolve_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  pattern_width, 0, 2 * pattern_width, pattern_height,
			  buffer_to_test, GL_NEAREST);

	/* Test with ALPHA_TO_COVERAGE_DITHER_ENABLE */
	pass = test_dither_control() && pass;

	if (!piglit_automatic &&
	    buffer_to_test != GL_DEPTH_BUFFER_BIT)
		piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
