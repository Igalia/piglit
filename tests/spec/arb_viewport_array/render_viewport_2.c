/*
 * Copyright © 2015 Red Hat.
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
 *
 */

/**
 * Tests rendering into a single framebuffer surface with multiple viewports
 * via a geometry shader. 
 * This test is inspired by a CTS test.
 * For one point, the geom shader emits a triangle strip with a color 
 * pre invocation. The viewports then should get one shade of red lighter.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

const char *vsSource = {
	"#version 150\n"
	"void main() {\n"
	"}\n"
};

const char *gsSource = {
	"#version 150\n"
	"#extension GL_ARB_gpu_shader5 : enable\n"
	"#extension GL_ARB_viewport_array : enable\n"
	"layout(points, invocations = 16) in;\n"
	"layout(triangle_strip, max_vertices = 4) out;\n"
	" flat out int gs_fs_color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"       gs_fs_color = gl_InvocationID;\n"
	"	gl_ViewportIndex = gl_InvocationID;\n"
	"       gl_Position = vec4(-1, -1, 0, 1);\n"
	"       EmitVertex();\n"
	"       gs_fs_color = gl_InvocationID;\n"
	"	gl_ViewportIndex = gl_InvocationID;\n"
	"       gl_Position = vec4(-1, 1, 0, 1);\n"
	"       EmitVertex();\n"
	"       gs_fs_color = gl_InvocationID;\n"
	"	gl_ViewportIndex = gl_InvocationID;\n"
	"       gl_Position = vec4(1, -1, 0, 1);\n"
	"       EmitVertex();\n"
	"       gs_fs_color = gl_InvocationID;\n"
	"	gl_ViewportIndex = gl_InvocationID;\n"
	"       gl_Position = vec4(1, 1, 0, 1);\n"
	"       EmitVertex();\n"
	"}\n"
};

const char *fsSource = {
	"#version 150\n"
	"flat in int gs_fs_color;\n"
	"uniform vec3 color;\n"
	"void main() {\n"
	"	gl_FragColor = vec4(1.0 / float(gs_fs_color + 1), 0.0, 0.0, 1.0);\n"
	"}\n"
};

static bool
draw_multi_viewport(void)
{
	bool pass = true;
	int i, j;
	const int divX=4, divY=4;
	GLfloat w = (GLfloat) piglit_width / (GLfloat) divX;
	GLfloat h = (GLfloat) piglit_height / (GLfloat) divY;
        GLfloat data[16 * 4 /* 4x4 * (x + y + w + h) */];
	int idx;
	int p;

	idx = 0;
	for (i = 0; i < divX; i++) {
		for (j = 0; j < divY; j++) {
			
			data[idx * 4 + 0] = (GLfloat)(i * w);
			data[idx * 4 + 1] = (GLfloat)(j * h);
			data[idx * 4 + 2] = w;
			data[idx * 4 + 3] = h;
			idx++;
		}
	}
	glViewport(0, 0, piglit_width, piglit_height); /* for glClear() */
	glClear(GL_COLOR_BUFFER_BIT);
	glViewportArrayv(0, 16, data);

	glDrawArrays(GL_POINTS, 0, 1);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	for (i = 0; i < divX; i++) {
		for (j = 0; j < divY; j++) {
			GLfloat expected[4];
			
			expected[0] = 1.0 / (1 + i*4 + j);
			expected[1] = 0.0;
			expected[2] = 0.0;
			p = piglit_probe_pixel_rgb(i * w + w/2, j * h + h /2,
						   expected);
			if (!p) {
				printf("Wrong color for viewport i,j %d %d\n",
				       i, j);
				pass = false;
			}
		}
	}
	      
	piglit_present_results();

	return pass;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	pass = draw_multi_viewport();
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint program;
	GLuint vao;
	piglit_require_extension("GL_ARB_viewport_array");
	piglit_require_extension("GL_ARB_gpu_shader5");

	program = piglit_build_simple_program_multiple_shaders(
					GL_VERTEX_SHADER, vsSource,
					GL_GEOMETRY_SHADER, gsSource,
					GL_FRAGMENT_SHADER, fsSource,
					0);
	glUseProgram(program);

	glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

}
