/*
 * Copyright (c) 2012 VMware, Inc.
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
 * @file sampler-incomplete.c
 *
 * With GL_ARB_sampler_objects it's possible for a texture to be both complete
 * and incomplete depending on the samplers used to access it.
 * Test that we get the right results in this situation.
 *
 * Brian Paul
 * March 2012
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static void
setup(void)
{
	static const GLfloat red[2][2][4] = {
		{ {0.25, 0, 0, 0}, {0.25, 0, 0, 0} },
		{ {0.25, 0, 0, 0}, {0.25, 0, 0, 0} } };
	static const char *fragShaderText =
		"uniform sampler2D tex0, tex1;\n"
		"void main()\n"
		"{\n"
		"   vec2 coord = vec2(0.5, 0.5); \n"
		"   gl_FragColor = texture2D(tex0, coord) \n"
		"                + texture2D(tex1, coord);\n"
		"}\n";
	GLuint samplers[2];
	GLuint prog;
	GLint u;
	GLuint tex;

	/* Create fragment shader that adds the two textures */
	prog = piglit_build_simple_program(NULL, fragShaderText);

	glUseProgram(prog);
	u = glGetUniformLocation(prog, "tex0");
	glUniform1i(u, 0);
	u = glGetUniformLocation(prog, "tex1");
	glUniform1i(u, 1);

	/* create texture with one mipmap level */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0,
		     GL_RGBA, GL_FLOAT, red);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   
	/* bind the texture to units 0 and 1 */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glEnable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex);
	glEnable(GL_TEXTURE_2D);


	glGenSamplers(2, samplers);

	/* sampler[0] - nearest filtering, no mipmap.
	 * The result of sampling the texture should be (0.25, 0, 0, 0)
	 */
	glBindSampler(0, samplers[0]);
	glSamplerParameteri(samplers[0], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(samplers[0], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	/* sampler[1] - nearest filtering, with mipmapping.
	 * The result of sampling the texture should be (0, 0, 0, 1) since the
	 * texture is incomplete with respect to this sampler (no mipmap).
	 */
	glBindSampler(1, samplers[1]);
	glSamplerParameteri(samplers[1], GL_TEXTURE_MIN_FILTER,
			    GL_NEAREST_MIPMAP_NEAREST);
	glSamplerParameteri(samplers[1], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}


enum piglit_result
piglit_display(void)
{
	/* NOTE: We're not checking the alpha value here.
	 * Some drivers (like NVIDIA) seem to return (0,0,0,0) when sampling
	 * an incomplete texture, but the spec says (0,0,0,1) should be
	 * returned.  That's not really important for this test though so
	 * just ignore alpha.
	 */
	GLfloat expected[3] = {0.25, 0, 0};
	bool p;

	glClear(GL_COLOR_BUFFER_BIT);

	piglit_draw_rect_tex(-1, -1, 2, 2,  0, 0, 1, 1);

	p = piglit_probe_pixel_rgb(piglit_width / 2, piglit_height / 2,
				   expected);

	piglit_present_results();

	return p ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char**argv)
{
	piglit_require_GLSL();
	piglit_require_extension("GL_ARB_sampler_objects");
	setup();
}
