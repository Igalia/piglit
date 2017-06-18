/* Copyright © 2017 Intel Corporation
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

/** @file simple-draw.c
 * Simple rendering tests of GL_NV_fog_distance
 */

#include "piglit-util-gl.h"
#include "minmax-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static GLenum distance_mode = GL_EYE_RADIAL_NV;

enum piglit_result
piglit_display(void)
{
	static const float fog_color[4] = { 0.0, 1.0, 0.0, 1.0 };
	static const float draw_color[4] = { 1.0, 0.0, 0.0, 1.0 };
	static const float mix_color[4] = { 0.5, 0.5, 0.0, 1.0 };
	bool pass = true;

	glViewport(0, 0, piglit_width, piglit_height);

	/* Pick a Z value for the mesh.  Select fog start and stop distances
	 * such that the middle of the window (the closest point) will have
	 * zero fog and the corners (the farthest points) will have full fog.
	 */
	float z = 0.5;
	float fog_start;
	float fog_end;

	switch (distance_mode) {
	case GL_EYE_RADIAL_NV:
		fog_start = z;
		fog_end = sqrt(1.0 + 1.0 + (z * z));
		break;
	case GL_EYE_PLANE:
		fog_start = 0.0;
		fog_end = 1.0;
		break;
	case GL_EYE_PLANE_ABSOLUTE_NV:
		/* For eye-plane absolute, set Z such that the eye-plane
		 * distance is negative.
		 */
		z = -0.5;
		fog_start = 0.0;
		fog_end = 1.0;
		break;
	default:
		assert(!"Impossible distance mode.");
	}

	glFogi(GL_FOG_DISTANCE_MODE_NV, distance_mode);
	glFogf(GL_FOG_START, fog_start);
	glFogf(GL_FOG_END, fog_end);
	glFogfv(GL_FOG_COLOR, fog_color);

	const unsigned n = MAX2(2, (piglit_width + 1) / 2);
	const unsigned m = MAX2(2, (piglit_height + 1) / 2);
	const float height = 2.0 / (m - 1);
	const float width = 2.0 / (n - 1);

	glColor3fv(draw_color);
	for (unsigned i = 0; i < m; i++) {
		const float y = i * height - 1.0;

		for (unsigned j = 0; j < n; j++) {
			const float x = j * width - 1.0;

			piglit_draw_rect_z(z, x, y, width, height);
		}
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	switch (distance_mode) {
	case GL_EYE_RADIAL_NV:
		pass = piglit_probe_pixel_rgb(0, 0, fog_color) && pass;
		pass = piglit_probe_pixel_rgb(piglit_width / 2,
					      piglit_height / 2,
					      draw_color) && pass;
		break;
	case GL_EYE_PLANE:
	case GL_EYE_PLANE_ABSOLUTE_NV:
		pass = piglit_probe_pixel_rgb(0, 0, mix_color) && pass;
		pass = piglit_probe_pixel_rgb(piglit_width / 2,
					      piglit_height / 2,
					      mix_color) && pass;
		break;
	default:
		assert(!"Impossible distance mode.");
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	static const struct {
		const char *name;
		GLenum mode;
	} modes[] = {
		{ "radial", GL_EYE_RADIAL_NV },
		{ "eye-plane", GL_EYE_PLANE },
		{ "eye-plane-absolute", GL_EYE_PLANE_ABSOLUTE_NV }
	};

	piglit_require_extension("GL_NV_fog_distance");
	piglit_require_extension("GL_EXT_fog_coord");
	piglit_require_extension("GL_ARB_vertex_buffer_object");

	if (argc > 1) {
		unsigned i;

		for (i = 0; i < ARRAY_SIZE(modes); i++) {
			if (strcmp(modes[i].name, argv[1]) == 0) {
				distance_mode = modes[i].mode;
				break;
			}
		}

		if (i == ARRAY_SIZE(modes)) {
			printf("Unknown distance mode \"%s\".\n",
			       argv[1]);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

     	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogi(GL_FOG_COORD_SRC, GL_FRAGMENT_DEPTH);
}
