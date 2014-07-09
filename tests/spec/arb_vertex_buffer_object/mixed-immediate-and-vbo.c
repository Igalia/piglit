/* © 2011 Intel Corporation
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

/** @file mixed-immediate-and-vbo.c
 *
 * Tests for a bug in the i965 driver.  When the index limits were
 * unknown (because they were in a VBO which Mesa tries to avoid
 * reading) and some VBOs plus immediate vertex data was used, the
 * immediate vertex data would be trashed.
 *
 * https://bugs.freedesktop.org/show_bug.cgi?id=37934
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint vbo;

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_vertex_buffer_object");
        piglit_require_GLSL();
}

const char *vs_source = {
	"void main() {"
	"	gl_Position = gl_Vertex;"
	"	gl_FrontColor = gl_Color;"
	"}"
};

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	float green[] = {0.0, 1.0, 0.0, 0.0};
	float vertex_data[] = {
		/* quad position */
		-1.0, -1.0, 0.0, 1.0,
		 1.0, -1.0, 0.0, 1.0,
		 1.0,  1.0, 0.0, 1.0,
		-1.0,  1.0, 0.0, 1.0,
	};
	uint32_t index_data[] = { 0, 1, 2, 3 };
	uintptr_t index_offset = sizeof(vertex_data);
	GLuint prog;

	glClearColor(1.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Use a vertex shader.  Otherwise mesa turns our immediate
	 * color data into a uniform in the fixed function vertex
	 * shader.
	 */
	prog = piglit_build_simple_program(vs_source, NULL);
	glUseProgram(prog);

	glGenBuffersARB(1, &vbo);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vbo);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,
			index_offset + sizeof(index_data),
			NULL, GL_DYNAMIC_DRAW);
	glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,
			   0, sizeof(vertex_data),
			   vertex_data);
	glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,
			   index_offset, sizeof(index_data),
			   index_data);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(4, GL_FLOAT, 0, (void *)0);
	glColor4f(0.0, 1.0, 0.0, 0.0);
	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT,
		       (void *)index_offset);

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green);

	piglit_present_results();

	glDisableClientState(GL_VERTEX_ARRAY);
	glDeleteBuffersARB(1, &vbo);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
