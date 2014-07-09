/*
 * Copyright © 2011 Marek Olšák
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

/** @file blending-in-shader.c
 *
 * Test programmable blending with GL_NV_texture_barrier.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static GLuint tex, fbo, prog, texloc;
static float tex_data[16*16*4], res_data[16*16*4];

static const char *fstext = {
	"uniform sampler2D fb;"
	"void main() {"
	"  gl_FragColor = sqrt(texture2D(fb, gl_FragCoord.xy / 16.0));"
	"}"
};

#define PASSES 3

enum piglit_result piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int i;

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 16, 16, 0, GL_RGBA, GL_FLOAT, tex_data);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glViewport(0, 0, 16, 16);

	glUseProgram(prog);
	glUniform1i(texloc, 0);

	for (i = 0; i < PASSES; i++) {
		if (i != 0)
			glTextureBarrierNV();
		piglit_draw_rect_tex(-1, -1, 2, 2, 0, 0, 1, 1);
	}

	pass = piglit_probe_image_rgba(0, 0, 16, 16, res_data);

	glUseProgram(0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glViewport(0, 0, piglit_width, piglit_height);

	piglit_draw_rect_tex(-1, -1, 2, 2, 0, 0, 1, 1);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	unsigned int i, j;

	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_NV_texture_barrier");
        piglit_require_GLSL();

	srand(0);
	for (i = 0; i < 16 * 16 * 4; ++i) {
		tex_data[i] = (rand() % 256) / 255.f;
		res_data[i] = tex_data[i];
		for (j = 0; j < PASSES; j++)
			res_data[i] = sqrt(res_data[i]);
	}

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 16, 16, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	assert(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT);

	prog = piglit_build_simple_program(NULL, fstext);

	texloc = glGetUniformLocation(prog, "fb");
}
