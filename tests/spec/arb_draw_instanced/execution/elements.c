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
 * @file elements.c
 *
 * Tests that glDrawElementsInstancedARB() can render multiple
 * instances and the instance IDs are propagated to the shader.
 *
 * This is a derivative of instance-array-dereference.c, which uses
 * glDrawArraysInstancedARB().
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_MAIN(
    70 /*window_width*/,
    30 /*window_height*/,
    PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE)

static const char *vs_source =
	"#version 120\n"
	"#extension GL_ARB_draw_instanced: require\n"
	"\n"
	"uniform vec4 instance_colors[] = vec4[](vec4(0.0, 1.0, 0.0, 1.0),\n"
	"					 vec4(0.0, 1.0, 1.0, 1.0),\n"
	"					 vec4(0.0, 0.0, 1.0, 1.0));\n"
	"\n"
	"varying vec4 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  color = instance_colors[gl_InstanceIDARB];\n"
	"\n"
	"  vec4 v = gl_Vertex;\n"
	"  v.x += 20.0 * float(gl_InstanceIDARB);\n"
	"\n"
	"  gl_Position = gl_ModelViewProjectionMatrix * v;\n"
	"}\n";

static const char *fs_source =
	"varying vec4 color;\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = color;\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	static const int indices[6] = {0, 1, 2, 0, 2, 3};
	static const float verts[] = {
		10, 10,
		20, 10,
		20, 20,
		10, 20,
	};
	static const float green[4] = {0.0, 1.0, 0.0, 1.0};
	static const float cyan[4]  = {0.0, 1.0, 1.0, 1.0};
	static const float blue[4]  = {0.0, 0.0, 1.0, 1.0};
	enum piglit_result result = PIGLIT_PASS;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(0.5, 0.5, 0.5, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glVertexPointer(2, GL_FLOAT, 0, verts);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawElementsInstancedARB(GL_TRIANGLES, 6, GL_UNSIGNED_INT,
				   indices, 3);
	glDisableClientState(GL_VERTEX_ARRAY);

	if (!piglit_probe_rect_rgba(10, 10, 10, 10, green))
		result = PIGLIT_FAIL;

	if (!piglit_probe_rect_rgba(30, 10, 10, 10, cyan))
		result = PIGLIT_FAIL;

	if (!piglit_probe_rect_rgba(50, 10, 10, 10, blue))
		result = PIGLIT_FAIL;

	piglit_present_results();

	return result;
}

void
piglit_init(int argc, char **argv)
{
	GLuint vs, fs, prog;

	piglit_require_extension("GL_ARB_draw_instanced");

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_source);

	prog = piglit_link_simple_program(vs, fs);

	if (!vs || !fs || !prog)
		piglit_report_result(PIGLIT_FAIL);

	glDeleteShader(vs);
	glDeleteShader(fs);

	glUseProgram(prog);
}
