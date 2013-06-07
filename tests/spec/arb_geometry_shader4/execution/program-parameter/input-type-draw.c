/*
 * Copyright © 2013 The Piglit project
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
 * \file input-type-draw.c
 *
 * Test required errors for wrong GL_GEOMETRY_INPUT_TYPE and drawing mode
 * parameter combinations.
 *
 * From the ARB_geometry_shader4 spec (section Errors):
 * "The error INVALID_OPERATION is generated if Begin, or any command that
 * implicitly calls Begin, is called when a geometry shader is active and:
 *
 *     * the input primitive type of the current geometry shader is
 *       POINTS and <mode> is not POINTS,
 *
 *     * the input primitive type of the current geometry shader is
 *       LINES and <mode> is not LINES, LINE_STRIP, or LINE_LOOP,
 *
 *     * the input primitive type of the current geometry shader is
 *       TRIANGLES and <mode> is not TRIANGLES, TRIANGLE_STRIP or
 *       TRIANGLE_FAN,
 *
 *     * the input primitive type of the current geometry shader is
 *       LINES_ADJACENCY_ARB and <mode> is not LINES_ADJACENCY_ARB or
 *       LINE_STRIP_ADJACENCY_ARB, or
 *
 *     * the input primitive type of the current geometry shader is
 *       TRIANGLES_ADJACENCY_ARB and <mode> is not
 *       TRIANGLES_ADJACENCY_ARB or TRIANGLE_STRIP_ADJACENCY_ARB."
 */

#include "common.h"


struct primitive_draw_info {
	GLenum type;
	GLenum base_type;
};


/* Primitive types passed to glBegin (or equivalent) and matching geometry
 * shader input type;
 */
static const struct primitive_draw_info primitives_draw[] = {
	{GL_POINTS, GL_POINTS},

	{GL_LINES, GL_LINES},
	{GL_LINE_STRIP, GL_LINES},
	{GL_LINE_LOOP, GL_LINES},

	{GL_TRIANGLES, GL_TRIANGLES},
	{GL_TRIANGLE_STRIP, GL_TRIANGLES},
	{GL_TRIANGLE_FAN, GL_TRIANGLES},

	{GL_LINES_ADJACENCY, GL_LINES_ADJACENCY},
	{GL_LINE_STRIP_ADJACENCY, GL_LINES_ADJACENCY},

	{GL_TRIANGLES_ADJACENCY, GL_TRIANGLES_ADJACENCY},
	{GL_TRIANGLE_STRIP_ADJACENCY, GL_TRIANGLES_ADJACENCY},

	{GL_QUADS, 0xFFFF},
	{GL_QUAD_STRIP, 0xFFFF},
	{GL_POLYGON, 0xFFFF},
};


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint prog;
	int i, j;

	piglit_require_extension("GL_ARB_geometry_shader4");
	/* NV_geometry_shader4 relaxes some restrictions on valid program
	 * parameters.
	 */
	piglit_require_not_extension("GL_NV_geometry_shader4");

	/* Create shader. */
	prog = create_shader(vs_text, gs_text, fs_text);
	glProgramParameteri(prog, GL_GEOMETRY_VERTICES_OUT_ARB, 3);

	for (i = 0; i < ARRAY_SIZE(primitives_in); i++) {
		const struct primitive_geom_info geom = primitives_in[i];

		if (geom.error != GL_NO_ERROR)
			continue;

		glProgramParameteri(prog, GL_GEOMETRY_INPUT_TYPE_ARB,
				    geom.type);

		glLinkProgram(prog);
		if (!piglit_link_check_status(prog) ||
		    !piglit_check_gl_error(GL_NO_ERROR)) {
			piglit_report_result(PIGLIT_FAIL);
		}
		glUseProgram(prog);
		glUniform1i(glGetUniformLocation(prog, "vertex_count"), 1);

		for (j = 0; j < ARRAY_SIZE(primitives_draw); j++) {
			const struct primitive_draw_info draw =
				primitives_draw[j];
			GLenum e;

			printf("Testing drawing type %s, geometry input "
			       "type %s.\n",
			       piglit_get_prim_name(draw.type),
			       piglit_get_prim_name(geom.type));

			if (draw.base_type == geom.type)
				e = GL_NO_ERROR;
			else
				e = GL_INVALID_OPERATION;

			glDrawArrays(draw.type, 0, 0);
			pass = piglit_check_gl_error(e) && pass;
		}
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
