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
 */

/**
 * @file linear-fog.c:  Draw using linear fog in GLSL.
 *
 * Simple fog test with constant fog coordinates.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char *vs_text =
	"void main() {\n"
	"   gl_Position = gl_Vertex; \n"
	"   gl_FogFragCoord = gl_MultiTexCoord0.x; \n"
	"   gl_FrontColor = gl_Color; \n"
	"}\n";
static const char *fs_text =
	"void main() {\n"
	"   float bf = (gl_FogFragCoord - gl_Fog.start) * gl_Fog.scale; \n"
	"   gl_FragColor = mix(gl_Color, gl_Fog.color, bf); \n"
	"}\n";

static const float vertex_color[] = {.25, .5, .75, .25};
static const float fog_color[] = {1, .5, 1, 0};
static const float fog_start = 100;
static const float fog_end = 200;

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	for (int j = 0; j < 5; ++j) {
		const float fog_coord = 100 + j * 25;
		const float bf =
			(fog_coord - fog_start) / (fog_end - fog_start);
		float expected_color[4];
		for (int i = 0; i < 4; ++i)
			expected_color[i] = bf * fog_color[i] +
					    (1 - bf) * vertex_color[i];

		glClear(GL_COLOR_BUFFER_BIT);

		glTexCoord1f(fog_coord);
		piglit_draw_rect(-1, -1, 2, 2);

		pass = piglit_probe_pixel_rgba(piglit_width / 2,
					       piglit_height / 2,
					       expected_color) &&
		       pass;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	const int program = piglit_build_simple_program(vs_text, fs_text);

	glUseProgram(program);

	glColor4fv(vertex_color);
	glFogf(GL_FOG_START, fog_start);
	glFogf(GL_FOG_END, fog_end);
	glFogfv(GL_FOG_COLOR, fog_color);
}
