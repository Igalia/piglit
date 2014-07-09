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
 * \file end-primitive.c
 *
 * Verify functionality of the geometry shader EndPrimitive() function.
 *
 * This test checks that EndPrimitive() works properly for the max
 * vertex count specified on the command line.
 *
 * The test operates by sending three POINT primitives down the
 * pipeline.  The geometry shader converts each POINT primitive into a
 * triangle_strip containing the requested number of vertices.
 * EndPrimitive() is called after every third vertex, so the resulting
 * image consists of discrete triangles.  The triangles are arranged
 * into a spiral pattern so that the maximum geometry shader output
 * vertex count can be accommodated without making the triangles too
 * small.
 *
 * Each of the 3 geometry shader invocations calls EndPrimitive() at
 * different times (the first invocation calls it prior to vertices 0,
 * 3, 6, 9, etc., the second invocation prior to vertices 1, 4, 7, 10,
 * etc., and the third invocation prior to vertices 2, 5, 8, 11,
 * etc.).  The colors of the triangles are red for the first geometry
 * shader invocation, green for the second, and blue for the third.
 * So the resulting image should show the entire triangle strip with
 * colors sequencing in red, green, blue order.
 *
 * Colors are communicated from the geometry shader to the fragment
 * shader by adjusting the value of gl_Position.z.  This allows us to
 * avoid taking up an extra varying slot to communicate color (which
 * might reduce the number of vertices we can test, due to
 * GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS).
 *
 * The test image is drawn twice, once in the manner described above
 * (the test pattern), and once without using geometry shaders (the
 * reference pattern).  The test and reference pattern are then
 * compared.
 *
 * The image is drawn with a blend equation of GL_MAX, so that if any
 * call to EndPrimitive() fails to work, the result will be visible,
 * even if a subsequent geometry shader invocation draws over the same
 * part of the image.
 */


#include "piglit-util-gl.h"

#define PATTERN_SIZE 256

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.window_width = 2*PATTERN_SIZE;
	config.window_height = PATTERN_SIZE;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static GLuint prog_ref, prog_test;
static int num_vertices;

/**
 * Function to compute the spiral pattern.  The distance between
 * adjacent vertices returned by this function is approximately
 * constant, so the resulting triangles will be approximately equal in
 * size.
 */
static const char *spiral_text =
	"#version 150\n"
	"\n"
	"uniform int num_vertices;\n"
	"\n"
	"vec2 spiral(int vertex_id)\n"
	"{\n"
	"  float pi = acos(-1.0);\n"
	"  float radial_spacing = 1.5;\n"
	"  float spiral_spacing = 0.5;\n"
	"  float a = 4.0*pi*spiral_spacing/radial_spacing;\n"
	"  float b = radial_spacing/(2*pi);\n"
	"  float theta = sqrt(a*float(vertex_id + 1));\n"
	"  float r = b*theta;\n"
	"  if (vertex_id % 2 == 1) r += 1.0;\n"
	"  float max_r = b*sqrt(a*float(num_vertices)) + 1.0;\n"
	"  r /= max_r;\n"
	"  return r*vec2(cos(theta), sin(theta));\n"
	"}\n";

/**
 * Vertex shader for drawing the test pattern.  The incoming vertex ID
 * is passed down into the geometry shader, so that it can tell which
 * invocation it is.
 */
static const char *vs_test_text =
	"#version 150\n"
	"\n"
	"out int end_prim_offset;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  end_prim_offset = gl_VertexID;\n"
	"}\n";

/**
 * Geometry shader for drawing the test pattern.
 */
static const char *gs_test_text =
	"#version 150\n"
	"\n"
	"vec2 spiral(int vertex_id);\n"
	"uniform int num_vertices;\n"
	"in int end_prim_offset[];\n"
	"\n"
	"void main()\n"
	"{\n"
	"  int i = 0;\n"
	"  while (true) {\n"
	"    if (i % 3 == end_prim_offset[0])\n"
	"      EndPrimitive();\n"
	"    if (i == num_vertices)\n"
	"      break;\n"
	"    gl_Position = vec4(spiral(i++), end_prim_offset[0]/4.0, 1.0);\n"
	"    EmitVertex();\n"
	"  }\n"
	"}\n";

/**
 * Printf template for the geometry shader layout.  %d will be filled
 * in with the number of vertices requested on the command line.
 */
static const char *gs_layout_template =
	"#version 150\n"
	"\n"
	"layout(points) in;\n"
	"layout(triangle_strip, max_vertices = %d) out;\n";

/**
 * Fragment shader for drawing both the test and reference patterns.
 */
static const char *fs_text =
	"#version 150\n"
	"\n"
	"void main()\n"
	"{\n"
	"  int end_prim_offset = int(round((gl_FragCoord.z - 0.5) * 8.0));\n"
	"  const vec4 colors[3] = vec4[3](\n"
	"    vec4(1.0, 0.0, 0.0, 1.0),\n"
	"    vec4(0.0, 1.0, 0.0, 1.0),\n"
	"    vec4(0.0, 0.0, 1.0, 1.0));\n"
	"  gl_FragColor = colors[end_prim_offset];\n"
	"}\n";

/**
 * Vertex shader for drawing the reference pattern.  gl_VertexID takes
 * the place of the variable i in the geometry shader.
 */
static const char *vs_ref_text =
	"#version 150\n"
	"\n"
	"vec2 spiral(int vertex_id);\n"
	"uniform int end_prim_offset;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_Position = vec4(spiral(gl_VertexID), end_prim_offset/4.0,\n"
	"                     1.0);\n"
	"}\n";


