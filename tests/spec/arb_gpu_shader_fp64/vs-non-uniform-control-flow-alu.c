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

/** @file fs-non-uniform-control-flow-alu.c
 *
 * This test checks the double ALU ops work correctly when they are
 * under non-uniform control flow.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.window_width = 62;
	config.window_height = 62;
	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_source[] =
	"#version 330\n"
	"#extension GL_ARB_gpu_shader_fp64 : require\n"
	"\n"
	"out vec4 color;\n"
	"\n"
	"uniform dvec2 u0;\n"
	"uniform dvec2 u1;\n"
	"\n"
	"layout(location = 0) in vec3 inVertexPosition;\n"
	"\n"
	"void main() {\n"
	"        gl_Position = vec4(inVertexPosition, 1);\n"
	"        dvec2 rg;\n"
	"        if (inVertexPosition.x < 0 && inVertexPosition.y < 0) {\n"
	"                double tmp0 = mod(u1.y, 4.0lf);\n"
	"                tmp0 += 3.0lf / 4.0lf + 0.25lf;\n"
	"                rg = dvec2(u1.x, tmp0 - 1.0);\n"
	"        } else {\n"
	"                dvec2 tmp0 = 4.0lf * (floor(u0) - dvec2(0.75, 0.0lf));\n"
	"                tmp0.y = max(tmp0.y - 2, 0);\n"
	"                rg = tmp0;\n"
	"        }\n"
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

	GLuint vertexBuffer;
	GLuint vs, fs;
	// Vertex data
	static const GLfloat vertexData[4 * 3] = {
		-1.0f,  -1.0f,  -1.0f,
		 1.0f,  -1.0f,  -1.0f,
		-1.0f,   1.0f,  -1.0f,
		 1.0f,   1.0f,  -1.0f,
	};
	GLuint u0, u1;
	double d0[2] = {1.4, 0.2};
	double d1[2] = {0.0, 5.0};

	piglit_require_extension("GL_ARB_uniform_buffer_object");
	piglit_require_extension("GL_ARB_gpu_shader_fp64");

	piglit_require_GLSL_version(130);
	piglit_require_vertex_shader();
	piglit_require_fragment_shader();

	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_source);
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);
	prog = piglit_link_simple_program(vs, fs);

	glUseProgram(prog);

	glClearColor(0, 0, 0, 1);
	glPointSize(10.0);

	u0 = glGetUniformLocation(prog, "u0");
	u1 = glGetUniformLocation(prog, "u1");

	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA,
			      piglit_width, piglit_height);

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, rb);

	/* Uniform*/
	u0 = glGetUniformLocation(prog, "u0");
	u1 = glGetUniformLocation(prog, "u1");

	glUniform2dv(u0, 1, d0);
	glUniform2dv(u1, 1, d1);

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
