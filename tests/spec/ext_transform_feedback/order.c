/*
 * Copyright © 2011 Intel Corporation
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
 * \file order.c
 *
 * Confirm that the order of vertices output by transform feedback
 * matches the order of vertices supplied to the GL pipeline.
 *
 * On OpenGL implementations that execute multiple vertex shader
 * threads in parallel, it's possible that the threads won't complete
 * in the same order that they were invoked.  When this happens, it's
 * critical that transform feedback records the vertices in the order
 * that they were inserted into the GL pipeline, not the order of
 * shader completion.
 *
 * This test verifies that transform feedback records vertices in the
 * correct order by using a vertex shader whose execution time is
 * dramatically different for different vertices.
 *
 * The test requries two command line arguments:
 *
 * - drawcall indicates which drawing function should be called.  A
 *   value of "arrays" causes DrawArrays() to be used.  A value of
 *   "elements" causes DrawElemeents() to be used.  When
 *   DrawElements() is used, we supply an indices array that scrambles
 *   the order in which vertices are sent to the shader, and verify
 *   that the scrambling is reflected in the transform feedback
 *   output.
 *
 * - mode indiceates which drawing mode should be used.  It may be
 *   "triangles", "lines", or "points".
 */

#include "piglit-util-gl.h"

#define NUM_POINTS 10002
#define SHIFT_COUNT 64

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static GLenum draw_mode;
static GLboolean use_draw_elements;
static GLuint prog;
static GLuint xfb_buf;
static unsigned *verts;
static unsigned *indices;

/* This vertex shader computes the hailstone sequence, which is
 * defined as:
 *
 * x[0] = starting_x
 * x[n+1] = x[n]/2 if x[n] is even
 *        = 3*x[n]+1 if x[n] is odd
 *
 * The shader measures, for different values of starting_x, the
 * minimum n such that x[n] = 1.  This value is output in
 * iteration_count.  The shader outputs a copy of starting_x in
 * starting_x_copy.
 *
 * To prevent an infinite loop, if starting_x is 0, it is changed to
 * 1.
 *
 * In addition, to consume more execution time, the shader maintains a
 * 31-bit shift register whose value starts at 1, and at each
 * iteration of the algorithm, shifts it left, in circular fashion,
 * shift_count times.  shift_count can be adjusted as necessary to
 * ensure that vertex shader threads complete out of order, but the
 * entire test doesn't take too long to finish.
 *
 * All of this pointless mathematics serves one purpose: to ensure
 * that different invocations of the vertex shader take dramatically
 * different amounts of time to execute.
 */
static const char *vstext =
	"#version 130\n"
	"in uint starting_x;\n"
	"flat out uint starting_x_copy;\n"
	"flat out uint iteration_count;\n"
	"flat out uint shift_reg_final;\n"
	"uniform uint shift_count;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_Position = vec4(0.0);\n"
	"  uint x = starting_x;\n"
	"  if (x == 0u)\n"
	"    x = 1u;\n"
	"  uint count = 0u;\n"
	"  uint shift_reg = 1u;\n"
	"  starting_x_copy = starting_x;\n"
	"  while (x != 1u) {\n"
	"    ++count;\n"
	"    if (x % 2u == 0u)\n"
	"      x /= 2u;\n"
	"    else\n"
	"      x = 3u * x + 1u;\n"
	"    uint i;\n"
	"    for (i = 0u; i < shift_count; ++i)\n"
	"      shift_reg = (shift_reg * 2u) % 0x7fffffffu;\n"
	"  }\n"
	"  iteration_count = count;\n"
	"  shift_reg_final = shift_reg;\n"
	"}\n";

static const char *varyings[] = {
	"starting_x_copy", "iteration_count", "shift_reg_final"
};

static void
initialize_shader_and_xfb()
{
	GLuint vs;

	piglit_require_gl_version(30);
	piglit_require_GLSL_version(130);
	piglit_require_transform_feedback();
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glTransformFeedbackVaryings(prog, 3, varyings, GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}
	glGenBuffers(1, &xfb_buf);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfb_buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
		     3*NUM_POINTS*sizeof(unsigned), NULL, GL_STREAM_READ);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf);
	glUseProgram(prog);
	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);
}

