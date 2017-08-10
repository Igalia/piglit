/*
 * Copyright © 2016 Intel Corporation
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

/** @file vs-non-uniform-control-flow-ubo.c
 *
 * This test checks that uniform block reads works correctly when they are
 * under non-uniform control flow.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.window_width = 62;
	config.window_height = 62;
	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

#define UBO_SIZE 12

static const char vs_source[] =
	"#version 330\n"
	"#extension GL_ARB_gpu_shader_fp64 : require\n"
	"#extension GL_ARB_shading_language_420pack : require\n"
	"\n"
	"out vec4 color;\n"
	"\n"
	"layout(binding=2) uniform ubo {\n"
	"        dvec2 color2[];\n"
	"};\n"
	"\n"
	"layout(location = 0) in vec3 inVertexPosition;\n"
	"\n"
	"void main() {\n"
	"        gl_Position = vec4(inVertexPosition, 1);\n"
	"        dvec2 rg;\n"
	"        if (inVertexPosition.x < 0 && inVertexPosition.y < 0)\n"
	"                rg = color2[0];\n"
	"        else\n"
	"                rg = color2[1];\n"
	"        color = vec4(rg, 0, 1);\n"
	"}\n";

static const char fs_source[] =
	"#version 130\n"
	"\n"
	"in vec4 color;\n"
	"out vec4 frag_color;\n"
	"\n"
	"void main() {\n"
	"        frag_color = color;\n"
	"}\n";

static GLuint prog, vertexArrayID;
static GLuint fb, rb;

void
piglit_init(int argc, char **argv)
{
	GLuint vertexBuffer, buffer;
	// Vertex data
	static const GLfloat vertexData[4 * 3] = {
		-1.0f,  -1.0f,  -1.0f,
		 1.0f,  -1.0f,  -1.0f,
		-1.0f,   1.0f,  -1.0f,
		 1.0f,   1.0f,  -1.0f,
	};
	static double ubo_values[UBO_SIZE] = {0, 1, 1, 0,
					      0, 0, 0, 0,
					      0, 0, 0, 0};

	piglit_require_extension("GL_ARB_gpu_shader_fp64");

	piglit_require_GLSL_version(330);

	prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);

	glClearColor(0, 0, 0, 1);
	glPointSize(10.0);

	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA,
			      piglit_width, piglit_height);

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, rb);

	// Record vertex data and attributes in a VAO
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);
	// Upload vertex position data to a VBO
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData),
		     vertexData, GL_STATIC_DRAW);

	// Bind vertex position VBO to vertex shader attribute index 0
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,                  // attribute index
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // buffer offset
		);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Unbind VAO
	glBindVertexArray(0);
	// Disable attribute arrays
	glDisableVertexAttribArray(0);

	glGenBuffers(1, &buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, buffer);
	glBufferData(GL_UNIFORM_BUFFER, UBO_SIZE*sizeof(GLdouble),
				&ubo_values[0], GL_DYNAMIC_DRAW);

	if (!piglit_check_gl_error(GL_NO_ERROR))
	   piglit_report_result(PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	bool pass = true;
	float red[4] = {1.0, 0.0, 0.0, 1.0};
	float green[4] = {0.0, 1.0, 0.0, 1.0};

	glUseProgram(prog);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
	glViewport(0, 0, piglit_width, piglit_height);

	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(vertexArrayID);
	glDrawArrays(GL_POINTS, 0, 4);
	glBindVertexArray(0);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);

	/* Verify */
	pass = piglit_probe_pixel_rgba(0, 0, green) && pass;
	pass = piglit_probe_pixel_rgba(0, piglit_height - 1, red) && pass;
	pass = piglit_probe_pixel_rgba(piglit_width - 1,
				       piglit_height - 1, red) && pass;
	pass = piglit_probe_pixel_rgba(piglit_width - 1, 0, red) && pass;

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
