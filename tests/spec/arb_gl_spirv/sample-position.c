/*
 * Copyright © 2018 Intel Corporation
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

#include "piglit-util-gl.h"

/**
 * @file sample-position.c
 *
 * Tests whether setting OriginUpperLeft and OriginLowerLeft affects
 * the SamplePosition builtin. It draws a grid of rectangles into a
 * multi-sample framebuffer where each rectangle only covers the top
 * half of the pixel. The fragment shaders store the sample position
 * in the buffer. It then uses multisample texturing to read back the
 * samples. Any samples written by the fragment shader should have the
 * y position all greater than 0.5. It’s not clear whether changing
 * the origin should affect this and there is an open spec issue about
 * it, but for now this test assumes that it shouldn’t.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 33;

PIGLIT_GL_TEST_CONFIG_END

#define TEX_SAMPLES 4
#define STR(x) #x
#define STRINGIFY(x) STR(x)

static GLuint tex;
static GLuint fb;
static GLuint spirv_prog;
static GLuint combine_prog;
static GLuint rectangles_vao;
static GLuint rectangles_vbo;

static const char
spirv_vert_shader_source[] =
	"               OpCapability Shader\n"
	"               OpMemoryModel Logical GLSL450\n"
	"               OpEntryPoint Vertex %main \"main\" %pos_in %pos_out\n"
	"               OpDecorate %pos_in Location 0\n"
	"               OpDecorate %pos_out BuiltIn Position\n"
	"       %void = OpTypeVoid\n"
	"  %func_type = OpTypeFunction %void\n"
	"      %float = OpTypeFloat 32\n"
	"    %v4float = OpTypeVector %float 4\n"
	"%_ptr_Input_v4float = OpTypePointer Input %v4float\n"
	"%_ptr_Output_v4float = OpTypePointer Output %v4float\n"
	"     %pos_in = OpVariable %_ptr_Input_v4float Input\n"
	"    %pos_out = OpVariable %_ptr_Output_v4float Output\n"
	"       %main = OpFunction %void None %func_type\n"
	" %main_label = OpLabel\n"
	" %pos_in_val = OpLoad %v4float %pos_in\n"
	"               OpStore %pos_out %pos_in_val\n"
	"               OpReturn\n"
	"               OpFunctionEnd\n";

static const char
spirv_frag_shader_source[] =
	/* Copies gl_SamplePosition to output color 0 */
	"               OpCapability Shader\n"
	"               OpCapability SampleRateShading\n"
	"               OpMemoryModel Logical GLSL450\n"
	"               OpEntryPoint Fragment %main \"main\" "
	"                            %color_out %gl_SamplePosition\n"
	"               OpExecutionMode %main {}\n"
	"               OpDecorate %color_out Location 0\n"
	"               OpDecorate %gl_SamplePosition BuiltIn SamplePosition\n"
	"       %void = OpTypeVoid\n"
	"          %3 = OpTypeFunction %void\n"
	"      %float = OpTypeFloat 32\n"
	"    %v4float = OpTypeVector %float 4\n"
	"%_ptr_Output_v4float = OpTypePointer Output %v4float\n"
	"  %color_out = OpVariable %_ptr_Output_v4float Output\n"
	"    %v2float = OpTypeVector %float 2\n"
	"%_ptr_Input_v2float = OpTypePointer Input %v2float\n"
	"%gl_SamplePosition = OpVariable %_ptr_Input_v2float Input\n"
	"    %float_0 = OpConstant %float 0\n"
	"    %float_1 = OpConstant %float 1\n"
	"       %main = OpFunction %void None %3\n"
	"          %5 = OpLabel\n"
	"         %13 = OpLoad %v2float %gl_SamplePosition\n"
	"         %16 = OpCompositeExtract %float %13 0\n"
	"         %17 = OpCompositeExtract %float %13 1\n"
	"         %18 = OpCompositeConstruct %v4float %16 %17 "
	"                                    %float_0 %float_1\n"
	"               OpStore %color_out %18\n"
	"               OpReturn\n"
	"               OpFunctionEnd\n";

static const char
combine_vert_shader_source[] =
	"#version 330\n"
	"\n"
	"uniform vec2 fb_size;\n"
	"layout(location = 0) in vec2 piglit_vertex;\n"
	"out vec2 tex_coord;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"        gl_Position = vec4(piglit_vertex, 0.0, 1.0);\n"
	"        tex_coord = (piglit_vertex + 1.0) / 2.0 * fb_size;\n"
	"}\n";

static const char
combine_frag_shader_source[] =
	"#version 330\n"
	"#extension GL_ARB_texture_multisample: require\n"
	"\n"
	"uniform sampler2DMS tex;\n"
	"in vec2 tex_coord;\n"
	"layout(location = 0) out vec4 color_out;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"        int count = 0;\n"
	"        bool pass = true;\n"
	"        ivec2 itex_coord = ivec2(floor(tex_coord));\n"
	"\n"
	"        for (int i = 0; i < " STRINGIFY(TEX_SAMPLES) "; i++) {\n"
	"                vec4 v = texelFetch(tex, itex_coord, i);\n"
	"                if (v.z < 0.5) {\n"
	"                        count++;\n"
	"                        if (v.y <= 0.5)\n"
	"                                pass = false;\n"
	"                }\n"
	"        }\n"
	"        if (pass && count > 0)\n"
	"                color_out = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"        else\n"
	"                color_out = vec4(1.0, count / 255.0, 0.0, 1.0);\n"
	"}\n";