static void
initialize_vertex_shader_inputs()
{
	GLint starting_x_index = glGetAttribLocation(prog, "starting_x");
	unsigned i;

	verts = malloc(NUM_POINTS*sizeof(unsigned));
	indices = malloc(NUM_POINTS*sizeof(unsigned));

	for (i = 0; i < NUM_POINTS; ++i)
		verts[i] = i;
	for (i = 0; i < NUM_POINTS/2; ++i) {
		indices[i] = 2*i;
		indices[i + NUM_POINTS/2] = 2*i+1;
	}

	glUniform1ui(glGetUniformLocation(prog, "shift_count"),
			  SHIFT_COUNT);
	glVertexAttribIPointer(starting_x_index, 1, GL_UNSIGNED_INT, sizeof(unsigned),
			       verts);
	glEnableVertexAttribArray(starting_x_index);
	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);
}

/**
 * Compute the value of iteration_count that we expect the vertex
 * shader to output for the given starting_x.
 */
static unsigned
compute_iteration_count(unsigned starting_x)
{
	unsigned count = 0;
	unsigned x = starting_x;
	if (x == 0)
		x = 1;
	while (x != 1) {
		++count;
		if (x % 2 == 0)
			x /= 2;
		else
			x = 3 * x + 1;
	}
	return count;
}

/**
 * Compute the value of shift_reg_final that we expect the vertex
 * shader to output for the given iteration_count.
 */
static unsigned
compute_shift_reg_final(unsigned iteration_count)
{
	/* shift_reg starts at 1 and is shifted left
	 * SHIFT_COUNT*iteration_count times.  After every 31 shifts,
	 * it resets to 1.
	 */
	return ((unsigned) 1) << ((SHIFT_COUNT * iteration_count) % 31);
}

static void
draw()
{
	glEnable(GL_RASTERIZER_DISCARD);
	glBeginTransformFeedback(draw_mode);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	if (use_draw_elements) {
		glDrawElements(draw_mode, NUM_POINTS, GL_UNSIGNED_INT,
			       indices);
	} else {
		glDrawArrays(draw_mode, 0, NUM_POINTS);
	}
	glEndTransformFeedback();
	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);
}

static void
check_results_and_exit()
{
	unsigned *readback;
	unsigned i;
	GLboolean pass = GL_TRUE;

	readback = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY);
	pass = piglit_check_gl_error(0) && pass;

	for (i = 0; i < NUM_POINTS; ++i) {
		unsigned expected_starting_x =
			use_draw_elements ? indices[i] : i;
		unsigned expected_iteration_count =
			compute_iteration_count(expected_starting_x);
		unsigned expected_shift_reg_final =
			compute_shift_reg_final(expected_iteration_count);
		if (readback[3*i] != expected_starting_x) {
			printf("Order changed at vertex %u\n", i);
			pass = GL_FALSE;
			break;
		}
		if (readback[3*i+1] != expected_iteration_count) {
			printf("Incorrect iteration_count at vertex %u\n", i);
			pass = GL_FALSE;
			break;
		}
		if (readback[3*i+2] != expected_shift_reg_final) {
			printf("Incorrect shift_reg_final at vertex %u\n", i);
			pass = GL_FALSE;
			break;
		}
	}
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

static void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <drawcall> <mode>\n"
	       "  where <drawcall is one of:\n"
	       "    arrays\n"
	       "    elements\n"
	       "  and <mode> is one of:\n"
	       "    triangles\n"
	       "    lines\n"
	       "    points\n", prog_name);
	exit(1);
}

void
piglit_init(int argc, char **argv)
{
	/* Interpret command line args */
	if (argc != 3)
		print_usage_and_exit(argv[0]);
	if (strcmp(argv[1], "arrays") == 0)
		use_draw_elements = GL_FALSE;
	else if (strcmp(argv[1], "elements") == 0)
		use_draw_elements = GL_TRUE;
	else
		print_usage_and_exit(argv[0]);
	if (strcmp(argv[2], "triangles") == 0)
		draw_mode = GL_TRIANGLES;
	else if (strcmp(argv[2], "lines") == 0)
		draw_mode = GL_LINES;
	else if (strcmp(argv[2], "points") == 0)
		draw_mode = GL_POINTS;
	else
		print_usage_and_exit(argv[0]);

	initialize_shader_and_xfb();
	initialize_vertex_shader_inputs();
	draw();
	check_results_and_exit();
}

enum piglit_result piglit_display(void)
{
	/* Test should finish before we reach here. */
	return PIGLIT_FAIL;
}