static void
print_usage_and_exit(const char *prog_name)
{
	printf("Usage: %s <vertex_count>\n"
	       "  where <vertex_count> is the number of vertices to test, or\n"
	       "  0 to test the maximum possible number of vertices.\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}


void
piglit_init(int argc, char **argv)
{
	GLuint vs_spiral, gs_spiral, vs_ref_main, vs_test_main, gs_test_main,
		gs_layout, fs_main, vao, element_buf;
	GLint max_gs_out_vertices, max_gs_out_components;
	int max_testable_vertices;
	char *text, *endptr;

	/* parse args */
	if (argc != 2)
		print_usage_and_exit(argv[0]);
	endptr = NULL;
	num_vertices = strtol(argv[1], &endptr, 0);
	if (endptr != argv[1] + strlen(argv[1]))
		print_usage_and_exit(argv[0]);

	/* Figure out the maximum number of vertices we can test. */
	glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &max_gs_out_vertices);
	glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS,
		      &max_gs_out_components);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	max_testable_vertices = MIN2(max_gs_out_vertices,
				     max_gs_out_components / 4);

	/* If num_vertices == 0, test the maximum possible number of
	 * vertices.  Otherwise ensure that the requested number is
	 * supported by the implementation.
	 */
	if (num_vertices == 0)
		num_vertices = max_testable_vertices;
	else if (num_vertices > max_testable_vertices) {
		printf("Can't test more than %d vertices\n",
		       max_testable_vertices);
		piglit_report_result(PIGLIT_SKIP);
	}

	/* Compile shaders */
	vs_spiral = piglit_compile_shader_text(GL_VERTEX_SHADER, spiral_text);
	gs_spiral = piglit_compile_shader_text(GL_GEOMETRY_SHADER,
					       spiral_text);
	vs_ref_main = piglit_compile_shader_text(GL_VERTEX_SHADER,
						 vs_ref_text);
	vs_test_main = piglit_compile_shader_text(GL_VERTEX_SHADER,
						  vs_test_text);
	gs_test_main = piglit_compile_shader_text(GL_GEOMETRY_SHADER,
						  gs_test_text);
	asprintf(&text, gs_layout_template, num_vertices);
	gs_layout = piglit_compile_shader_text(GL_GEOMETRY_SHADER, text);
	free(text);
	fs_main = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);

	prog_ref = glCreateProgram();
	glAttachShader(prog_ref, vs_ref_main);
	glAttachShader(prog_ref, vs_spiral);
	glAttachShader(prog_ref, fs_main);
	glLinkProgram(prog_ref);
	if (!piglit_link_check_status(prog_ref))
		piglit_report_result(PIGLIT_FAIL);

	prog_test = glCreateProgram();
	glAttachShader(prog_test, vs_test_main);
	glAttachShader(prog_test, gs_test_main);
	glAttachShader(prog_test, gs_spiral);
	glAttachShader(prog_test, gs_layout);
	glAttachShader(prog_test, fs_main);
	glLinkProgram(prog_test);
	if (!piglit_link_check_status(prog_test))
		piglit_report_result(PIGLIT_FAIL);

	glDeleteShader(vs_spiral);
	glDeleteShader(gs_spiral);
	glDeleteShader(vs_ref_main);
	glDeleteShader(vs_test_main);
	glDeleteShader(gs_test_main);
	glDeleteShader(gs_layout);
	glDeleteShader(fs_main);

	/* Various other GL objects needed by the test */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &element_buf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buf);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}


/**
 * Simulate the action of the 3 geometry shader invocations by making
 * 3 glDrawElements() calls.  Primitive restart is used to simulate
 * the action of EndPrimitive().
 */
static void
draw_ref_pattern()
{
	int i, vertex_count, end_prim_offset;

	glUseProgram(prog_ref);
	glUniform1i(glGetUniformLocation(prog_ref, "num_vertices"),
		    num_vertices);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xffffffff);

	for (end_prim_offset = 0; end_prim_offset < 3; end_prim_offset++) {
		/* Note: this over-allocates the buffer somewhat.  The
		 * actual amount of buffer space we need is a complex
		 * formula involving num_vertices and end_prim_offset,
		 * and it's not worth computing precisely.
		 */
		GLuint *index_buffer =
			malloc(2 * sizeof(GLuint) * num_vertices);
		i = vertex_count = 0;
		while (true) {
			if (i % 3 == end_prim_offset)
				index_buffer[vertex_count++] = 0xffffffff;
			if (i == num_vertices)
				break;
			index_buffer[vertex_count++] = i++;
		}
		glUniform1i(glGetUniformLocation(prog_ref, "end_prim_offset"),
			    end_prim_offset);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			     sizeof(GLuint) * vertex_count, index_buffer,
			     GL_STATIC_DRAW);
		free(index_buffer);
		glDrawElements(GL_TRIANGLE_STRIP, vertex_count,
			       GL_UNSIGNED_INT, NULL);
	}

	glDisable(GL_PRIMITIVE_RESTART);
}


static void
draw_test_pattern()
{
	glUseProgram(prog_test);
	glUniform1i(glGetUniformLocation(prog_test, "num_vertices"),
		    num_vertices);
	glDrawArrays(GL_POINTS, 0, 3);
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;

	glEnable(GL_BLEND);
	glBlendEquation(GL_MAX);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Left half of the window is the test pattern */
	glViewport(0, 0, PATTERN_SIZE, PATTERN_SIZE);
	draw_test_pattern();

	/* Right half of the window is the reference image */
	glViewport(PATTERN_SIZE, 0, PATTERN_SIZE, PATTERN_SIZE);
	draw_ref_pattern();

	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	/* Compare window halves */
	pass = piglit_probe_rect_halves_equal_rgba(0, 0, 2*PATTERN_SIZE,
						   PATTERN_SIZE) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
