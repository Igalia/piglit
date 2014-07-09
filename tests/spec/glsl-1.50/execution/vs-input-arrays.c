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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 *  \file vs-input-arrays.c
 *
 * Test that vertex shader inputs can be arrays
 *
 * Section 4.3.4 (Inputs) of the GLSL 1.50 spec says:
 *  "Vertex shader inputs can only be float, floating-point vectors, matrices,
 *   signed and unsigned integers and integer vectors.Vertex shader inputs can
 *   also form arrays of these types, but not structures."
 *
 * This test verifies basic functionality of vertex shader inputs using
 * arrays of float, int, and vec3 respectively.
 *
 * The test functions as follows:
 * Pass four different verts to VS, each with different values. Values increment
 * by one. VS uses gl_VertexID to test that each is (expected value +
 * gl_VertexID). VS emits a float, 0 for pass, 1 for fail. This is done because
 * bool cannot be sent as a varying, and using flat shading for sending an int
 * results in additional vertex info being discarded. FS draws GREEN if it
 * received the expeced 0 from the VS, RED if !0.
*/

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END

static const char vs_text[] =
	"#version 150\n"
	"\n"
	"in vec4 vertex;\n"
	"\n"
	"in float a[2];\n"
	"in int   b[2];\n"
	"in vec3  c[2];\n"
	"\n"
	"out float i_failed;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vertex;\n"
	"\n"
	"	//check expected values against incoming\n"
	"\n"
	"	bool failed = false;\n"
	"\n"
	"	if( a[0] != 10.0 + float(gl_VertexID) ) failed = true;\n"
	"	if( a[1] != 20.0 + float(gl_VertexID) ) failed = true;\n"
	"\n"
	"	if( b[0] != 30 + gl_VertexID ) failed = true;\n"
	"	if( b[1] != 40 + gl_VertexID ) failed = true;\n"
	"\n"
	"	if( c[0].x != 1.0 + float(gl_VertexID) ) failed = true;\n"
	"	if( c[0].y != 2.0 + float(gl_VertexID) ) failed = true;\n"
	"	if( c[0].z != 3.0 + float(gl_VertexID) ) failed = true;\n"
	"	if( c[1].x != 4.0 + float(gl_VertexID) ) failed = true;\n"
	"	if( c[1].y != 5.0 + float(gl_VertexID) ) failed = true;\n"
	"	if( c[1].z != 6.0 + float(gl_VertexID) ) failed = true;\n"
	"	\n"
	"	if (failed)\n"
	"		i_failed = 1;\n"
	"	else\n"
	"		i_failed = 0;\n"
	"\n"
	"}\n";

static const char fs_text[] =
	"#version 150\n"
	"\n"
	"in float i_failed;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\n"
	"	bool failed = bool(i_failed);\n"
	"\n"
	"	if (failed)\n"
	"		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
	"	else\n"
	"		gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"}\n";

static GLuint prog;

static GLuint vao;
static GLuint vbo;

struct vertex_inputs {
        GLfloat vertex[3];
        GLfloat a[2];
        GLint b[2];
        GLfloat c[2][3];
} vertex_data[] = {
	{ { -1.0, -1.0, 0.0 },
	  { 10.0, 20.0 },
	  { 30, 40 },
	  { { 1.0, 2.0, 3.0 },
	    { 4.0, 5.0, 6.0 } }
	},

	{ { -1.0, 1.0, 0.0 },
	  { 11.0, 21.0 },
	  { 31, 41 },
	  { { 2.0, 3.0, 4.0 },
	    { 5.0, 6.0, 7.0 } }
	},

	{ { 1.0, 1.0, 0.0 },
	  { 12.0, 22.0 },
	  { 32, 42 },
	  { { 3.0, 4.0, 5.0 },
	    { 6.0, 7.0, 8.0 } }
	},

	{ { 1.0, -1.0, 0.0 },
	  { 13.0, 23.0 },
	  { 33, 43 },
	  { { 4.0, 5.0, 6.0 },
	    { 7.0, 8.0, 9.0 } }
	}
};

typedef struct vertex_inputs vertex_inputs;

void
piglit_init(int argc, char **argv)
{
	size_t stride;
	GLint vertex_index;
	GLint a_index;
	GLint b_index;
	GLint c_index;

	prog = piglit_build_simple_program(vs_text, fs_text);

	glLinkProgram( prog );

	glUseProgram( prog );

	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );

	/* stride */
	stride = sizeof( vertex_data[0] );

	/* get locations */
	vertex_index = glGetAttribLocation( prog, "vertex" );
	a_index = glGetAttribLocation( prog, "a" );
	b_index = glGetAttribLocation( prog, "b" );
	c_index = glGetAttribLocation( prog, "c" );

	/* create buffers */
	glGenBuffers( 1, &vbo );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof( vertex_data ), &vertex_data,
		      GL_STATIC_DRAW);

	/* attrib pointers:  */
	/* GLfloat vertex[3] */
	glVertexAttribPointer( vertex_index, 3, GL_FLOAT, GL_FALSE, stride,
			       (void*) offsetof( vertex_inputs, vertex[0] ));
	/* GLfloat a[2] */
	glVertexAttribPointer( a_index, 1, GL_FLOAT, GL_FALSE, stride,
			       (void*) offsetof( vertex_inputs, a[0] ));
	glVertexAttribPointer( a_index + 1, 1, GL_FLOAT, GL_FALSE, stride,
			       (void*) offsetof( vertex_inputs, a[1] ));
	/* GLint b[2] */
	glVertexAttribIPointer( b_index, 1, GL_INT, stride,
			       (void*) offsetof( vertex_inputs, b[0] ));
	glVertexAttribIPointer( b_index + 1, 1, GL_INT, stride,
			       (void*) offsetof( vertex_inputs, b[1] ));
	/* GLfloat  c[2][3] */
	glVertexAttribPointer( c_index, 3, GL_FLOAT, GL_FALSE, stride,
			       (void*) offsetof( vertex_inputs, c[0] ));
	glVertexAttribPointer( c_index + 1, 3, GL_FLOAT, GL_FALSE, stride,
			       (void*) offsetof( vertex_inputs, c[1] ));

	/* enable vertex attrib arrays */
	glEnableVertexAttribArray( vertex_index );
	glEnableVertexAttribArray( a_index );
	glEnableVertexAttribArray( a_index + 1 );
	glEnableVertexAttribArray( b_index );
	glEnableVertexAttribArray( b_index + 1 );
	glEnableVertexAttribArray( c_index );
	glEnableVertexAttribArray( c_index + 1 );

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;

	float expected_color[3] = { 0.0, 1.0, 0.0 };

	glClearColor( 0.5, 0.5, 0.5, 1.0 );

	glClear( GL_COLOR_BUFFER_BIT );

	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );

	pass = piglit_probe_pixel_rgb( 0.5, 0.5, expected_color );

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
