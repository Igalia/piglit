/*
 * Copyright (c) 2014 VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
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
 * @file
 * Tests GL_ARB_base_instance.  This test also requires GL_ARB_draw_instanced
 * and GL_ARB_instanced_arrays
 */

#include "piglit-util-gl.h"
#include "piglit-matrix.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_core_version = 31;
	config.window_width = 400;
	config.window_height = 400;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "arb_base_instance-drawarrays";

static GLint MVPUniform, PosUniform;
static GLint VertexAttrib, ColorAttrib;

static float modelview[16], projection[16], modelviewproj[16];

#define PRIMS 8

/* Ortho projection width, height */
#define W 10.0
#define H 10.0


/**
 * Vertex position comes from Pos[instance].  Color comes from an
 * instanced array.
 */
static const char *VertShaderText =
	"#version 130 \n"
	"#extension GL_ARB_draw_instanced: enable \n"
	"in vec4 Vertex, Color; \n"
	"uniform vec2 Pos[8]; \n"
	"uniform mat4 MVP; \n"
	"out vec4 ColorVarying; \n"
	"void main() \n"
	"{ \n"
	"	vec4 p = Vertex; \n"
	"	vec2 pos = Pos[gl_InstanceIDARB]; \n"
	"	p.xy += pos; \n"
	"	gl_Position = MVP * p; \n"
	"	ColorVarying = Color; \n"
	"} \n";

/* Simple color pass-through */
static const char *FragShaderText =
	"#version 130 \n"
	"in vec4 ColorVarying; \n"
	"out vec4 FragColor; \n"
	"void main() \n"
	"{ \n"
	"	FragColor = ColorVarying; \n"
	"} \n";


static GLuint VertShader, FragShader, Program;

/* Instance positions in uniform array */
static const GLfloat Positions[PRIMS][2] = {
	{ -6, 6 },
	{ -4, 4 },
	{ -2, 2 },
	{  0, 0 },
	{  2, -2 },
	{  4, -4 },
	{  6, -6 },
	{  8, -8 }
};

/* Instance colors in vertex array */
static const GLfloat Colors[PRIMS][4] = {
	{1, 0, 0, 1},
	{0, 1, 0, 1},
	{0, 0, 1, 1},
	{1, 1, 0, 1},
	{0, 1, 1, 1},
	{1, 0, 1, 1},
	{1, 1, 1, 1},
	{0.5, 0.5, 0.5, 1},
};


/** Convert object position to window position */
static void
objpos_to_winpos(const float obj[2], int win[2])
{
	float winpos[4];
	float objpos[4] = { obj[0], obj[1], 0.0, 1.0 };

	piglit_project_to_window(winpos, objpos, modelview, projection,
				 0, 0, piglit_width, piglit_height);
	win[0] = (int) winpos[0];
	win[1] = (int) winpos[1];
}


static bool
test_instancing(GLuint divisor, GLuint baseInstance)
{
	GLuint verts_bo, colors_bo;

	static const GLfloat verts[4][2] = {
		{-1, -1}, {1, -1}, {1, 1}, {-1, 1}
	};
	const GLuint numPrims = PRIMS - baseInstance;
        GLint i, pos[2];

	glGenBuffers(1, &verts_bo);
	glBindBuffer(GL_ARRAY_BUFFER, verts_bo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	glVertexAttribPointer(VertexAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(VertexAttrib);

	glGenBuffers(1, &colors_bo);
	glBindBuffer(GL_ARRAY_BUFFER, colors_bo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Colors), Colors, GL_STATIC_DRAW);

	glVertexAttribPointer(ColorAttrib, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(ColorAttrib);
	/* advance color once every 'n' instances */
	glVertexAttribDivisorARB(ColorAttrib, divisor);

	glViewport(0, 0, piglit_width, piglit_height);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(Program);

	glDrawArraysInstancedBaseInstance(GL_TRIANGLE_FAN, 0, 4,
					  numPrims, baseInstance);

	/* check rendering */
	for (i = 0; i < numPrims; i++) {
		GLuint elem = i / divisor + baseInstance;

		/* compute probe location */
		objpos_to_winpos(Positions[i], pos);

		if (!piglit_probe_pixel_rgba(pos[0], pos[1], Colors[elem])) {
			fprintf(stderr, "%s: instance %d failed to draw correctly\n",
				TestName, i);
			fprintf(stderr, "%s: color instance divisor = %u  base = %u\n",
				TestName, divisor, baseInstance);
			piglit_present_results();
			return false;
		}
	}

	glDisableVertexAttribArray(VertexAttrib);
	glDisableVertexAttribArray(ColorAttrib);

	glDeleteBuffers(1, &verts_bo);
	glDeleteBuffers(1, &colors_bo);

	piglit_present_results();

	return true;
}


enum piglit_result
piglit_display(void)
{
	GLuint div, baseInst;

	for (div = 1; div <= PRIMS; div++) {
		for (baseInst = 0; baseInst < PRIMS -1; baseInst++) {
			if (!test_instancing(div, baseInst))
				return PIGLIT_FAIL;
		}
	}

	return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
	GLuint vao;

	piglit_require_extension("GL_ARB_draw_instanced");
	piglit_require_extension("GL_ARB_instanced_arrays");
	piglit_require_extension("GL_ARB_base_instance");

	VertShader = piglit_compile_shader_text(GL_VERTEX_SHADER,
						VertShaderText);
	assert(VertShader);

	FragShader = piglit_compile_shader_text(GL_FRAGMENT_SHADER,
						FragShaderText);
	assert(FragShader);

	Program = piglit_link_simple_program(VertShader, FragShader);

	glUseProgram(Program);

	MVPUniform = glGetUniformLocation(Program, "MVP");
	PosUniform = glGetUniformLocation(Program, "Pos");
	VertexAttrib = glGetAttribLocation(Program, "Vertex");
	ColorAttrib = glGetAttribLocation(Program, "Color");

	glUniform2fv(PosUniform, PRIMS, (GLfloat *) Positions);

	/* Setup coordinate transformation */
	piglit_scale_matrix(modelview, 0.5, 0.5, 1.0);
	piglit_ortho_matrix(projection, -0.5 * W, 0.5 * W,
			    -0.5 * H, 0.5 * H, -1.0, 1.0);
	piglit_matrix_mul_matrix(modelviewproj, modelview, projection);

	glUniformMatrix4fv(MVPUniform, 1, GL_FALSE, modelviewproj);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
}
