/* Copyright 2012 Google Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Authors:
 *   Stuart Abercrombie <sabercrombie@google.com>
*/

/** @file triangle-guardband-vewport.c
 *
 * Tests whether clipping of triangles to the clip volume
 * is reflected in what is rasterized. Specifically,
 * triangles (unlike some other primitives) should not be
 * rasterized outside the viewport extents because they should
 * have been clipped to the clip volume mapping to the viewport.
 *
 * Faulty guard-band clipping optimizations have been known to
 * not honor this requirement.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	static float green[] = {0.0, 1.0, 0.0, 0.0};
	static float blue[]  = {0.0, 0.0, 1.0, 0.0};

	/* make the whole window green. */
	glClearColor(0.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* set viewport to the left half of the window */
	glViewport(0, 0, piglit_width / 2, piglit_height);

	/* draw blue rect extending beyond the right edge of the
	 * frustrum, notionally across the whole window
	 */
	glColor4fv(blue);
	piglit_draw_rect(-1.0, -1.0, 4.0, 2.0);

	/* check that the right half of the window, outside
	 * the viewport, still has the clear color
	 */
	pass = piglit_probe_rect_rgb(piglit_width/2, 0,
				     piglit_width/2, piglit_height,
				     green);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
}
