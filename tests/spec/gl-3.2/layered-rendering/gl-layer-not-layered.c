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


/** @file framebuffer-multi-attachments.c
 *
 * Section 4.4.7(Framebuffer Objects) From GL spec 3.2 core:
 *
 * A layer number written by a geometry shader has no effect if
 * the framebuffer is not layered.
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
	"		gl_Layer = layer;\n"
	"		gl_Position = vert[i];\n"
	"		EmitVertex();\n"
	"	}\n"
	"}\n"
};

const char *fs_source = {
	"#version 150\n"
	"void main() {\n"
	"	gl_FragColor = vec4(0, 1, 0, 1);\n"
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
	GLint program;
	GLuint fbo, texture;
	GLuint layer_uniform;
	bool pass = true;

	float expected[] = {
		0.0, 1.0, 0.0,
	};

	program = piglit_build_simple_program_multiple_shaders(
						GL_VERTEX_SHADER,   vs_source,
						GL_GEOMETRY_SHADER, gs_source,
						GL_FRAGMENT_SHADER, fs_source,
						0);
	glUseProgram(program);

	layer_uniform = glGetUniformLocation(program, "layer");

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 10,
		     10, 0, GL_RGB, GL_FLOAT, NULL);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, texture, 0);

	/* Check for errors during setup */
	if(!check_framebuffer_status(GL_FRAMEBUFFER, GL_FRAMEBUFFER_COMPLETE) ||
	   !piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Try to draw quad to layer 2 */
	glUniform1i(layer_uniform, 2);
	piglit_draw_rect(-1, -1, 2, 2);

	pass = piglit_probe_rect_rgb(0, 0, 10, 10, expected) && pass;

	/* Clean up */
	glDeleteTextures(1, &texture);
	glDeleteFramebuffers(1, &fbo);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}
