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

/** @file fog-coord.c
 * Verify that the GL_FOG_DISTANCE_MODE_NV setting is ignored when
 * GL_FOG_COORDINATE_SOURCE is set to GL_FOG_COORDINATE.
 *
 * The issuses section of GL_NV_fog_distance says:
 *
 *    How does this extension interact with the EXT_fog_coord extension?
 *
 *         If FOG_COORDINATE_SOURCE_EXT is set to FOG_COORDINATE_EXT,
 *         then the fog distance mode is ignored.  However, the fog
 *         distance mode is used when the FOG_COORDINATE_SOURCE_EXT is
 *         set to FRAGMENT_DEPTH_EXT.  Essentially, when the EXT_fog_coord
 *         functionality is enabled, the fog distance is supplied by the
 *         user-supplied fog-coordinate so no automatic fog distance computation
 *         is performed.
 */

#include "piglit-util-gl.h"
#include "minmax-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	static const float fog_color[4] = { 0.0, 1.0, 0.0, 1.0 };
	static const float draw_color[4] = { 1.0, 0.0, 0.0, 1.0 };
	static const float mix_color[4] = { 0.5, 0.5, 0.0, 1.0 };
	bool pass = true;

	glViewport(0, 0, piglit_width, piglit_height);

	/* Pick a Z value for the mesh.  Select fog start and stop distances
	 * such that the middle of the window (the smallest fog distance) will
	 * have zero fog and the corners (the largest fog distance) will have
	 * full fog.
	 */
	const float z = 0.5;
	const float fog_start = z;
	const float fog_end = sqrt(1.0 + 1.0 + (z * z));

	glFogf(GL_FOG_START, fog_start);
	glFogf(GL_FOG_END, fog_end);
	glFogfv(GL_FOG_COLOR, fog_color);

	glFogCoordf((fog_end + fog_start) / 2.0);
	glColor3fv(draw_color);

	piglit_draw_rect_z(z, -1.0, -1.0, 2.0, 2.0);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	pass = piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height,
				     mix_color);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_NV_fog_distance");
	piglit_require_extension("GL_EXT_fog_coord");
	piglit_require_extension("GL_ARB_vertex_buffer_object");

	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogi(GL_FOG_COORD_SRC, GL_FOG_COORDINATE);
	glFogi(GL_FOG_DISTANCE_MODE_NV, GL_EYE_RADIAL_NV);
}
