/*
 * Copyright © 2013 Intel Corporation
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

/** @file gl-layer-render.c
 * Section 4.4.7(Framebuffer Objects) From GL spec 3.2 core:
 *
 * Layer numbers for cube map texture faces. The Layers are numbered
 * in the same sequence as the cube map face token values.
 *
 * Table 4.12:
 * Layer Number		Cube Map Face
 * 0			TEXTURE_CUBE_MAP_POSITIVE_X
 * 1			TEXTURE_CUBE_MAP_NEGATIVE_X
 * 2			TEXTURE_CUBE_MAP_POSITIVE_Y
 * 3			TEXTURE_CUBE_MAP_NEGATIVE_Y
 * 4			TEXTURE_CUBE_MAP_POSITIVE_Z
 * 5			TEXTURE_CUBE_MAP_NEGATIVE_Z
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version   = 32;

PIGLIT_GL_TEST_CONFIG_END

const char *vs_source = {
	"#version 150\n"
	"in vec4 piglit_vertex;\n"
	"out vec4 vert;\n"
	"void main() {\n"
	"	gl_Position = piglit_vertex;\n"
	"	vert = piglit_vertex;\n"
	"}\n"
};

const char *gs_source = {
	"#version 150\n"
	"layout(triangles) in;\n"
	"layout(triangle_strip, max_vertices = 3) out;\n"
	"in vec4 vert[3];\n"
	"uniform int layer;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	for(int i = 0; i < 3; i++) {\n"
	"		gl_Position = vert[i];\n"
	"		gl_Layer = layer;\n"
	"		EmitVertex();\n"
	"	}\n"
	"}\n"
};

const char *fs_source = {
	"#version 150\n"
	"uniform vec3 color;\n"
	"void main() {\n"
	"	gl_FragColor = vec4(color.xyz, 1);\n"
	"}\n"
};

bool check_framebuffer_status(GLenum target, GLenum expected) {
	GLenum observed = glCheckFramebufferStatus(target);
	if(expected != observed) {
		printf("Unexpected framebuffer status!\n"
		       "  Observed: %s\n  Expected: %s\n",
		       piglit_get_gl_enum_name(observed),
		       piglit_get_gl_enum_name(expected));
		return false;
	}
	return true;
}

void
piglit_init(int argc, char **argv)
{
	int i, j;
	bool pass = true;
	GLuint fbo, texture, program;
	GLuint color_uniform, layer_uniform;

	float colors[6*3] = {
		0, 0, 1,
		0, 1, 0,
		0, 1, 1,
		1, 0, 0,
		1, 0, 1,
		1, 1, 0
	};

	program = piglit_build_simple_program_multiple_shaders(
					GL_VERTEX_SHADER, vs_source,
					GL_GEOMETRY_SHADER, gs_source,
					GL_FRAGMENT_SHADER, fs_source,
					0);
	glUseProgram(program);

	/* Retrieve index from vs */
	color_uniform = glGetUniformLocation(program, "color");
	layer_uniform = glGetUniformLocation(program, "layer");

	/* Gen cubemap texture */
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	for(i = 0; i < 6; i++) {
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			     GL_RGB, 6, 6, 0, GL_RGB, GL_FLOAT, NULL);
	}

	/* Attach cubemap as a layered texture */
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);

	/* Check for errors during setup */
	if(!check_framebuffer_status(GL_FRAMEBUFFER, GL_FRAMEBUFFER_COMPLETE) ||
	   !piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Error occured during setup.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Loop through each face of the cube map,
	 * and render a colored quad to each face.
	 */
	for(j = 0; j < 6; j++) {
		glUniform1i(layer_uniform, j);
		glUniform3f(color_uniform, colors[j*3+0],
			    colors[j*3+1], colors[j*3+2]);

		piglit_draw_rect(-1, -1, 2, 2);
	}

	/* Check for correct color on each cube map face */
	for(i = 0; i < 6; i++) {
		glFramebufferTexture2D(GL_FRAMEBUFFER,
				       GL_COLOR_ATTACHMENT0,
				       GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,
				       texture,
				       0);

		if(!check_framebuffer_status(GL_FRAMEBUFFER,
					     GL_FRAMEBUFFER_COMPLETE)) {
			printf("Error occured while probing texture\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		pass = piglit_probe_rect_rgb(0, 0, 6, 6, &colors[i*3]) && pass;
	}

	/* Clean up */
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &texture);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}
