/*
 * Copyright © 2009 Intel Corporation
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
 *
 * Authors:
 *    Pierre-Eric Pelloux-Prayer <pelloux@gmail.com>
 */

/** @file draw-elements-instanced-base-vertex.c
 * (Heavily based on draw-elements-base-vertex.c)
 * Tests ARB_draw_elements_instanced_base_vertex functionality by drawing a series of
 * pairs of quads using different base vertices, using the same vertex and
 * index buffers.
 * Each pair of quads is drawn using 2 instances, and gl_InstanceID is used as a
 * color modifier and an y offset.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 300;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define NUM_QUADS  10

static const char *VertShaderText =
	"#extension GL_ARB_draw_instanced : enable\n"
	"attribute float yOffsetPerInstance; \n"
	"void main() \n"
	"{\n"
	"	vec4 p = gl_Vertex;\n"
	"	p.y += yOffsetPerInstance * float(gl_InstanceIDARB);\n"
	"	gl_Position = gl_ModelViewProjectionMatrix * p; \n"
	"	gl_FrontColor = vec4(1.0-float(gl_InstanceIDARB), 1.0, 1.0, 1.0); \n"
	"}\n";

static const char *FragShaderText =
	"void main() \n"
	"{ \n"
	"	gl_FragColor = gl_Color; \n"
	"}\n";

static GLuint Program;

static uintptr_t ib_offset;

void
piglit_init(int argc, char **argv)
{
	GLfloat *vb;
	GLuint *ib;
	GLuint vbo;
	GLint OffsetAttrib;
	GLboolean user_va = GL_FALSE;
	int i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "user_varrays")) {
			user_va = GL_TRUE;
			puts("Testing user vertex arrays.");
		}
	}

	piglit_require_GLSL();
	if (!user_va)
		piglit_require_extension("GL_ARB_vertex_buffer_object");
	piglit_require_extension("GL_ARB_draw_instanced");
	piglit_require_extension("GL_ARB_draw_elements_base_vertex");

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	if (!user_va) {
		glGenBuffersARB(1, &vbo);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB,
				NUM_QUADS * 8 * sizeof(GLfloat) +
				2 * 4 * sizeof(GLuint),
				NULL, GL_DYNAMIC_DRAW);
		vb = glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
	} else {
		vb = malloc(NUM_QUADS * 8 * sizeof(GLfloat) +
			    2 * 4 * sizeof(GLuint));
	}

	for (i = 0; i < NUM_QUADS; i++) {
		float x1 = 10;
		float y1 = 10 + i * 20;
		float x2 = 20;
		float y2 = 20 + i * 20;

		vb[i * 8 + 0] = x1; vb[i * 8 + 1] = y1;
		vb[i * 8 + 2] = x2; vb[i * 8 + 3] = y1;
		vb[i * 8 + 4] = x2; vb[i * 8 + 5] = y2;
		vb[i * 8 + 6] = x1; vb[i * 8 + 7] = y2;
	}
	ib_offset = NUM_QUADS * 8 * sizeof(GLfloat);
	ib = (GLuint *)((char *)vb + ib_offset);
	for (i = 0; i < 8; i++)
		ib[i] = i;

	if (user_va) {
		ib_offset = (uintptr_t)ib;
	} else {
		glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vbo);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, user_va ? vb : NULL);

	Program = piglit_build_simple_program(VertShaderText, FragShaderText);

	glUseProgram(Program);

	OffsetAttrib = glGetAttribLocation(Program, "yOffsetPerInstance");
	glVertexAttrib1f(OffsetAttrib, 20);
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	float white[3] = {1.0, 1.0, 1.0};
	float blue[3]  = {0.0, 1.0, 1.0};
	float clear[3] = {0.0, 0.0, 0.0};
	int i, j;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor3fv(white);
	/* Draw columns with each successive pair of the quads. */
	for (i = 0; i < NUM_QUADS - 1; i++) {
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, piglit_width, 0, piglit_height, -1, 1);
		glTranslatef(i * 20, 0, 0);

		glDrawElementsInstancedBaseVertex(GL_QUADS, 4, GL_UNSIGNED_INT,
						(void *)(uintptr_t)ib_offset, 2, i * 4);

		glPopMatrix();
	}

	for (i = 0; i < NUM_QUADS - 1; i++) {
		for (j = 0; j < NUM_QUADS; j++) {
			float *expected;

			int x = 15 + i * 20;
			int y = 15 + j * 20;

			if (j == i)
				expected = white;
			else if (j == i + 1)
				expected = blue;
			else
				expected = clear;
			pass = piglit_probe_pixel_rgb(x, y, expected) && pass;
		}
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
