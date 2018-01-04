/*
 * Copyright © 2017 Fabian Bieler
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
 *
 */

/** @file texture-halves-ping-pong-operation-chain.c
 *
 * From the GL_ARB_texture_barrier spec:
 *
 *  "Specifically, the values of rendered fragments are undefined if any
 *  shader stage fetches texels and the same texels are written via fragment
 *  shader outputs, even if the reads and writes are not in the same Draw
 *  call, unless any of the following exceptions apply:
 *
 *  - The reads and writes are from/to disjoint sets of texels (after
 *    accounting for texture filtering rules).
 *
 *  - There is only a single read and write of each texel, and the read is in
 *    the fragment shader invocation that writes the same texel (e.g. using
 *    "texelFetch2D(sampler, ivec2(gl_FragCoord.xy), 0);").
 *
 *  - If a texel has been written, then in order to safely read the result
 *    a texel fetch must be in a subsequent Draw separated by the command
 *
 *      void TextureBarrier(void);
 *
 *    TextureBarrier() will guarantee that writes have completed and caches
 *    have been invalidated before subsequent Draws are executed."
 *
 * This test aims to test points 1 and 3 of that statement.
 *
 * It uses an uint texture bound as the colorbuffer and texture sampler
 * source. It renders 6 passes.
 * Every pass one half of the texture is sampled from while the other is
 * written to. Which is which is swapped every pass.
 * The operations in the passes are chosen so that the end result will differ
 * if any pass is missing or the passes aren't executed in the correct order.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 31;
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static unsigned tex, render_pass_loc;

static int width;
static int height;

static const char *vs_text =
	"#version 130\n"
	"in vec4 piglit_vertex;"
	"void main() {\n"
	"	gl_Position = piglit_vertex;\n"
	"}\n";

static const char *fs_text =
	"#version 130\n"
	"uniform int render_pass;\n"
	"uniform usampler2D fb;\n"
	"out uvec4 color;\n"
	"void main() {\n"
	"	int x_ofs = textureSize(fb, 0).x / 2;\n"
	"	x_ofs *= render_pass % 2 != 0 ? -1 : 1;\n"
	"	ivec2 tex_coord = ivec2(gl_FragCoord.xy) + ivec2(x_ofs, 0);\n"
	"	uvec4 prev_color = texelFetch(fb, tex_coord, 0);\n"
	"	switch (render_pass) {\n"
	"	case 0:\n"
	"		color = uvec4(1);\n"
	"		break;\n"
	"	case 1:\n"
	"		color = prev_color * 2u;\n"
	"		break;\n"
	"	case 2:\n"
	"		color = prev_color + 2u;\n"
	"		break;\n"
	"	case 3:\n"
	"		color = prev_color ^ 1023u;\n"
	"		break;\n"
	"	case 4:\n"
	"		color = prev_color << 2u;\n"
	"		break;\n"
	"	case 5:\n"
	"		color = prev_color % 32u;\n"
	"		break;\n"
	"	}\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = 0; true; ++i) {
		glUniform1i(render_pass_loc, i);
		if (i%2)
			piglit_draw_rect(0, -1, 1, 2);
		else
			piglit_draw_rect(-1, -1, 1, 2);
		if (i >= 5)
			break;
		glTextureBarrier();
	}

	unsigned color = 1;
	color *= 2;
	color += 2;
	color ^= 1023;
	color <<= 2;
	const unsigned expected_left[] = { color, 0, 0, 1 };
	color %= 32;
	const unsigned expected_right[] = { color, 0, 0, 1 };
	pass = piglit_probe_rect_rgba_uint(0, 0, width / 2, height,
					   expected_left) && pass;
	pass = piglit_probe_rect_rgba_uint(width / 2, 0, width / 2, height,
					   expected_right) && pass;

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static void
initialize_texture()
{
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

static void
initialize_fbo()
{
	unsigned fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0);
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) ==
	       GL_FRAMEBUFFER_COMPLETE);
}

static void
initialize_program()
{
	unsigned prog;
	prog = piglit_build_simple_program(vs_text, fs_text);
	glUseProgram(prog);

	render_pass_loc = glGetUniformLocation(prog, "render_pass");
	if (render_pass_loc == -1) {
		fprintf(stderr, "Error getting uniform render_pass.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	int fb_loc = glGetUniformLocation(prog, "fb");
	glUniform1i(fb_loc, 0);
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_texture_barrier");
	piglit_require_GLSL_version(130);

	width = 256;
	height = 128;

	initialize_program();
	initialize_texture();
	initialize_fbo();

	glViewport(0, 0, width, height);
}