static GLuint
compile_spirv_shader(GLenum target,
		     const char *source)
{
	GLuint shader = piglit_assemble_spirv(target,
					      strlen(source),
					      source);
	glSpecializeShaderARB(shader,
			      "main",
			      0, /* num specializations */
			      NULL, /* constant index */
			      NULL /* constant value */);

	return shader;
}

static GLuint
compile_spirv_program(const char *vert_source,
		      const char *frag_source)
{
	GLuint vert_shader =
		compile_spirv_shader(GL_VERTEX_SHADER, vert_source);
	GLuint frag_shader =
		compile_spirv_shader(GL_FRAGMENT_SHADER, frag_source);
	GLuint prog = piglit_link_simple_program(vert_shader, frag_shader);

	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);

	if (prog == 0)
		piglit_report_result(PIGLIT_FAIL);

	return prog;
}

static void
create_rectangles_vao(void)
{
	struct vertex { float x, y; };
	struct vertex *buffer, *p;
	size_t buffer_size = piglit_width * piglit_height * 6 * sizeof *buffer;

	/* Create a buffer with 2 triangles to form a quad for each
	 * pixel. Each quad only covers the top half of the pixel
	 * without touching the center
	 */
	buffer = malloc(buffer_size);

	p = buffer;
	for (int y = 0; y < piglit_height; y++) {
		float y1 = (y + 0.501f) * 2.0f / piglit_height - 1.0f;
		float y2 = y1 + 0.499f * 2.0f / piglit_height;

		for (int x = 0; x < piglit_width; x++) {
			float x1 = x * 2.0f / piglit_width - 1.0f;
			float x2 = x1 + 2.0f / piglit_width;

			p[0].x = x1; p[0].y = y1;
			p[1].x = x2; p[1].y = y1;
			p[2].x = x1; p[2].y = y2;

			p[3] = p[2];
			p[4] = p[1];
			p[5].x = x2; p[5].y = y2;

			p += 6;
		}
	}

	assert((uint8_t *) p - (uint8_t *) buffer == buffer_size);

	glGenBuffers(1, &rectangles_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, rectangles_vbo);
	glBufferData(GL_ARRAY_BUFFER,
		     buffer_size,
		     buffer,
		     GL_STATIC_DRAW);

	free(buffer);

	glGenVertexArrays(1, &rectangles_vao);
	glBindVertexArray(rectangles_vao);
	glVertexAttribPointer(0, /* index */
			      2, /* size */
			      GL_FLOAT,
			      GL_FALSE, /* normalized */
			      sizeof buffer[0],
			      NULL /* pointer */);
	glEnableVertexAttribArray(0);
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_texture_multisample");
	piglit_require_extension("GL_ARB_gl_spirv");
	piglit_require_extension("GL_ARB_sample_shading");

	if (argc <= 1) {
		fprintf(stderr,
			"usage: %s OriginUpperLeft|OriginLowerLeft\n",
			argv[0]);
		piglit_report_result(PIGLIT_FAIL);
	}
	const char *origin = argv[1];

	/* We need to support multisample textures with at least 4
	 * samples
	 */
	int samples;
	glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &samples);
	if (samples < TEX_SAMPLES) {
		printf("At least %i texture samples are required but only %i "
		       "are allowed\n",
		       TEX_SAMPLES,
		       samples);
		piglit_report_result(PIGLIT_SKIP);
	}

	/* Create a multisample texture */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
				TEX_SAMPLES,
				GL_RGBA8,
				piglit_width,
				piglit_height,
				GL_FALSE /* fixedsamplelocations */);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glFramebufferTexture2D(GL_FRAMEBUFFER,
			       GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D_MULTISAMPLE,
			       tex,
			       0 /* level */);

	GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (result != GL_FRAMEBUFFER_COMPLETE) {
		printf("multisample framebuffer is incomplete\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	const char *stub_pos = strstr(spirv_frag_shader_source, "{}");
	char *frag_source = malloc(sizeof spirv_frag_shader_source +
				   strlen(origin));
	memcpy(frag_source,
	       spirv_frag_shader_source,
	       stub_pos - spirv_frag_shader_source);
	strcpy(frag_source + (stub_pos - spirv_frag_shader_source), origin);
	strcpy(frag_source +
	       (stub_pos - spirv_frag_shader_source) +
	       strlen(origin),
	       stub_pos + 2);
	spirv_prog = compile_spirv_program(spirv_vert_shader_source,
					   frag_source);
	free(frag_source);

	create_rectangles_vao();

	combine_prog = piglit_build_simple_program(combine_vert_shader_source,
						   combine_frag_shader_source);
	glUseProgram(combine_prog);
	GLuint fb_size_loc = glGetUniformLocation(combine_prog, "fb_size");
	glUniform2f(fb_size_loc, piglit_width, piglit_height);
	GLuint tex_loc = glGetUniformLocation(combine_prog, "tex");
	glUniform1i(tex_loc, 0);
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	glClearColor(0.0, 0.0, 1.0, 1.0);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(spirv_prog);

	glBindVertexArray(rectangles_vao);
	glDrawArrays(GL_TRIANGLES,
		     0, /* first */
		     piglit_width * piglit_height * 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	glUseProgram(combine_prog);
	piglit_draw_rect(-1, -1, 2, 2);

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      (float[]) { 0.0f, 1.0f, 0.0f, 1.0f });

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
