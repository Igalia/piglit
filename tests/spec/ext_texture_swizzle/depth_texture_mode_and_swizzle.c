/*
 * Copyright © 2012 Intel Corporation
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
 * \file depth_texture_mode_and_swizzle.c
 *
 * Tests the interactions between EXT_texture_swizzle and DEPTH_TEXTURE_MODE.
 *
 * From the EXT_texture_swizzle specfication:
 * "4) How does this interact with depth component textures?
 *
 *  RESOLVED: The swizzle is applied after the DEPTH_TEXTURE_MODE. This
 *  naturally falls out of specifying the swizzle in terms of Table 3.20."
 *
 * It would be very easy to write an implementation that respects one or the
 * other (but not both), or applies them in the wrong order.  This test guards
 * against those pitfalls.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

    config.supports_gl_compat_version = 10;

    config.window_width = 170;
    config.window_height= 30;
    config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display()
{
	bool pass = true;
	int i = 0;
	static const struct {
		GLenum depth_mode;
		int swizzles[4];
		float expected[4];
	} tests[] = {
		{
			GL_INTENSITY,
			{ GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA },
			{ .5, .5, .5, .5 }
		},
		{
			GL_INTENSITY,
			{ GL_ONE, GL_GREEN, GL_BLUE, GL_ALPHA },
			{ 1, .5, .5, .5 }
		},
		{
			GL_LUMINANCE,
			{ GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA },
			{ .5, .5, .5, 1 }
		},
		{
			GL_LUMINANCE,
			{ GL_RED, GL_ALPHA, GL_ALPHA, GL_ONE },
			{ .5, 1, 1, 1 }
		},
		{
			GL_RED,
			{ GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA },
			{ .5, 0, 0, 1 }
		},
		{
			GL_RED,
			{ GL_BLUE, GL_GREEN, GL_RED, GL_ALPHA },
			{ 0, 0, .5, 1 }
		},
		{
			GL_ALPHA,
			{ GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA },
			{ 0, 0, 0, 0.5 }
		},
		{
			GL_ALPHA,
			{ GL_ONE, GL_GREEN, GL_ALPHA, GL_ZERO },
			{ 1, 0, .5, 0 }
		},
	};

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(0.15, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	for (i = 0; i < ARRAY_SIZE(tests); i++) {
		glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE,
				tests[i].depth_mode);
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA,
				 tests[i].swizzles);

		piglit_draw_rect(10 + 20 * i, 10, 10, 10);

		pass = piglit_probe_rect_rgba(10 + 20 * i, 10, 10, 10,
					      tests[i].expected) && pass;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

/**
 * Create texel data: a 1x1 depth texture containing 0.5.
 */
void
setup_texture()
{
	GLuint tex;

	const float contents = 0.5;

	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1, 1, 0,
		     GL_DEPTH_COMPONENT, GL_FLOAT, &contents);

	/* Omit the complexity of depth comparisons; just use the raw data. */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
}

/**
 * Generate, compile, link, and use the GLSL shaders.
 */
void
setup_shaders()
{
	GLuint prog, tex_location;

	static const char *vs_code =
		 "#version 120\n"
		 "void main()\n"
		 "{\n"
		 "   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
		 "}\n";
	static const char *fs_code =
		 "#version 120\n"
		 "uniform sampler2D tex;\n"
		 "void main()\n"
		 "{\n"
		 "   gl_FragColor = texture2D(tex, vec2(0.5, 0.5));\n"
		 "}\n";

	prog = piglit_build_simple_program(vs_code, fs_code);
	glUseProgram(prog);

	tex_location = glGetUniformLocation(prog, "tex");
	glUniform1i(tex_location, 0);
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_texture_swizzle");
	piglit_require_extension("GL_ARB_texture_rg");

	setup_shaders();
	setup_texture();
}
