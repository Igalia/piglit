/*
 * Copyright © 2008 Intel Corporation
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
 * \file
 * Test passing fog coordinates into a fragment program.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util-gl.h"

static GLint prog = 0;

static const char* const program_text =
	"!!ARBfp1.0\n"
	"MOV result.color, fragment.fogcoord;\n"
	"END\n"
	;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static PFNGLFOGCOORDFPROC pglFogCoordf = NULL;

enum piglit_result
piglit_display(void)
{
	static const struct {
		float x, y, r;
	}
	probes[4] = {
		{ 0.5, 1.5, 0.3 },
		{ 1.5, 1.5, 0.6 },
		{ 0.5, 0.5, 0.8 },
		{ 1.5, 0.5, 0.4 },
	};
	int pass = 1;
	unsigned i;

	piglit_ortho_projection(2.0, 2.0, GL_FALSE);

	glClear(GL_COLOR_BUFFER_BIT);

	pglFogCoordf(0.3);
	glBegin(GL_QUADS);
	glVertex2f(0, 1);
	glVertex2f(1, 1);
	glVertex2f(1, 2);
	glVertex2f(0, 2);
	glEnd();

	pglFogCoordf(0.6);
	glBegin(GL_QUADS);
	glVertex2f(1, 1);
	glVertex2f(2, 1);
	glVertex2f(2, 2);
	glVertex2f(1, 2);
	glEnd();

	pglFogCoordf(0.8);
	glBegin(GL_QUADS);
	glVertex2f(0, 0);
	glVertex2f(1, 0);
	glVertex2f(1, 1);
	glVertex2f(0, 1);
	glEnd();

	pglFogCoordf(0.4);
	glBegin(GL_QUADS);
	glVertex2f(1, 0);
	glVertex2f(2, 0);
	glVertex2f(2, 1);
	glVertex2f(1, 1);
	glEnd();

	for (i = 0; i < 4; i++) {
		float expected_color[4];

		expected_color[0] = probes[i].r;
		expected_color[1] = 0.0;
		expected_color[2] = 0.0;
		expected_color[3] = 1.0;

		pass &= piglit_probe_pixel_rgba(probes[i].x * piglit_width / 2,
						probes[i].y * piglit_height / 2,
						expected_color);
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));

	glClearColor(0.3, 0.3, 0.3, 0.3);

	if (piglit_get_gl_version() >= 14) {
		pglFogCoordf = glFogCoordf;
	} else if (piglit_is_extension_supported("GL_EXT_fog_coord")) {
		pglFogCoordf = glFogCoordfEXT;
	} else {
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_require_fragment_program();
	prog = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, program_text);

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, prog);

	glFogi(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);
}
