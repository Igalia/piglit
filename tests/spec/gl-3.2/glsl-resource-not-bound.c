/*
 * Copyright © 2014 Advanced Micro Devices, Inc.
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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint
get_program(const char *target)
{
	char fs_text[1024];
	static const char *fs_templ =
		"#version 150\n"
		"#extension GL_ARB_texture_cube_map_array : enable\n"
	        "uniform sampler%s s;\n"
	        "void main()\n"
	        "{\n"
	        "   gl_FragColor = %s;\n"
	        "}\n";
	static const char *vs_text =
		"#version 150\n"
		"#extension GL_ARB_explicit_attrib_location : require\n"
		"layout(location=0) in vec4 pos;\n"
	        "void main()\n"
	        "{\n"
	        "   gl_Position = pos;\n"
	        "}\n";

	if (!strcmp(target, "1D"))
		sprintf(fs_text, fs_templ, target, "texelFetch(s, 0, 0)");
        else if (!strcmp(target, "2D"))
		sprintf(fs_text, fs_templ, target, "texelFetch(s, ivec2(0), 0)");
	else if (!strcmp(target, "3D"))
		sprintf(fs_text, fs_templ, target, "texelFetch(s, ivec3(0), 0)");
	else if (!strcmp(target, "2DRect"))
		sprintf(fs_text, fs_templ, target, "texelFetch(s, ivec2(0))");
	else if (!strcmp(target, "1DArray"))
		sprintf(fs_text, fs_templ, target, "texelFetch(s, ivec2(0), 0)");
	else if (!strcmp(target, "2DArray"))
		sprintf(fs_text, fs_templ, target, "texelFetch(s, ivec3(0), 0)");
	else if (!strcmp(target, "2DMS"))
		sprintf(fs_text, fs_templ, target, "texelFetch(s, ivec2(0), 0)");
	else if (!strcmp(target, "2DMSArray"))
		sprintf(fs_text, fs_templ, target, "texelFetch(s, ivec3(0), 0)");
	else if (!strcmp(target, "Buffer"))
		sprintf(fs_text, fs_templ, target, "texelFetch(s, 0)");
	else if (!strcmp(target, "Cube"))
		sprintf(fs_text, fs_templ, target, "texture(s, vec3(0.0))");
	else if (!strcmp(target, "CubeArray")) {
		piglit_require_extension("GL_ARB_texture_cube_map_array");
		sprintf(fs_text, fs_templ, target, "texture(s, vec4(0.0))");
	}
	else {
		printf("Unknown target = %s\n", target);
		piglit_report_result(PIGLIT_FAIL);
	}

	return piglit_build_simple_program_multiple_shaders(GL_VERTEX_SHADER, vs_text,
							    GL_FRAGMENT_SHADER, fs_text,
							    0);
}

static void
draw_rect_core(int x, int y, int w, int h)
{
	float verts[4][4];

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = 0.0;
	verts[0][3] = 1.0;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = 0.0;
	verts[1][3] = 1.0;
	verts[2][0] = x + w;
	verts[2][1] = y + h;
	verts[2][2] = 0.0;
	verts[2][3] = 1.0;
	verts[3][0] = x;
	verts[3][1] = y + h;
	verts[3][2] = 0.0;
	verts[3][3] = 1.0;

	glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(float),
		     verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

enum piglit_result
piglit_display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	/* This shouldn't crash, but the result is undefined unless
	 * the context was created with robust buffer access. */
	draw_rect_core(-1, -1, 1, 1);

	piglit_present_results();
	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	GLuint vao, bo;

	if (argc != 2) {
		puts("Wrong parameters.");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_require_extension("GL_ARB_explicit_attrib_location");

	glUseProgram(get_program(argv[1]));

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &bo);
	glBindBuffer(GL_ARRAY_BUFFER, bo);
}
